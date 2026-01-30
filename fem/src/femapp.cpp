#include "femapp.h"
#include "iostream"
#include "QDebug"

using cc::neolux::femconfig::FEMConfig;

FemApp::FemApp(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void FemApp::loadFEMConfig(void)
{
    this->loadFEMConfig(
        this->femc_info->absoluteFilePath());
}

void FemApp::loadFEMConfig(const QString &filePath)
{
    if (filePath.isEmpty())
    {
        // raise a error window here
        qWarning() << "file path is Empty!\n";
        return;
    }
    qDebug() << "Loading config file from " << filePath << "\n";
    if (!FEMConfig::ReadFile(filePath.toStdString(), this->femdata))
    {
        qDebug() << "Failed to read file.";
        return;
    }
    this->onLoadFile();
}

void FemApp::onLoadFile(void)
{
    ui.lnFolder->setText(QString::fromLocal8Bit(femdata.folderPattern));
}

FemApp::~FemApp() = default;
