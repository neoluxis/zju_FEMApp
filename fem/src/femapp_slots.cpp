//
// Created by neolux on 2026/1/31.
//

#include "femapp.h"
#include "string"
#include "sstream"
#include "QDebug"
#include "QFileDialog"
#include "QDir"


void FemApp::onLoadClicked() {
  QString femconfig_path = QFileDialog::getOpenFileName(
    this,
    tr("Open FEM Config File"),
    QDir::currentPath(),
    tr("FEM Config Files (*.fem);;All Files (*)"));
  if (femconfig_path.isEmpty()) {
    return;
  }
  this->loadFEMConfig(femconfig_path);
  qInfo() << "Loaded FEM config file from " << femconfig_path;
}

void FemApp::onSaveClicked() {
  // Update raw content by dumping current femdata
  std::ostringstream oss;
  if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata, oss)) {
    showError(this, tr("Failed to generate FEM config content."));
    return;
  }
  femdata.rawContent = oss.str();
  ui.txtConfigRaw->setPlainText(QString::fromUtf8(femdata.rawContent.c_str()));

  // Save to file
  if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata,
                                                     this->femc_info->absoluteFilePath().toUtf8().toStdString())) {
    showError(this, tr("Failed to save FEM config file."));
    return;
  }
  clearModifiedFlag();
  qInfo() << "Saved FEM config file to " << this->femc_info->absoluteFilePath();
}

void FemApp::onSaveAsClicked() {
  QString femconfig_path = QFileDialog::getSaveFileName(
    this,
    tr("Save FEM Config File As"),
    QDir::currentPath(),
    tr("FEM Config Files (*.fem);;All Files (*)"));
  if (femconfig_path.isEmpty()) {
    return;
  }

  // Update raw content by dumping current femdata
  std::ostringstream oss;
  if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata, oss)) {
    showError(this, tr("Failed to generate FEM config content."));
    return;
  }
  femdata.rawContent = oss.str();
  ui.txtConfigRaw->setPlainText(QString::fromUtf8(femdata.rawContent.c_str()));

  // Save to file
  if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata,
                                                     femconfig_path.toUtf8().toStdString())) {
    showError(this, tr("Failed to save FEM config file."));
    return;
  }
  currentFilePath = femconfig_path;
  clearModifiedFlag();
  qInfo() << "Saved FEM config file to " << femconfig_path;
}

void FemApp::onBrowseClicked() {
  QString dir = QFileDialog::getExistingDirectory(
    this,
    tr("Select Folder"),
    QDir::currentPath(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (dir.isEmpty()) {
    return;
  }
  // get relative path
  QDir cwd = QDir::current();
  QString relativePath = cwd.relativeFilePath(dir);
  this->ui.lnFolder->setText(relativePath);
  femdata.folderPattern = relativePath.toUtf8().toStdString();
  updateFileList(relativePath.toUtf8().toStdString());
  qInfo() << "Folder selected: " << dir;
}

void FemApp::onMatchClicked() {
  std::string folder = getFolderMatched();
  if (folder.empty()) {
    return;
  }
  this->ui.lnFolder->setText(QString::fromUtf8(folder.c_str()));
  femdata.folderPattern = folder;
  updateFileList(folder);
  qInfo() << "Folder matched: " << QString::fromUtf8(folder.c_str());
}

void FemApp::onFolderEdited(const QString &text) {
  femdata.folderPattern = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Folder pattern changed to " << text;
}


void FemApp::onFileChanged(const QString &text) {
  femdata.filenamePattern = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "File pattern changed to " << text;

  // Skip automatic sheet matching during file loading to avoid errors
  // The loading process (onLoadFile) handles sheet matching itself
  if (isLoading) {
    return;
  }

  // Only update sheet list if a file is selected
  QString selectedFile = getCurrentSelectedFile();
  if (!selectedFile.isEmpty()) {
    auto sheet = getSheetMatched(selectedFile.toUtf8().toStdString());
    if (!sheet.empty()) {
      qInfo() << "Sheet auto-selected: " << QString::fromUtf8(sheet.c_str());
    }
  }
}

void FemApp::onSheetChanged(const QString &text) {
  femdata.sheetPattern = text.toUtf8().toStdString();
  markAsModified();
}

void FemApp::onDModeChanged(const QString &text) {
  femdata.dose.mode = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Dose mode changed to " << text;
}

void FemApp::onDUnitChanged(const QString &text) {
  femdata.dose.unit = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Dose unit changed to " << text;
}

void FemApp::onDCenterChanged(double value) {
  femdata.dose.center = value;
  markAsModified();
  qInfo() << "Dose center changed to " << value;
}

void FemApp::onDStepChanged(double value) {
  femdata.dose.step = value;
  markAsModified();
  qInfo() << "Dose step changed to " << value;
}

void FemApp::onDNoChanged(int value) {
  femdata.dose.no = value;
  markAsModified();
  qInfo() << "Dose no changed to " << value;
}

void FemApp::onDColsEdited() {
  auto text = ui.lnDCols->text();
  femdata.dose.cols = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Dose cols changed to " << text;
}

void FemApp::onFModeChanged(const QString &text) {
  femdata.focus.mode = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Focus mode changed to " << text;
}

void FemApp::onFUnitChanged(const QString &text) {
  femdata.focus.unit = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Focus unit changed to " << text;
}

void FemApp::onFCenterChanged(double value) {
  femdata.focus.center = value;
  markAsModified();
  qInfo() << "Focus center changed to " << value;
}

void FemApp::onFStepChanged(double value) {
  femdata.focus.step = value;
  markAsModified();
  qInfo() << "Focus step changed to " << value;
}

void FemApp::onFNoChanged(int value) {
  femdata.focus.no = value;
  markAsModified();
  qInfo() << "Focus no changed to " << value;
}

void FemApp::onFRowsEdited() {
  auto text = ui.lnFRows->text();
  femdata.focus.rows = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "Focus rows changed to " << text;
}

void FemApp::onFEMModeChanged(const QString &text) {
  femdata.fem.mode = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "FEM mode changed to " << text;
}

void FemApp::onFEMUnitChanged(const QString &text) {
  femdata.fem.unit = text.toUtf8().toStdString();
  markAsModified();
  qInfo() << "FEM unit changed to " << text;
}

void FemApp::onFEMTargChanged(double value) {
  femdata.fem.target = value;
  markAsModified();
  qInfo() << "FEM target changed to " << value;
}

void FemApp::onFEMSpecChanged(double value) {
  femdata.fem.spec = value;
  markAsModified();
  qInfo() << "FEM spec changed to " << value;
}

void FemApp::onRawFileEdited() {
  markAsModified();
}


void FemApp::onTxtResetClicked() {
  ui.txtConfigRaw->setPlainText(QString::fromUtf8(this->femdata.rawContent.c_str()));
}

void FemApp::onTxtApplyClicked() {
  std::string text = ui.txtConfigRaw->toPlainText().toUtf8().toStdString();
  auto *newdata = new cc::neolux::femconfig::FEMData();
  if (!cc::neolux::femconfig::FEMConfig::ParseContent(text, *newdata)) {
    showError(this, tr("Failed to parse raw FEM config content."));
    delete newdata;
    return;
  }
  this->femdata = *newdata;
  delete newdata;
  this->onLoadFile();
}
