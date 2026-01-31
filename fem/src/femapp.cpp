#include "femapp.h"
#include "iostream"
#include "QDebug"
#include "filesystem"

#include "cc/neolux/fem/xlsx_proc.h"

#include <QMessageBox>

using cc::neolux::femconfig::FEMConfig;
namespace fs = std::filesystem;
namespace app = cc::neolux::fem;

void showError(QWidget *parent, const QString &text) {
  QMessageBox::critical(parent, "Error", text, QMessageBox::Ok);
}

void showWarning(QWidget *parent, const QString &text) {
  QMessageBox::warning(parent, "Warning", text, QMessageBox::Ok);
}

void showInfo(QWidget *parent, const QString &text) {
  QMessageBox::information(parent, "Info", text, QMessageBox::Ok);
}


FemApp::FemApp(QWidget *parent)
  : QWidget(parent) {
  ui.setupUi(this);

  QObject::connect(this->ui.btnLoad, SIGNAL(clicked()), this, SLOT(onLoadClicked()));
  QObject::connect(this->ui.btnSave, SIGNAL(clicked()), this, SLOT(onSaveClicked()));
  QObject::connect(this->ui.btnSaveas, SIGNAL(clicked()), this, SLOT(onSaveAsClicked()));
  QObject::connect(this->ui.btnFolderBrowse, SIGNAL(clicked()), this, SLOT(onBrowseClicked()));
  QObject::connect(this->ui.btnFolderMatch, SIGNAL(clicked()), this, SLOT(onMatchClicked()));
  QObject::connect(this->ui.lnFolder, SIGNAL(textEdited(QString)), this, SLOT(onFolderEdited()));
  QObject::connect(this->ui.cbFile, SIGNAL(currentTextChanged(QString)), this, SLOT(onFileChanged()));
  QObject::connect(this->ui.cbSheet, SIGNAL(currentTextChanged(QString)), this, SLOT(onSheetChanged()));

  QObject::connect(this->ui.cbDMode, SIGNAL(currentTextChanged(QString)), this, SLOT(onDModeChanged()));
  QObject::connect(this->ui.cbDUnit, SIGNAL(currentTextChanged(QString)), this, SLOT(onDUnitChanged()));
  QObject::connect(this->ui.spnDCenter, SIGNAL(valueChanged(int)), this, SLOT(onDCenterChanged()));
  QObject::connect(this->ui.dspnDStep, SIGNAL(valueChanged(double)), this, SLOT(onDStepChanged()));
  QObject::connect(this->ui.lnDCols, SIGNAL(textEdited(QString)), this, SLOT(onDColsEdited()));

  QObject::connect(this->ui.cbFMode, SIGNAL(currentTextChanged(QString)), this, SLOT(onFModeChanged()));
  QObject::connect(this->ui.cbFUnit, SIGNAL(currentTextChanged(QString)), this, SLOT(onFUnitChanged()));
  QObject::connect(this->ui.spnFCenter, SIGNAL(valueChanged(int)), this, SLOT(onFCenterChanged()));
  QObject::connect(this->ui.dspnFStep, SIGNAL(valueChanged(double)), this, SLOT(onFStepChanged()));
  QObject::connect(this->ui.lnFRows, SIGNAL(textEdited(QString)), this, SLOT(onFRowsEdited()));

  QObject::connect(this->ui.cbFEMMode, SIGNAL(currentTextChanged(QString)), this, SLOT(onFEMModeChanged()));
  QObject::connect(this->ui.cbFEMUnit, SIGNAL(currentTextChanged(QString)), this, SLOT(onFEMUnitChanged()));
  QObject::connect(this->ui.spnFEMTarg, SIGNAL(valueChanged(int)), this, SLOT(onFEMTargChanged()));
  QObject::connect(this->ui.spnFEMSpec, SIGNAL(valueChanged(int)), this, SLOT(onFEMSpecChanged()));

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

void FemApp::onLoadFile(void) {
  auto folders = FEMConfig::ExpandFolderPattern(this->femdata);
  qDebug() << "Expanded Folders:" << folders.size();
  for (const auto &folder: folders) {
    qDebug() << "  " << QString::fromStdString(folder);
  }
  if (folders.empty()) {
    showError(this, tr("No folders matched the folder pattern. "));
    return;
  }
  if (folders.size() > 1) {
    showError(this, tr("Multiple folders matched the folder pattern. Please ensure only one folder matches."));
    return;
  }
  // 只处理第一个匹配到的文件夹
  auto folder = folders.front();
  this->ui.lnFolder->setText(QString::fromStdString(folder));

  // 处理文件名通配符
  auto filenames = FEMConfig::ExpandFilenamePattern(folder, this->femdata);
  qDebug() << "Expanded Filenames:" << filenames.size();
  for (const auto &filename: filenames) {
    qDebug() << "  " << QString::fromStdString(filename);
  }
  if (filenames.empty()) {
    showError(this, tr("No filenames matched the filename pattern. "));
    return;
  }
  if (filenames.size() > 1) {
    showError(this, tr("Multiple filenames matched the filename pattern. Please ensure only one filename matches."));
    return;
  }

  // 把文件夹中所有文件(include but not only matched), 添加到 ui.cbFile中，并默认选中匹配到的一个
  this->ui.cbFile->clear();
  for (const auto &entry: fs::directory_iterator(folder)) {
    if (!fs::is_regular_file(entry.status()))
      continue;
    QString qFilename = QString::fromStdString(entry.path().filename().u8string());
    this->ui.cbFile->addItem(qFilename);
    if (entry.path().string() == filenames.front()) {
      this->ui.cbFile->setCurrentText(qFilename);
    }
  }

  // 把工作表填入，并选中匹配到者
  auto sheets = cc::neolux::femconfig::FEMConfig::ExpandSheetPattern(
    filenames.front(),
    this->femdata);
  if (sheets.empty()) {
    showError(this, tr("No sheets matched the sheet pattern. "));
    return;
  }
  if (sheets.size() > 1) {
    showError(this, tr("Multiple sheets matched the sheet pattern. Please ensure only one sheet matches."));
    return;
  }

  this->ui.cbSheet->clear();
  // 获取所有工作表名称
  auto allSheets = cc::neolux::fem::XlsxProc::GetSheetNames(filenames.front());
  for (const auto &sheetName: allSheets) {
    QString qSheetName = QString::fromStdString(sheetName);
    this->ui.cbSheet->addItem(qSheetName);
    if (sheetName == sheets.front()) {
      this->ui.cbSheet->setCurrentText(qSheetName);
    }
  }

  // =======================
  // Dose
  this->ui.cbDMode->setCurrentText(QString::fromStdString(this->femdata.dose.mode));
  this->ui.cbDUnit->setCurrentText(QString::fromStdString(this->femdata.dose.unit));
  this->ui.spnDCenter->setValue(static_cast<int>(this->femdata.dose.center)); // TODO: 需要询问此处数据类型，改控件和代码
  this->ui.dspnDStep->setValue(this->femdata.dose.step);
  this->ui.lblDNoVal->setText(QString::number(this->femdata.dose.no));
  this->ui.lnDCols->setText(QString::fromStdString(this->femdata.dose.cols));

  // Focus
  this->ui.cbFMode->setCurrentText(QString::fromStdString(this->femdata.focus.mode));
  this->ui.cbFUnit->setCurrentText(QString::fromStdString(this->femdata.focus.unit));
  this->ui.spnFCenter->setValue(static_cast<int>(this->femdata.focus.center)); // TODO: 需要询问此处数据类型，改控件和代码
  this->ui.dspnFStep->setValue(this->femdata.focus.step);
  this->ui.lblFNoVal->setText(QString::number(this->femdata.focus.no));
  this->ui.lnFRows->setText(QString::fromStdString(this->femdata.focus.rows));

  // FEM
  this->ui.cbFEMMode->setCurrentText(QString::fromStdString(this->femdata.fem.mode));
  this->ui.cbFEMUnit->setCurrentText(QString::fromStdString(this->femdata.fem.unit));
  this->ui.spnFEMTarg->setValue(static_cast<int>(this->femdata.fem.target));
  this->ui.spnFEMSpec->setValue(static_cast<int>(this->femdata.fem.spec));
}

FemApp::~FemApp() = default;
