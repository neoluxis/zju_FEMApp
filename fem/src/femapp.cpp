#include "femapp.h"

#include <QCloseEvent>
#include <QDir>
#include <QMessageBox>
#include <QShortcut>
#include <QSizePolicy>

#include "QDebug"
#include "cc/neolux/fem/xlsx_proc.h"
#include "iostream"

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
    ui.cbFile->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    ui.cbFile->setMinimumContentsLength(12);
    ui.cbFile->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui.cbSheet->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    ui.cbSheet->setMinimumContentsLength(12);
    ui.cbSheet->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    xlsxEditorModule = std::make_unique<cc::neolux::fem::XlsxEditorModule>(ui.vboxSheetEdit);
    xlsxEditorModule->setDryRun(ui.chkDryRun->isChecked());

    connect(ui.btnLoad, &QPushButton::clicked, this, &FemApp::onLoadClicked);
    connect(ui.btnSave, &QPushButton::clicked, this, &FemApp::onSaveClicked);
    connect(ui.btnSaveas, &QPushButton::clicked, this, &FemApp::onSaveAsClicked);
    connect(ui.btnFolderBrowse, &QPushButton::clicked, this, &FemApp::onBrowseClicked);
    connect(ui.btnFolderMatch, &QPushButton::clicked, this, &FemApp::onMatchClicked);
    connect(ui.lnFolder, &QLineEdit::textEdited, this, &FemApp::onFolderEdited);
    connect(ui.cbFile, &QComboBox::currentTextChanged, this, &FemApp::onFileChanged);
    connect(ui.cbSheet, &QComboBox::currentTextChanged, this, &FemApp::onSheetChanged);

    connect(ui.cbDMode, &QComboBox::currentTextChanged, this, &FemApp::onDModeChanged);
    connect(ui.cbDUnit, &QComboBox::currentTextChanged, this, &FemApp::onDUnitChanged);
    connect(ui.dspnDCenter, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &FemApp::onDCenterChanged);
    connect(ui.dspnDStep, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &FemApp::onDStepChanged);
    connect(ui.spnDNo, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onDNoChanged);
    connect(ui.lnDCols, &QLineEdit::editingFinished, this, &FemApp::onDColsEdited);

    connect(ui.cbFMode, &QComboBox::currentTextChanged, this, &FemApp::onFModeChanged);
    connect(ui.cbFUnit, &QComboBox::currentTextChanged, this, &FemApp::onFUnitChanged);
    connect(ui.dspnFCenter, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &FemApp::onFCenterChanged);
    connect(ui.dspnFStep, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &FemApp::onFStepChanged);
    connect(ui.spnFNo, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onFNoChanged);
    connect(ui.lnFRows, &QLineEdit::editingFinished, this, &FemApp::onFRowsEdited);

    connect(ui.cbFEMMode, &QComboBox::currentTextChanged, this, &FemApp::onFEMModeChanged);
    connect(ui.cbFEMUnit, &QComboBox::currentTextChanged, this, &FemApp::onFEMUnitChanged);
    connect(ui.dspnFEMTarg, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &FemApp::onFEMTargChanged);
    connect(ui.dspnFEMSpec, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &FemApp::onFEMSpecChanged);

    connect(ui.btnTxtReset, &QPushButton::clicked, this, &FemApp::onTxtResetClicked);
    connect(ui.btnTxtApply, &QPushButton::clicked, this, &FemApp::onTxtApplyClicked);
    connect(ui.txtConfigRaw, &QPlainTextEdit::textChanged, this, &FemApp::onRawFileEdited);
    connect(ui.btnRefreshEditor, &QPushButton::clicked, this, &FemApp::onRefreshEditorClicked);
    connect(ui.chkDryRun, &QCheckBox::toggled, this, [this](bool checked) {
        if (xlsxEditorModule) {
            xlsxEditorModule->setDryRun(checked);
        }
    });

    // Setup keyboard shortcuts
    // Ctrl+Q to exit
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, [this]() { this->close(); });

    // Ctrl+S to save
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this, [this]() { this->onSaveClicked(); });

    // Ctrl+Shift+S to save as
    new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S), this,
                  [this]() { this->onSaveAsClicked(); });

    // Alt+S to apply text edit
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_S), this, [this]() { this->onTxtApplyClicked(); });

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
            onSaveClicked();
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
    int index = ui.cbFile->findText(matchedFilename);
    if (index >= 0) {
        ui.cbFile->setCurrentIndex(index);
    } else {
        qWarning() << "Matched file not found in combo box:" << matchedFilename;
    }

    // 返回匹配的文件路径
    return file;
}

QString FemApp::getCurrentSelectedFile() {
    // 获取当前选中项的完整路径
    QString fullPath = ui.cbFile->currentData().toString();
    return fullPath;
}

void FemApp::updateFileList(const std::string& folder) {
    // 清空现有项
    ui.cbFile->clear();

    // 遍历文件夹并添加文件名及其路径到ComboBox
    QDir dir(QString::fromUtf8(folder.c_str()));
    QStringList files = dir.entryList(QDir::Files);
    for (const QString& file : files) {
        QString fullpath = dir.absoluteFilePath(file);
        // 将文件名添加到ComboBox，并设置userData为完整路径
        ui.cbFile->addItem(file, fullpath);
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

    // 把工作表填入 ui.cbSheet 中，并默认选中匹配到的一个
    this->updateSheetList(filename);
    QString matchedSheet = QString::fromUtf8(sheets.front().c_str());
    int index = ui.cbSheet->findText(matchedSheet);
    if (index >= 0) {
        ui.cbSheet->setCurrentIndex(index);
    } else {
        qWarning() << "Matched sheet not found in combo box:" << matchedSheet;
    }
    // 只ret第一个匹配到的工作表
    return sheets.front();
}

void FemApp::updateSheetList(const std::string& filename) {
    this->ui.cbSheet->clear();

    // Skip if filename is empty to avoid attempting to open non-existent files
    if (filename.empty()) {
        return;
    }

    try {
        app::XlsxProc xlsxproc;
        auto all_sheets = xlsxproc.GetSheetNames(filename);
        for (const auto& sheet : all_sheets) {
            QString qSheetName = QString::fromUtf8(sheet.c_str());
            this->ui.cbSheet->addItem(qSheetName);
        }
    } catch (const std::exception& e) {
        qWarning() << tr("Failed to read Excel file: ") << e.what();
    }
}

void FemApp::onLoadFile() {
    // Set loading flag to prevent marking as modified during control updates
    isLoading = true;

    // 先把原始内容填入文本框，即便出错也能修改重加载
    this->ui.txtConfigRaw->setPlainText(QString::fromUtf8(this->femdata.rawContent.c_str()));

    auto folder = getFolderMatched();
    if (folder.empty()) {
        isLoading = false;
        return;
    }
    this->ui.lnFolder->setText(QString::fromUtf8(folder.c_str()));

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
    this->ui.cbDMode->setCurrentText(QString::fromUtf8(this->femdata.dose.mode.c_str()));
    this->ui.cbDUnit->setCurrentText(QString::fromUtf8(this->femdata.dose.unit.c_str()));
    this->ui.dspnDCenter->setValue(
        static_cast<int>(this->femdata.dose.center));  // TODO: 需要询问此处数据类型，改控件和代码
    this->ui.dspnDStep->setValue(this->femdata.dose.step);
    this->ui.spnDNo->setValue(this->femdata.dose.no);
    this->ui.lnDCols->setText(QString::fromUtf8(this->femdata.dose.cols.c_str()));

    // Focus
    this->ui.cbFMode->setCurrentText(QString::fromUtf8(this->femdata.focus.mode.c_str()));
    this->ui.cbFUnit->setCurrentText(QString::fromUtf8(this->femdata.focus.unit.c_str()));
    this->ui.dspnFCenter->setValue(
        static_cast<int>(this->femdata.focus.center));  // TODO: 需要询问此处数据类型，改控件和代码
    this->ui.dspnFStep->setValue(this->femdata.focus.step);
    this->ui.spnFNo->setValue(this->femdata.focus.no);
    this->ui.lnFRows->setText(QString::fromUtf8(this->femdata.focus.rows.c_str()));

    // FEM
    this->ui.cbFEMMode->setCurrentText(QString::fromUtf8(this->femdata.fem.mode.c_str()));
    this->ui.cbFEMUnit->setCurrentText(QString::fromUtf8(this->femdata.fem.unit.c_str()));
    this->ui.dspnFEMTarg->setValue(static_cast<int>(this->femdata.fem.target));
    this->ui.dspnFEMSpec->setValue(static_cast<int>(this->femdata.fem.spec));

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

    ui.labelFEMFile->setText(displayText);
    this->setWindowTitle(windowTitle);
}

void FemApp::refreshXlsxEditor() {
    if (!xlsxEditorModule) {
        return;
    }
    xlsxEditorModule->setDryRun(ui.chkDryRun->isChecked());
    QString filePath = getCurrentSelectedFile();
    QString sheetName = ui.cbSheet->currentText();
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

void FemApp::onRefreshEditorClicked() {
    refreshXlsxEditor();
}

FemApp::~FemApp() = default;
