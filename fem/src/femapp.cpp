#include "femapp.h"

#include <QCloseEvent>
#include <QDir>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QVBoxLayout>
#include <sstream>
#include <string>

#include "QDebug"
#include "cc/neolux/fem/xlsx_proc.h"

using cc::neolux::femconfig::FEMConfig;
namespace app = cc::neolux::fem;

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
            [this]() { refreshXlsxEditor(); });
    connect(projectControlWidget, &cc::neolux::projectcontrol::ProjectControlWidget::dryRunToggled,
            this, [this](bool checked) {
                if (xlsxEditorModule) {
                    xlsxEditorModule->setDryRun(checked);
                }
            });

    auto* fileMenu = ui.menuBar->addMenu(tr("File"));
    auto* actionOpenProject = fileMenu->addAction(tr("Open Project..."));
    auto* actionSaveProject = fileMenu->addAction(tr("Save"));
    auto* actionSaveProjectAs = fileMenu->addAction(tr("Save As..."));
    fileMenu->addSeparator();
    auto* actionExit = fileMenu->addAction(tr("Exit"));

    connect(actionOpenProject, &QAction::triggered, this, [this]() { loadConfigFromDialog(); });
    connect(actionSaveProject, &QAction::triggered, this, [this]() { saveCurrentConfig(); });
    connect(actionSaveProjectAs, &QAction::triggered, this, [this]() { saveCurrentConfigAs(); });
    connect(actionExit, &QAction::triggered, this, [this]() { close(); });

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
            event->accept();
        } else if (result == QMessageBox::Discard) {
            // Discard changes and close
            event->accept();
        } else {
            // Cancel closing
            event->ignore();
        }
    } else {
        // No unsaved changes, close normally
        event->accept();
    }
}

void FemApp::loadFEMConfig() {
    this->loadFEMConfig(this->femc_info->absoluteFilePath());
}

void FemApp::loadFEMConfig(const QString& filePath) {
    if (filePath.isEmpty()) {
        // raise a error window here
        qWarning() << "file path is Empty!\n";
        return;
    }
    qDebug() << "Loading config file from " << filePath << "\n";

    // 切换工作文件夹
    QFileInfo fileInfo(filePath);
    QDir::setCurrent(fileInfo.absolutePath());

    if (!FEMConfig::ReadFile(filePath.toUtf8().toStdString(), this->femdata)) {
        qDebug() << "Failed to read file.";
        return;
    }

    // Track the current file path and clear modified flag
    currentFilePath = filePath;
    isModified = false;
    updateFileLabel();

    this->onLoadFile();
}

std::string FemApp::getFolderMatched() {
    auto folders = FEMConfig::ExpandFolderPattern(this->femdata);
    qDebug() << "Expanded Folders:" << folders.size();
    for (const auto& folder : folders) {
        qDebug() << "  " << QString::fromUtf8(folder.c_str());
    }
    if (folders.empty()) {
        showError(this, tr("No folders matched the folder pattern. "));
        return "";
    }
    if (folders.size() > 1) {
        showError(this, tr("Multiple folders matched the folder pattern. Please ensure only one "
                           "folder matches."));
        return "";
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
        showError(this, tr("No filenames matched the filename pattern."));
        return "";
    }

    // 如果有多个文件匹配
    if (filenames.size() > 1) {
        showError(this, tr("Multiple filenames matched the filename pattern. Please ensure only "
                           "one filename matches."));
        return "";
    }

    // 获取匹配的文件路径
    auto file = filenames.front();
    // 更新文件列表（只显示文件名）
    this->updateFileList(folder);

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
    if (sheets.empty()) {
        showError(this, tr("No sheets matched the sheet pattern. "));
        return "";
    }
    if (sheets.size() > 1) {
        showError(
            this,
            tr("Multiple sheets matched the sheet pattern. Please ensure only one sheet matches."));
        return "";
    }

    // 填入工作表列表，并默认选中匹配到的项
    this->updateSheetList(filename);
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
    refreshXlsxEditor();
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

void FemApp::loadConfigFromDialog() {
    QString femconfig_path =
        QFileDialog::getOpenFileName(this, tr("Open FEM Config File"), QDir::currentPath(),
                                     tr("FEM Config Files (*.fem);;All Files (*)"));
    if (femconfig_path.isEmpty()) {
        return;
    }
    this->loadFEMConfig(femconfig_path);
    qInfo() << "Loaded FEM config file from " << femconfig_path;
}

void FemApp::saveCurrentConfig() {
    std::ostringstream oss;
    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata, oss)) {
        showError(this, tr("Failed to generate FEM config content."));
        return;
    }
    femdata.rawContent = oss.str();
    projectControlWidget->setRawConfigText(QString::fromUtf8(femdata.rawContent.c_str()));

    if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(
            femdata, this->femc_info->absoluteFilePath().toUtf8().toStdString())) {
        showError(this, tr("Failed to save FEM config file."));
        return;
    }
    clearModifiedFlag();
    qInfo() << "Saved FEM config file to " << this->femc_info->absoluteFilePath();
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
    currentFilePath = femconfig_path;
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
}

void FemApp::setFilenamePattern(const QString& text) {
    femdata.filenamePattern = text.toUtf8().toStdString();
    markAsModified();
    qInfo() << "File pattern changed to " << text;

    if (isLoading) {
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
