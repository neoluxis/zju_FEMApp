#pragma once

#include <QFileInfo>
#include "ui_mainwindow.h"
#include "cc/neolux/FEMConfig/FEMConfig.h"


class FemApp : public QWidget {
  Q_OBJECT

public:
  explicit FemApp(QWidget *parent = nullptr);

  QFileInfo *femc_info;

  /**
   * Load a FEMConfig file and switch working dir to the file
   */
  void loadFEMConfig();

  void loadFEMConfig(const QString &filePath);

  void onLoadFile();

  ~FemApp() override;


  // ----------- Slots can be defined here if needed -----------
  void onLoadClicked();
  void onSaveClicked();
  void onSaveAsClicked();
  void onBrowseClicked();
  void onMatchClicked();
  void onFolderEdited();
  void onFileChanged();
  void onSheetChanged();
  void onDModeChanged();
  void onDUnitChanged();
  void onDCenterChanged();
  void onDStepChanged();
  void onDColsEdited();
  void onFModeChanged();
  void onFUnitChanged();
  void onFCenterChanged();
  void onFStepChanged();
  void onFRowsEdited();
  void onFEMModeChanged();
  void onFEMUnitChanged();
  void onFEMTargChanged();
  void onFEMSpecChanged();


private:
  Ui::FemApp ui;
  cc::neolux::femconfig::FEMData femdata; // Instance of FEMConfig
};
