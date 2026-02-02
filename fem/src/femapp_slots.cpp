//
// Created by neolux on 2026/1/31.
//

#include "femapp.h"
#include "string"
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
  if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata,
                                                     this->femc_info->absoluteFilePath().toStdString())) {
    showError(this, tr("Failed to save FEM config file."));
  }
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
  if (!cc::neolux::femconfig::FEMConfig::dumpFEMData(femdata,
                                                     femconfig_path.toStdString())) {
    showError(this, tr("Failed to save FEM config file."));
  }
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
  femdata.folderPattern = relativePath.toStdString();
  updateFileList(relativePath.toStdString());
  qInfo() << "Folder selected: " << dir;
}

void FemApp::onMatchClicked() {
  std::string folder = getFolderMatched();
  if (folder.empty()) {
    return;
  }
  this->ui.lnFolder->setText(QString::fromStdString(folder));
  femdata.folderPattern = folder;
  updateFileList(folder);
  qInfo() << "Folder matched: " << QString::fromStdString(folder);
}

void FemApp::onFolderEdited(const QString &text) {
  femdata.folderPattern = text.toStdString();
  qInfo() << "Folder pattern changed to " << text;
}


void FemApp::onFileChanged(const QString &text) {
  femdata.filenamePattern = text.toStdString();
  qInfo() << "File pattern changed to " << text;
  updateSheetList(getCurrentSelectedFile().toStdString());
}

void FemApp::onSheetChanged(const QString &text) {
  femdata.sheetPattern = text.toStdString();
}

void FemApp::onDModeChanged(const QString &text) {
  femdata.dose.mode = text.toStdString();
  qInfo() << "Dose mode changed to " << text;
}

void FemApp::onDUnitChanged(const QString &text) {
  femdata.dose.unit = text.toStdString();
  qInfo() << "Dose unit changed to " << text;
}

void FemApp::onDCenterChanged(int value) {
  femdata.dose.center = static_cast<double>(value);
  qInfo() << "Dose center changed to " << value;
}

void FemApp::onDStepChanged(double value) {
  femdata.dose.step = value;
  qInfo() << "Dose step changed to " << value;
}

void FemApp::onDNoChanged(int value) {
  femdata.dose.no = value;
  qInfo() << "Dose no changed to " << value;
}

void FemApp::onDColsEdited() {
  auto text = ui.lnDCols->text();
  femdata.dose.cols = text.toStdString();
  qInfo() << "Dose cols changed to " << text;
}

void FemApp::onFModeChanged(const QString &text) {
  femdata.focus.mode = text.toStdString();
  qInfo() << "Focus mode changed to " << text;
}

void FemApp::onFUnitChanged(const QString &text) {
  femdata.focus.unit = text.toStdString();
  qInfo() << "Focus unit changed to " << text;
}

void FemApp::onFCenterChanged(int value) {
  femdata.focus.center = static_cast<double>(value);
  qInfo() << "Focus center changed to " << value;
}

void FemApp::onFStepChanged(double value) {
  femdata.focus.step = value;
  qInfo() << "Focus step changed to " << value;
}

void FemApp::onFNoChanged(int value) {
  femdata.focus.no = value;
  qInfo() << "Focus no changed to " << value;
}

void FemApp::onFRowsEdited() {
  auto text = ui.lnFRows->text();
  femdata.focus.rows = text.toStdString();
  qInfo() << "Focus rows changed to " << text;
}

void FemApp::onFEMModeChanged(const QString &text) {
  femdata.fem.mode = text.toStdString();
  qInfo() << "FEM mode changed to " << text;
}

void FemApp::onFEMUnitChanged(const QString &text) {
  femdata.fem.unit = text.toStdString();
  qInfo() << "FEM unit changed to " << text;
}

void FemApp::onFEMTargChanged(int value) {
  femdata.fem.target = static_cast<double>(value);
  qInfo() << "FEM target changed to " << value;
}

void FemApp::onFEMSpecChanged(int value) {
  femdata.fem.spec = static_cast<double>(value);
  qInfo() << "FEM spec changed to " << value;
}
