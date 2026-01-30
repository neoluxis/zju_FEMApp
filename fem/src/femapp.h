#pragma once

#include <QWidget>
#include <QFileInfo>
#include "ui_mainwindow.h"
#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "string"


class FemApp : public QWidget
{
    Q_OBJECT

public:
    explicit FemApp(QWidget *parent = nullptr);

    QFileInfo *femc_info;

    /**
     * Load a FEMConfig file and switch working dir to the file
     */
    void loadFEMConfig(void);
    void loadFEMConfig(const QString &filePath);

    void onLoadFile(void);

    ~FemApp() override;

private:
    Ui::FemApp ui;
    cc::neolux::femconfig::FEMData femdata; // Instance of FEMConfig

};
