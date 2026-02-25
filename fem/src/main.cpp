#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QTranslator>
#include <iostream>
#include <string>

#ifndef FEMAPP_SKIP_WINDOWS_API
#include "cc/neolux/fem/FileAssociation.h"
#endif
#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "femapp.h"

#ifndef FEMAPP_SKIP_WINDOWS_API
using cc::neolux::fem::FileAssociation;
#endif
using namespace cc::neolux::femconfig;
using std::cout;
using std::endl;
using std::string;

void fileAssociat() {
#ifndef FEMAPP_SKIP_WINDOWS_API
    // 自动注册打开方式，如果没有注册

    if (!FileAssociation::IsAssociated("fem")) {
        qDebug() << "Registering .fem file association...";
        FileAssociation::CleanRegister("fem", "FemApp.Document", "FEM Analysis File");
        qDebug() << ".fem association registered";
    } else {
        // qDebug() << "Unregistering .fem file association...";
        // FileAssociation::UnregisterAssociation("fem");
        // qDebug() << ".fem association unregistered";
    }
#endif
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    fileAssociat();
    QLocale locale = QLocale::system();    // 自动检测系统语言
    QString languageCode = locale.name();  // 例如 "zh_CN" 或 "en_US"
    qDebug() << "System locale detected:" << languageCode;

    QTranslator translator;

    QString qmFile = QString("translations/femapp.%1.qm").arg(languageCode);
    if (!translator.load(qmFile)) {
        auto _ = translator.load("translations/femapp.en_US.qm");
    }

    QApplication::installTranslator(&translator);

    FemApp w;
    w.show();

    if (argc > 1) {
        QString qFilePath = QString::fromLocal8Bit(argv[1]);
        w.femc_info = new QFileInfo(qFilePath);
        w.loadFEMConfig();
    }

    return app.exec();
}
