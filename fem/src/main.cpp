#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

#ifndef FEMAPP_SKIP_WINDOWS_API
#include "cc/neolux/fem/FileAssociation.h"
#endif
#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "cc/neolux/fem/mpw/multi_project_workspace.h"
#include "cc/neolux/fem/recent/recent_project_history.h"
#include "femapp.h"

#ifndef FEMAPP_SKIP_WINDOWS_API
using cc::neolux::fem::FileAssociation;
#endif
using namespace cc::neolux::femconfig;

void fileAssociat(std::string ext, std::string progId = "FemApp.Document",
                  std::string description = "FEM Analysis File") {
#ifndef FEMAPP_SKIP_WINDOWS_API
    // 自动注册打开方式，如果没有注册

    if (!FileAssociation::IsAssociated(ext.c_str())) {
        qDebug() << "Registering ." << ext.c_str() << " file association...";
        FileAssociation::CleanRegister(ext.c_str(), progId.c_str(), description.c_str());
        qDebug() << "." << ext.c_str() << " association registered";
    } else {
        // qDebug() << "Unregistering .fem file association...";
        // FileAssociation::UnregisterAssociation("fem");
        // qDebug() << ".fem association unregistered";
    }
#endif
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    fileAssociat("fem", "FemApp.Document", "FEM Analysis File");  // project
    fileAssociat("femmpw", "FemApp.MultiProjectWorkspace",
                 "FEM Multi-Project Workspace");  // multi proj ws
    QLocale locale = QLocale::system();           // 自动检测系统语言
    QString languageCode = locale.name();         // 例如 "zh_CN" 或 "en_US"
    qDebug() << "System locale detected:" << languageCode;

    QTranslator translator;

    QString qmFile = QString("translations/femapp.%1.qm").arg(languageCode);
    if (!translator.load(qmFile)) {
        auto _ = translator.load("translations/femapp.en_US.qm");
    }

    QApplication::installTranslator(&translator);

    FemApp w;
    w.show();

    cc::neolux::fem::recent::RecentProjectHistory recentHistory;
    if (argc > 1) {
        QString qFilePath = QString::fromLocal8Bit(argv[1]);
        if (cc::neolux::fem::mpw::MultiProjectWorkspace::IsValidWorkspaceFile(qFilePath)) {
            w.loadMultiProjectWorkspace(qFilePath);
        } else {
            w.openSingleProject(qFilePath);
        }
    } else {
        const QString latestProject = recentHistory.latestProject();
        if (!latestProject.isEmpty()) {
            if (cc::neolux::fem::mpw::MultiProjectWorkspace::IsValidWorkspaceFile(latestProject)) {
                w.loadMultiProjectWorkspace(latestProject);
            } else {
                w.openSingleProject(latestProject);
            }
        }
    }

    return app.exec();
}
