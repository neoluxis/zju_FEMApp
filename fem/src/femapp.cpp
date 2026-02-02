#include "femapp.h"
#include "iostream"
#include "QDebug"
#include "filesystem"

#include "cc/neolux/fem/xlsx_proc.h"

#include <QMessageBox>

using cc::neolux::femconfig::FEMConfig;
namespace fs = std::filesystem;
namespace app = cc::neolux::fem;

void FemApp::showError(QWidget *parent, const QString &text) {
  QMessageBox::critical(parent, "Error", text, QMessageBox::Ok);
}

void FemApp::showWarning(QWidget *parent, const QString &text) {
  QMessageBox::warning(parent, "Warning", text, QMessageBox::Ok);
}

void FemApp::showInfo(QWidget *parent, const QString &text) {
  QMessageBox::information(parent, "Info", text, QMessageBox::Ok);
}


FemApp::FemApp(QWidget *parent)
  : QWidget(parent) {
  ui.setupUi(this);

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
  connect(ui.spnDCenter, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onDCenterChanged);
  connect(ui.dspnDStep, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FemApp::onDStepChanged);
  connect(ui.spnDNo, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onDNoChanged);
  connect(ui.lnDCols, &QLineEdit::editingFinished, this, &FemApp::onDColsEdited);

  connect(ui.cbFMode, &QComboBox::currentTextChanged, this, &FemApp::onFModeChanged);
  connect(ui.cbFUnit, &QComboBox::currentTextChanged, this, &FemApp::onFUnitChanged);
  connect(ui.spnFCenter, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onFCenterChanged);
  connect(ui.dspnFStep, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FemApp::onFStepChanged);
  connect(ui.spnFNo, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onFNoChanged);
  connect(ui.lnFRows, &QLineEdit::editingFinished, this, &FemApp::onFRowsEdited);

  connect(ui.cbFEMMode, &QComboBox::currentTextChanged, this, &FemApp::onFEMModeChanged);
  connect(ui.cbFEMUnit, &QComboBox::currentTextChanged, this, &FemApp::onFEMUnitChanged);
  connect(ui.spnFEMTarg, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onFEMTargChanged);
  connect(ui.spnFEMSpec, qOverload<int>(&QSpinBox::valueChanged), this, &FemApp::onFEMSpecChanged);
}

void FemApp::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui.retranslateUi(this);
  }
  QWidget::changeEvent(event);
}


void FemApp::loadFEMConfig(void) {
  this->loadFEMConfig(
    this->femc_info->absoluteFilePath());
}

void FemApp::loadFEMConfig(const QString &filePath) {
  if (filePath.isEmpty()) {
    // raise a error window here
    qWarning() << "file path is Empty!\n";
    return;
  }
  qDebug() << "Loading config file from " << filePath << "\n";

  // 切换工作文件夹
  std::filesystem::path folder = std::filesystem::path(filePath.toStdString()).parent_path();
  std::filesystem::current_path(folder);


  if (!FEMConfig::ReadFile(filePath.toStdString(), this->femdata)) {
    qDebug() << "Failed to read file.";
    return;
  }
  this->onLoadFile();
}

std::string FemApp::getFolderMatched() {
  auto folders = FEMConfig::ExpandFolderPattern(this->femdata);
  qDebug() << "Expanded Folders:" << folders.size();
  for (const auto &folder: folders) {
    qDebug() << "  " << QString::fromStdString(folder);
  }
  if (folders.empty()) {
    showError(this, tr("No folders matched the folder pattern. "));
    return "";
  }
  if (folders.size() > 1) {
    showError(this, tr("Multiple folders matched the folder pattern. Please ensure only one folder matches."));
    return "";
  }
  // 只处理第一个匹配到的文件夹
  auto folder = folders.front();

  // 获取相对路径
  fs::path cwd = fs::current_path();
  fs::path folderPath(folder);
  fs::path relativePath = fs::relative(folderPath, cwd);
  return relativePath.u8string();
}

std::string FemApp::getFileMatched(std::string folder) {
  // 获取符合文件名模式的文件列表
  auto filenames = FEMConfig::ExpandFilenamePattern(folder, this->femdata);
  qDebug() << "Expanded Filenames:" << filenames.size();

  // 输出匹配的文件名
  for (const auto &filename: filenames) {
    qDebug() << "  " << QString::fromStdString(filename);
  }

  // 若没有匹配的文件
  if (filenames.empty()) {
    showError(this, tr("No filenames matched the filename pattern."));
    return "";
  }

  // 如果有多个文件匹配
  if (filenames.size() > 1) {
    showError(this, tr("Multiple filenames matched the filename pattern. Please ensure only one filename matches."));
    return "";
  }

  // 获取匹配的文件路径
  auto file = filenames.front();
  // 更新文件列表（只显示文件名）
  this->updateFileList(folder);

  // 从路径获取文件名
  fs::path p = fs::u8path(file);
  QString matchedFilename = QString::fromUtf8(p.filename().u8string().c_str());

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


void FemApp::updateFileList(const std::string &folder) {
  // 清空现有项
  ui.cbFile->clear();

  // 遍历文件夹并添加文件名及其路径到ComboBox
  for (const auto &entry: fs::directory_iterator(folder)) {
    if (!entry.is_regular_file()) {
      continue;
    }

    // 获取文件名和完整路径
    QString filename = QString::fromUtf8(entry.path().filename().u8string().c_str());
    QString fullpath = QString::fromUtf8(entry.path().u8string().c_str());

    // 将文件名添加到ComboBox，并设置userData为完整路径
    ui.cbFile->addItem(filename, fullpath);
  }
}

std::string FemApp::getSheetMatched(std::string filename) {
  auto sheets = FEMConfig::ExpandSheetPattern(
    filename,
    this->femdata);
  qDebug() << "Expanded Sheets:" << sheets.size();
  for (const auto &sheet: sheets) {
    qDebug() << "  " << QString::fromStdString(sheet);
  }
  if (sheets.empty()) {
    showError(this, tr("No sheets matched the sheet pattern. "));
    return "";
  }
  if (sheets.size() > 1) {
    showError(this, tr("Multiple sheets matched the sheet pattern. Please ensure only one sheet matches."));
    return "";
  }

  // 把工作表填入 ui.cbSheet 中，并默认选中匹配到的一个
  this->updateSheetList(filename);
  QString matchedSheet = QString::fromStdString(sheets.front());;
  int index = ui.cbSheet->findText(matchedSheet);
  if (index >= 0) {
    ui.cbSheet->setCurrentIndex(index);
  } else {
    qWarning() << "Matched sheet not found in combo box:" << matchedSheet;
  }
  // 只ret第一个匹配到的工作表
  return sheets.front();
}

void FemApp::updateSheetList(const std::string &filename) {
  this->ui.cbSheet->clear();
  try {
    app::XlsxProc xlsxproc;
    auto all_sheets = xlsxproc.GetSheetNames(filename);
    for (const auto &sheet: all_sheets) {
      QString qSheetName = QString::fromStdString(sheet);
      this->ui.cbSheet->addItem(qSheetName);
    }
  } catch (const std::exception &e) {
    qWarning() << tr("Failed to read Excel file: ") << e.what();
  }
}

void FemApp::onLoadFile(void) {
  auto folder = getFolderMatched();
  if (folder.empty()) {
    return;
  }
  this->ui.lnFolder->setText(QString::fromStdString(folder));

  // 处理文件名通配符
  auto filename = getFileMatched(folder);
  if (filename.empty()) {
    return;
  }


  // 把工作表填入，并选中匹配到者
  auto sheet = getSheetMatched(filename);
  if (sheet.empty()) {
    return;
  }


  // =======================
  // Dose
  this->ui.cbDMode->setCurrentText(QString::fromStdString(this->femdata.dose.mode));
  this->ui.cbDUnit->setCurrentText(QString::fromStdString(this->femdata.dose.unit));
  this->ui.spnDCenter->setValue(static_cast<int>(this->femdata.dose.center)); // TODO: 需要询问此处数据类型，改控件和代码
  this->ui.dspnDStep->setValue(this->femdata.dose.step);
  // this->ui.lblDNoVal->setText(QString::number(this->femdata.dose.no));
  this->ui.spnDNo->setValue(this->femdata.dose.no);
  this->ui.lnDCols->setText(QString::fromStdString(this->femdata.dose.cols));

  // Focus
  this->ui.cbFMode->setCurrentText(QString::fromStdString(this->femdata.focus.mode));
  this->ui.cbFUnit->setCurrentText(QString::fromStdString(this->femdata.focus.unit));
  this->ui.spnFCenter->setValue(static_cast<int>(this->femdata.focus.center)); // TODO: 需要询问此处数据类型，改控件和代码
  this->ui.dspnFStep->setValue(this->femdata.focus.step);
  // this->ui.lblFNoVal->setText(QString::number(this->femdata.focus.no));
  this->ui.spnFNo->setValue(this->femdata.focus.no);
  this->ui.lnFRows->setText(QString::fromStdString(this->femdata.focus.rows));

  // FEM
  this->ui.cbFEMMode->setCurrentText(QString::fromStdString(this->femdata.fem.mode));
  this->ui.cbFEMUnit->setCurrentText(QString::fromStdString(this->femdata.fem.unit));
  this->ui.spnFEMTarg->setValue(static_cast<int>(this->femdata.fem.target));
  this->ui.spnFEMSpec->setValue(static_cast<int>(this->femdata.fem.spec));
}

FemApp::~FemApp() = default;
