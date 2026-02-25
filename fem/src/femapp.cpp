#include "femapp.h"

#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QVBoxLayout>
#include <sstream>
#include <string>

#include "QDebug"
#include "cc/neolux/fem/mpw/multi_project_workspace.h"
#include "cc/neolux/fem/xlsx_proc.h"

using cc::neolux::femconfig::FEMConfig;
namespace app = cc::neolux::fem;

namespace {
cc::neolux::femconfig::FEMData BuildDefaultFemData() {
    cc::neolux::femconfig::FEMData data;
    data.folderPattern = ".";
    data.filenamePattern = "*.xlsx";
    data.sheetPattern = "*";

    data.dose.mode = "LowHigh";
    data.dose.unit = "mJ/cm2";
    data.dose.center = 0;
    data.dose.step = 0.05;
    data.dose.no = 23;
    data.dose.cols = "B:K";

    data.focus.mode = "NegPos2";
    data.focus.unit = "um";
    data.focus.center = 0;
    data.focus.step = 0.03;
    data.focus.no = 29;
    data.focus.rows = "3:60";

    data.fem.mode = "Focus2DoseLinear";
    data.fem.unit = "mJ/cm2";
    data.fem.target = 80;
    data.fem.spec = 5;
    return data;
}
}  // namespace

void FemApp::showError(QWidget* parent, const QString& text) {
    QMessageBox::critical(parent, tr("Error"), text, QMessageBox::Ok);
}

void FemApp::showWarning(QWidget* parent, const QString& text) {
    QMessageBox::warning(parent, tr("Warning"), text, QMessageBox::Ok);
}

void FemApp::showInfo(QWidget* parent, const QString& text) {
    QMessageBox::information(parent, tr("Info"), text, QMessageBox::Ok);
}

// TODO: 1. 回写 .fem 文件时，规范化。给folder, filename, sheet 添加上双引号。并为此双引号修复解析器
FemApp::FemApp(QWidget* parent) : QWidget(parent), currentFilePath(""), isModified(false) {
    ui.setupUi(this);

    ui.gridLayout_2->setAlignment(ui.vboxSettings, Qt::AlignTop | Qt::AlignLeft);

    auto* multiPrjWsLayout = new QVBoxLayout(ui.multiPrjWsHost);
    multiPrjWsLayout->setContentsMargins(0, 0, 0, 0);
    multiPrjWsWidget = new cc::neolux::fem::mpw::MultiPrjWsWidget(ui.multiPrjWsHost);
    multiPrjWsLayout->addWidget(multiPrjWsWidget);
    setWorkspaceMode(false);

    auto* projectControlLayout = new QVBoxLayout(ui.projectControlHost);
    projectControlLayout->setContentsMargins(0, 0, 0, 0);
    projectControlWidget =
        new cc::neolux::projectcontrol::ProjectControlWidget(ui.projectControlHost);
    projectControlLayout->addWidget(projectControlWidget);

    xlsxEditorModule = std::make_unique<cc::neolux::fem::XlsxEditorModule>(ui.vboxSheetEdit);
    xlsxEditorModule->setDryRun(projectControlWidget->isDryRunChecked());

    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::folderBrowseClicked, this,
            [this]() { browseFolder(); });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::folderMatchClicked, this,
            [this]() { matchFolderPattern(); });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::folderEdited,
            this, [this](const QString& text) { setFolderPattern(text); });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::fileChanged,
            this, [this](const QString& text) { setFilenamePattern(text); });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::sheetChanged,
            this, [this](const QString& text) { setSheetPattern(text); });

    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::doseModeChanged, this,
            [this](const QString& text) {
                femdata.dose.mode = text.toUtf8().toStdString();
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::doseUnitChanged, this,
            [this](const QString& text) {
                femdata.dose.unit = text.toUtf8().toStdString();
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::doseCenterChanged, this,
            [this](double value) {
                femdata.dose.center = value;
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::doseStepChanged, this,
            [this](double value) {
                femdata.dose.step = value;
                markAsModified();
            });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::doseNoChanged,
            this, [this](int value) {
                femdata.dose.no = value;
                markAsModified();
            });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::doseColsEdited,
            this, [this]() {
                femdata.dose.cols = projectControlWidget->doseColsText().toUtf8().toStdString();
                markAsModified();
            });

    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::focusModeChanged, this,
            [this](const QString& text) {
                femdata.focus.mode = text.toUtf8().toStdString();
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::focusUnitChanged, this,
            [this](const QString& text) {
                femdata.focus.unit = text.toUtf8().toStdString();
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::focusCenterChanged, this,
            [this](double value) {
                femdata.focus.center = value;
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::focusStepChanged, this,
            [this](double value) {
                femdata.focus.step = value;
                markAsModified();
            });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::focusNoChanged,
            this, [this](int value) {
                femdata.focus.no = value;
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::focusRowsEdited, this, [this]() {
                femdata.focus.rows = projectControlWidget->focusRowsText().toUtf8().toStdString();
                markAsModified();
            });

    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::femModeChanged,
            this, [this](const QString& text) {
                femdata.fem.mode = text.toUtf8().toStdString();
                markAsModified();
            });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::femUnitChanged,
            this, [this](const QString& text) {
                femdata.fem.unit = text.toUtf8().toStdString();
                markAsModified();
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::femTargetChanged, this,
            [this](double value) {
                femdata.fem.target = value;
                markAsModified();
            });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::femSpecChanged,
            this, [this](double value) {
                femdata.fem.spec = value;
                markAsModified();
            });

    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::txtResetClicked, this, [this]() {
                projectControlWidget->setRawConfigText(
                    QString::fromUtf8(this->femdata.rawContent.c_str()));
            });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::txtApplyClicked, this,
            [this]() { applyRawConfigText(); });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::rawTextEdited,
            this, [this]() { markAsModified(); });
    connect(projectControlWidget,
            &cc::neolux::projectcontrol::ProjectControlWidget::refreshEditorClicked, this,
            [this]() {
                if (relaxPatternMatchValidation) {
                    relaxPatternMatchValidation = false;
                }
                refreshXlsxEditor();
            });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::dryRunToggled,
            this, [this](bool checked) {
                if (xlsxEditorModule) {
                    xlsxEditorModule->setDryRun(checked);
                }
            });
    connect(multiPrjWsWidget, &cc::neolux::fem::mpw::MultiPrjWsWidget::projectActivated, this,
            [this](const QString& projectFilePath) { loadFEMConfig(projectFilePath); });

    auto* fileMenu = ui.menuBar->addMenu(tr("File"));
    auto* newMenu = fileMenu->addMenu(tr("New"));
    auto* actionNewProject = newMenu->addAction(tr("Project"));
    auto* actionNewWorkspace = newMenu->addAction(tr("Workspace"));
    fileMenu->addSeparator();

    auto* actionOpenProject = fileMenu->addAction(tr("Open Project..."));
    workspaceConfigAction = fileMenu->addAction(tr("Workspace Config..."));
    recentMenu = fileMenu->addMenu(tr("Recent"));
    auto* actionSaveProject = fileMenu->addAction(tr("Save"));
    auto* actionSaveProjectAs = fileMenu->addAction(tr("Save As..."));
    fileMenu->addSeparator();
    auto* actionExit = fileMenu->addAction(tr("Exit"));

    connect(actionNewProject, &QAction::triggered, this, [this]() { createNewProject(); });
    connect(actionNewWorkspace, &QAction::triggered, this, [this]() { createNewWorkspace(); });
    connect(actionOpenProject, &QAction::triggered, this, [this]() { loadConfigFromDialog(); });
    connect(workspaceConfigAction, &QAction::triggered, this,
            [this]() { openMultiProjectWorkspaceConfig(); });
    connect(actionSaveProject, &QAction::triggered, this, [this]() { saveCurrentConfig(); });
    connect(actionSaveProjectAs, &QAction::triggered, this, [this]() { saveCurrentConfigAs(); });
    connect(actionExit, &QAction::triggered, this, [this]() { close(); });

    refreshRecentMenu();

    // Setup keyboard shortcuts
    // Ctrl+Q to exit
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, [this]() { this->close(); });

    // Ctrl+S to save
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this,
                  [this]() { this->saveCurrentConfig(); });

    // Ctrl+Shift+S to save as
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this,
                  [this]() { this->saveCurrentConfigAs(); });

    // Alt+S to apply text edit
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_S), this,
                  [this]() { this->applyRawConfigText(); });

    restoreWindowGeometryState();

    // Initialize label
    updateFileLabel();
}

void FemApp::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        ui.retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void FemApp::closeEvent(QCloseEvent* event) {
    // Check if there are unsaved changes
    if (isModified && !currentFilePath.isEmpty()) {
        int result = QMessageBox::warning(
            this, tr("Unsaved Changes"),
            tr("You have unsaved changes. Do you want to save before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

        if (result == QMessageBox::Save) {
            // Save the file
            saveCurrentConfig();
            saveWindowGeometryState();
            event->accept();
        } else if (result == QMessageBox::Discard) {
            // Discard changes and close
            saveWindowGeometryState();
            event->accept();
        } else {
            // Cancel closing
            event->ignore();
        }
    } else {
        // No unsaved changes, close normally
        saveWindowGeometryState();
        event->accept();
    }
}

bool FemApp::loadFEMConfig() {
    if (currentFilePath.isEmpty()) {
        return false;
    }
    return this->loadFEMConfig(currentFilePath);
}

bool FemApp::openSingleProject(const QString& filePath) {
    setWorkspaceMode(false);
    currentWorkspaceFilePath.clear();
    return loadFEMConfig(filePath);
}

bool FemApp::loadFEMConfig(const QString& filePath) {
    if (filePath.isEmpty()) {
        showError(this, tr("Config file path is empty."));
        return false;
    }
    QFileInfo fileInfo(filePath);
    const QString absoluteFilePath = fileInfo.absoluteFilePath();

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        showError(this, tr("Config file does not exist: ") + absoluteFilePath);
        return false;
    }

    qDebug() << "Loading config file from " << absoluteFilePath << "\n";

    // 切换工作文件夹
    QDir::setCurrent(fileInfo.absolutePath());

    if (!FEMConfig::ReadFile(absoluteFilePath.toUtf8().toStdString(), this->femdata)) {
        showError(this, tr("Failed to read FEM config file."));
        return false;
    }

    // Track the current file path and clear modified flag
    currentFilePath = absoluteFilePath;
    isModified = false;
    updateFileLabel();
    recentProjectHistory.addProject(currentFilePath);
    refreshRecentMenu();

    this->onLoadFile();
    return true;
}

bool FemApp::loadMultiProjectWorkspace(const QString& filePath) {
    if (!multiPrjWsWidget) {
        return false;
    }

    setWorkspaceMode(true);
    currentWorkspaceFilePath = QFileInfo(filePath).absoluteFilePath();
    recentProjectHistory.addProject(currentWorkspaceFilePath);
    refreshRecentMenu();

    QString errorMessage;
    if (!multiPrjWsWidget->loadWorkspaceFile(currentWorkspaceFilePath, &errorMessage)) {
        showError(this, errorMessage);
        return false;
    }

    const QString firstProjectPath = multiPrjWsWidget->firstEnabledProjectPath();
    if (!firstProjectPath.isEmpty()) {
        const bool loaded = loadFEMConfig(firstProjectPath);
        if (loaded) {
            multiPrjWsWidget->markProjectOpened(firstProjectPath);
        }
        return loaded;
    }
    return true;
}

void FemApp::openMultiProjectWorkspaceConfig() {
    if (currentWorkspaceFilePath.isEmpty()) {
        showWarning(this, tr("No workspace is currently opened."));
        return;
    }

    cc::neolux::fem::mpw::MultiPrjWsConfigDialog dialog(this);
    QString errorMessage;
    if (!dialog.loadWorkspace(currentWorkspaceFilePath, &errorMessage)) {
        showError(this, errorMessage);
        return;
    }

    connect(&dialog, &cc::neolux::fem::mpw::MultiPrjWsConfigDialog::workspaceSaved, this,
            [this](const QString& workspaceFilePath) {
                QString reloadError;
                if (!multiPrjWsWidget->loadWorkspaceFile(workspaceFilePath, &reloadError)) {
                    showError(this, reloadError);
                    return;
                }
                if (!currentFilePath.isEmpty()) {
                    multiPrjWsWidget->markProjectOpened(currentFilePath);
                }
            });

    dialog.exec();
}

void FemApp::setWorkspaceMode(bool enabled) {
    workspaceMode = enabled;
    if (ui.multiPrjWsHost) {
        ui.multiPrjWsHost->setVisible(enabled);
    }
    if (workspaceConfigAction) {
        workspaceConfigAction->setEnabled(enabled);
    }
}

void FemApp::restoreWindowGeometryState() {
    QSettings settings("neolux", "FemApp");
    const QByteArray geometry = settings.value("ui/mainWindowGeometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    const int stateValue =
        settings.value("ui/mainWindowState", static_cast<int>(Qt::WindowNoState)).toInt();
    setWindowState(static_cast<Qt::WindowStates>(stateValue));
}

void FemApp::saveWindowGeometryState() {
    QSettings settings("neolux", "FemApp");
    settings.setValue("ui/mainWindowGeometry", saveGeometry());
    settings.setValue("ui/mainWindowState", static_cast<int>(windowState()));
}

std::string FemApp::getFolderMatched() {
    auto folders = FEMConfig::ExpandFolderPattern(this->femdata);
    qDebug() << "Expanded Folders:" << folders.size();
    for (const auto& folder : folders) {
        qDebug() << "  " << QString::fromUtf8(folder.c_str());
    }
    if (folders.empty()) {
        if (!relaxPatternMatchValidation) {
            showError(this, tr("No folders matched the folder pattern. "));
        }
        return "";
    }
    if (folders.size() > 1) {
        if (!relaxPatternMatchValidation) {
            showError(this,
                      tr("Multiple folders matched the folder pattern. Please ensure only one "
                         "folder matches."));
            return "";
        }
    }
    // 只处理第一个匹配到的文件夹
    auto folder = folders.front();

    // 获取相对路径
    QDir cwd(QDir::currentPath());
    QFileInfo folderInfo(QString::fromUtf8(folder.c_str()));
    QString relativePath = cwd.relativeFilePath(folderInfo.absoluteFilePath());
    return relativePath.toUtf8().toStdString();
}

std::string FemApp::getFileMatched(std::string folder) {
    // 获取符合文件名模式的文件列表
    auto filenames = FEMConfig::ExpandFilenamePattern(folder, this->femdata);
    qDebug() << "Expanded Filenames:" << filenames.size();

    // 输出匹配的文件名
    for (const auto& filename : filenames) {
        qDebug() << "  " << QString::fromUtf8(filename.c_str());
    }

    // 若没有匹配的文件
    if (filenames.empty()) {
        if (!relaxPatternMatchValidation) {
            showError(this, tr("No filenames matched the filename pattern."));
        }
        return "";
    }

    // 更新文件列表（只显示文件名）
    this->updateFileList(folder);

    // 如果有多个文件匹配
    if (filenames.size() > 1) {
        if (!relaxPatternMatchValidation) {
            showError(this,
                      tr("Multiple filenames matched the filename pattern. Please ensure only "
                         "one filename matches."));
            return "";
        }
    }

    // 获取匹配的文件路径
    auto file = filenames.front();

    // 从路径获取文件名
    QFileInfo fileInfo(QString::fromUtf8(file.c_str()));
    QString matchedFilename = fileInfo.fileName();

    qInfo() << "Matched filename: " << matchedFilename;

    // 在ComboBox中选中匹配的文件
    int index = projectControlWidget->findFileText(matchedFilename);
    if (index >= 0) {
        projectControlWidget->setFileIndex(index);
    } else {
        qWarning() << "Matched file not found in combo box:" << matchedFilename;
    }

    // 返回匹配的文件路径
    return file;
}

QString FemApp::getCurrentSelectedFile() {
    // 获取当前选中项的完整路径
    QString fullPath = projectControlWidget->currentFileData();
    return fullPath;
}

void FemApp::updateFileList(const std::string& folder) {
    // 清空现有项
    projectControlWidget->clearFiles();

    // 遍历文件夹并添加文件名及其路径到ComboBox
    QDir dir(QString::fromUtf8(folder.c_str()));
    QStringList files = dir.entryList(QDir::Files);
    for (const QString& file : files) {
        QString fullpath = dir.absoluteFilePath(file);
        // 将文件名添加到ComboBox，并设置userData为完整路径
        projectControlWidget->addFileItem(file, fullpath);
    }
}

std::string FemApp::getSheetMatched(std::string filename) {
    auto sheets = FEMConfig::ExpandSheetPattern(filename, this->femdata);
    qDebug() << "Expanded Sheets:" << sheets.size();
    for (const auto& sheet : sheets) {
        qDebug() << "  " << QString::fromUtf8(sheet.c_str());
    }

    // 先填入工作表列表，避免在多匹配时中断 UI 选项初始化
    this->updateSheetList(filename);

    if (sheets.empty()) {
        if (!relaxPatternMatchValidation) {
            showError(this, tr("No sheets matched the sheet pattern. "));
        }
        return "";
    }
    if (sheets.size() > 1) {
        if (!relaxPatternMatchValidation) {
            showError(this, tr("Multiple sheets matched the sheet pattern. Please ensure only one "
                               "sheet matches."));
            return "";
        }
    }

    // 默认选中匹配到的第一项
    QString matchedSheet = QString::fromUtf8(sheets.front().c_str());
    int index = projectControlWidget->findSheetText(matchedSheet);
    if (index >= 0) {
        projectControlWidget->setSheetIndex(index);
    } else {
        qWarning() << "Matched sheet not found in combo box:" << matchedSheet;
    }
    // 只ret第一个匹配到的工作表
    return sheets.front();
}

void FemApp::updateSheetList(const std::string& filename) {
    projectControlWidget->clearSheets();

    // Skip if filename is empty to avoid attempting to open non-existent files
    if (filename.empty()) {
        return;
    }

    try {
        app::XlsxProc xlsxproc;
        auto all_sheets = xlsxproc.GetSheetNames(filename);
        for (const auto& sheet : all_sheets) {
            QString qSheetName = QString::fromUtf8(sheet.c_str());
            projectControlWidget->addSheetItem(qSheetName);
        }
    } catch (const std::exception& e) {
        qWarning() << tr("Failed to read Excel file: ") << e.what();
    }
}

void FemApp::onLoadFile() {
    // Set loading flag to prevent marking as modified during control updates
    isLoading = true;

    // 先把原始内容填入文本框，即便出错也能修改重加载
    projectControlWidget->setRawConfigText(QString::fromUtf8(this->femdata.rawContent.c_str()));

    auto folder = getFolderMatched();
    if (folder.empty()) {
        isLoading = false;
        return;
    }
    projectControlWidget->setFolderText(QString::fromUtf8(folder.c_str()));

    // 处理文件名通配符
    auto filename = getFileMatched(folder);
    if (filename.empty()) {
        isLoading = false;
        return;
    }

    // 把工作表填入，并选中匹配到者
    auto sheet = getSheetMatched(filename);
    if (sheet.empty()) {
        isLoading = false;
        return;
    }

    // =======================
    // Dose
    projectControlWidget->setDoseModeText(QString::fromUtf8(this->femdata.dose.mode.c_str()));
    projectControlWidget->setDoseUnitText(QString::fromUtf8(this->femdata.dose.unit.c_str()));
    projectControlWidget->setDoseCenterValue(
        static_cast<int>(this->femdata.dose.center));  // TODO: 需要询问此处数据类型，改控件和代码
    projectControlWidget->setDoseStepValue(this->femdata.dose.step);
    projectControlWidget->setDoseNoValue(this->femdata.dose.no);
    projectControlWidget->setDoseColsText(QString::fromUtf8(this->femdata.dose.cols.c_str()));

    // Focus
    projectControlWidget->setFocusModeText(QString::fromUtf8(this->femdata.focus.mode.c_str()));
    projectControlWidget->setFocusUnitText(QString::fromUtf8(this->femdata.focus.unit.c_str()));
    projectControlWidget->setFocusCenterValue(
        static_cast<int>(this->femdata.focus.center));  // TODO: 需要询问此处数据类型，改控件和代码
    projectControlWidget->setFocusStepValue(this->femdata.focus.step);
    projectControlWidget->setFocusNoValue(this->femdata.focus.no);
    projectControlWidget->setFocusRowsText(QString::fromUtf8(this->femdata.focus.rows.c_str()));

    // FEM
    projectControlWidget->setFemModeText(QString::fromUtf8(this->femdata.fem.mode.c_str()));
    projectControlWidget->setFemUnitText(QString::fromUtf8(this->femdata.fem.unit.c_str()));
    projectControlWidget->setFemTargetValue(static_cast<int>(this->femdata.fem.target));
    projectControlWidget->setFemSpecValue(static_cast<int>(this->femdata.fem.spec));

    // Clear loading flag after all controls are updated
    isLoading = false;
    // 在所有配置加载并初始化完成后，自动刷新一次编辑器（仅在未发生错误时到达此处）
    if (skipAutoRefreshEditorOnce) {
        skipAutoRefreshEditorOnce = false;
    } else {
        refreshXlsxEditor();
    }
}

void FemApp::updateFileLabel() {
    QString displayText;
    QString windowTitle = tr("FemApp");

    if (currentFilePath.isEmpty()) {
        displayText = "";
    } else {
        QFileInfo fileInfo(currentFilePath);
        displayText = fileInfo.fileName();
        windowTitle = tr("FemApp") + " - " + displayText;

        if (isModified) {
            displayText += " *";
            windowTitle += " *";
        }
    }

    projectControlWidget->setFileDisplayText(displayText);
    this->setWindowTitle(windowTitle);
}

void FemApp::refreshRecentMenu() {
    if (!recentMenu) {
        return;
    }

    recentMenu->clear();
    const QStringList projects = recentProjectHistory.recentProjects();

    QStringList workspacePaths;
    QStringList projectPaths;
    for (const QString& path : projects) {
        if (cc::neolux::fem::mpw::MultiProjectWorkspace::IsValidWorkspaceFile(path)) {
            workspacePaths.append(path);
        } else {
            projectPaths.append(path);
        }
    }

    auto addGroup = [this](const QString& title, const QStringList& paths) {
        if (paths.isEmpty()) {
            return;
        }

        QAction* header = recentMenu->addAction(title);
        header->setEnabled(false);
        for (const QString& path : paths) {
            QAction* action = recentMenu->addAction(path);
            connect(action, &QAction::triggered, this, [this, path]() {
                if (cc::neolux::fem::mpw::MultiProjectWorkspace::IsValidWorkspaceFile(path)) {
                    loadMultiProjectWorkspace(path);
                } else {
                    openSingleProject(path);
                }
            });
        }
    };

    addGroup(tr("Workspace"), workspacePaths);
    if (!workspacePaths.isEmpty() && !projectPaths.isEmpty()) {
        recentMenu->addSeparator();
    }
    addGroup(tr("Project"), projectPaths);

    recentMenu->addSeparator();
    QAction* clearAction = recentMenu->addAction(tr("Clear"));
    clearAction->setEnabled(!projects.isEmpty());
    connect(clearAction, &QAction::triggered, this, [this]() {
        recentProjectHistory.clear();
        refreshRecentMenu();
    });

    recentMenu->setEnabled(true);
}

void FemApp::refreshXlsxEditor() {
    if (!xlsxEditorModule) {
        return;
    }
    xlsxEditorModule->setDryRun(projectControlWidget->isDryRunChecked());
    QString filePath = getCurrentSelectedFile();
    QString sheetName = projectControlWidget->currentSheetText();
    QString cols = QString::fromUtf8(femdata.dose.cols.c_str());
    QString rows = QString::fromUtf8(femdata.focus.rows.c_str());
    QString range;
    if (!cols.isEmpty() && !rows.isEmpty()) {
        range = QString("%1,%2").arg(cols, rows);
    }
    xlsxEditorModule->load(filePath, sheetName, range);
}

void FemApp::markAsModified() {
    if (!isLoading) {
        isModified = true;
        updateFileLabel();
    }
}

void FemApp::clearModifiedFlag() {
    isModified = false;
    updateFileLabel();
}

void FemApp::applyRawConfigText() {
    std::string text = projectControlWidget->rawConfigText().toUtf8().toStdString();
    auto* newdata = new cc::neolux::femconfig::FEMData();
    if (!cc::neolux::femconfig::FEMConfig::ParseContent(text, *newdata)) {
        showError(this, tr("Failed to parse raw FEM config content."));
        delete newdata;
        return;
    }
    this->femdata = *newdata;
    delete newdata;
    this->onLoadFile();
}

void FemApp::createNewProject() {
    QString projectPath =
        QFileDialog::getSaveFileName(this, tr("Create New FEM Project"), QDir::currentPath(),
                                     tr("FEM Config Files (*.fem);;All Files (*)"));
    if (projectPath.isEmpty()) {
        return;
    }

    QFileInfo projectInfo(projectPath);
    if (projectInfo.suffix().isEmpty()) {
        projectPath += ".fem";
    }
    const QString absoluteProjectPath = QFileInfo(projectPath).absoluteFilePath();

    cc::neolux::femconfig::FEMData newData = BuildDefaultFemData();
    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(
            newData, absoluteProjectPath.toUtf8().toStdString())) {
        showError(this, tr("Failed to create FEM config file."));
        return;
    }

    relaxPatternMatchValidation = true;
    skipAutoRefreshEditorOnce = true;
    openSingleProject(absoluteProjectPath);
}

void FemApp::createNewWorkspace() {
    QString workspacePath =
        QFileDialog::getSaveFileName(this, tr("Create New Workspace File"), QDir::currentPath(),
                                     tr("Workspace Files (*.femmpw);;All Files (*)"));
    if (workspacePath.isEmpty()) {
        return;
    }

    QFileInfo workspaceInfo(workspacePath);
    if (workspaceInfo.suffix().isEmpty()) {
        workspacePath += ".femmpw";
        workspaceInfo = QFileInfo(workspacePath);
    }

    const QString absoluteWorkspacePath = workspaceInfo.absoluteFilePath();
    const QDir workspaceDir = workspaceInfo.absoluteDir();
    const QString baseName = workspaceInfo.completeBaseName().isEmpty()
                                 ? QStringLiteral("workspace")
                                 : workspaceInfo.completeBaseName();

    QString projectName = baseName + "_project.fem";
    QString absoluteProjectPath = workspaceDir.filePath(projectName);
    int index = 1;
    while (QFileInfo::exists(absoluteProjectPath)) {
        projectName = QString("%1_project_%2.fem").arg(baseName).arg(index++);
        absoluteProjectPath = workspaceDir.filePath(projectName);
    }

    cc::neolux::femconfig::FEMData newProjectData = BuildDefaultFemData();
    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(
            newProjectData, absoluteProjectPath.toUtf8().toStdString())) {
        showError(this, tr("Failed to create FEM config file."));
        return;
    }

    cc::neolux::fem::mpw::MultiProjectWorkspaceData workspaceData;
    workspaceData.workspaceName = baseName;

    cc::neolux::fem::mpw::WorkspaceProjectItem item;
    item.projectFilePath = workspaceDir.relativeFilePath(absoluteProjectPath);
    item.displayName = QFileInfo(absoluteProjectPath).fileName();
    item.note = QString();
    item.enabled = true;
    workspaceData.projects.append(item);

    QString errorMessage;
    if (!cc::neolux::fem::mpw::MultiProjectWorkspace::WriteFile(absoluteWorkspacePath,
                                                                workspaceData, &errorMessage)) {
        showError(this, errorMessage);
        return;
    }

    loadMultiProjectWorkspace(absoluteWorkspacePath);
}

void FemApp::loadConfigFromDialog() {
    QString femconfig_path =
        QFileDialog::getOpenFileName(this, tr("Open FEM Config File"), QDir::currentPath(),
                                     tr("FEM Config Files (*.fem);;All Files (*)"));
    if (femconfig_path.isEmpty()) {
        return;
    }
    this->openSingleProject(femconfig_path);
    qInfo() << "Loaded FEM config file from " << femconfig_path;
}

void FemApp::saveCurrentConfig() {
    if (currentFilePath.isEmpty()) {
        saveCurrentConfigAs();
        return;
    }

    std::ostringstream oss;
    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata, oss)) {
        showError(this, tr("Failed to generate FEM config content."));
        return;
    }
    femdata.rawContent = oss.str();
    projectControlWidget->setRawConfigText(QString::fromUtf8(femdata.rawContent.c_str()));

    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata,
                                                       currentFilePath.toUtf8().toStdString())) {
        showError(this, tr("Failed to save FEM config file."));
        return;
    }
    clearModifiedFlag();
    qInfo() << "Saved FEM config file to " << currentFilePath;
}

void FemApp::saveCurrentConfigAs() {
    QString femconfig_path =
        QFileDialog::getSaveFileName(this, tr("Save FEM Config File As"), QDir::currentPath(),
                                     tr("FEM Config Files (*.fem);;All Files (*)"));
    if (femconfig_path.isEmpty()) {
        return;
    }

    std::ostringstream oss;
    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata, oss)) {
        showError(this, tr("Failed to generate FEM config content."));
        return;
    }
    femdata.rawContent = oss.str();
    projectControlWidget->setRawConfigText(QString::fromUtf8(femdata.rawContent.c_str()));

    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata,
                                                       femconfig_path.toUtf8().toStdString())) {
        showError(this, tr("Failed to save FEM config file."));
        return;
    }
    currentFilePath = QFileInfo(femconfig_path).absoluteFilePath();
    recentProjectHistory.addProject(currentFilePath);
    refreshRecentMenu();
    clearModifiedFlag();
    qInfo() << "Saved FEM config file to " << femconfig_path;
}

void FemApp::browseFolder() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Folder"), QDir::currentPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) {
        return;
    }
    QDir cwd = QDir::current();
    QString relativePath = cwd.relativeFilePath(dir);
    projectControlWidget->setFolderText(relativePath);
    femdata.folderPattern = relativePath.toUtf8().toStdString();
    updateFileList(relativePath.toUtf8().toStdString());
    qInfo() << "Folder selected: " << dir;
}

void FemApp::matchFolderPattern() {
    std::string folder = getFolderMatched();
    if (folder.empty()) {
        return;
    }
    projectControlWidget->setFolderText(QString::fromUtf8(folder.c_str()));
    femdata.folderPattern = folder;
    updateFileList(folder);
    qInfo() << "Folder matched: " << QString::fromUtf8(folder.c_str());
}

void FemApp::setFolderPattern(const QString& text) {
    femdata.folderPattern = text.toUtf8().toStdString();
    markAsModified();
    qInfo() << "Folder pattern changed to " << text;

    if (isLoading || !relaxPatternMatchValidation) {
        return;
    }

    const std::string matchedFolder = getFolderMatched();
    if (matchedFolder.empty()) {
        projectControlWidget->clearFiles();
        projectControlWidget->clearSheets();
        return;
    }

    updateFileList(matchedFolder);
    const QString selectedFile = getCurrentSelectedFile();
    if (!selectedFile.isEmpty()) {
        updateSheetList(selectedFile.toUtf8().toStdString());
    } else {
        projectControlWidget->clearSheets();
    }
}

void FemApp::setFilenamePattern(const QString& text) {
    femdata.filenamePattern = text.toUtf8().toStdString();
    markAsModified();
    qInfo() << "File pattern changed to " << text;

    if (isLoading) {
        return;
    }

    if (relaxPatternMatchValidation) {
        QString selectedFile = getCurrentSelectedFile();
        if (!selectedFile.isEmpty()) {
            updateSheetList(selectedFile.toUtf8().toStdString());
        } else {
            projectControlWidget->clearSheets();
        }
        return;
    }

    QString selectedFile = getCurrentSelectedFile();
    if (!selectedFile.isEmpty()) {
        auto sheet = getSheetMatched(selectedFile.toUtf8().toStdString());
        if (!sheet.empty()) {
            qInfo() << "Sheet auto-selected: " << QString::fromUtf8(sheet.c_str());
        }
    }
}

void FemApp::setSheetPattern(const QString& text) {
    femdata.sheetPattern = text.toUtf8().toStdString();
    markAsModified();
}

FemApp::~FemApp() = default;
