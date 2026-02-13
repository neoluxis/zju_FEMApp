#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QTranslator>
#include <QLocale>


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
    FileAssociation::CleanRegister(
      "fem",
      "FemApp.Document",
      "FEM Analysis File");
    qDebug() << ".fem association registered";
  }
  else
  {
    qDebug() << "Unregistering .fem file association...";
    FileAssociation::UnregisterAssociation("fem");
    qDebug() << ".fem association unregistered";
  }
#endif
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  fileAssociat();
  QLocale locale = QLocale::system(); // 自动检测系统语言
  QString languageCode = locale.name(); // 例如 "zh_CN" 或 "en_US"
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

  // // // If command line argument provided, open the file
  // if (argc > 1)
  // {
  //     cout << "Opening file: " << argv[1] << endl;

  //     try
  //     {
  //         QString qFilePath = QString::fromLocal8Bit(argv[1]);

  //         // Convert to absolute path
  //         QFileInfo fileInfo(qFilePath);
  //         QString absolutePath = fileInfo.absoluteFilePath();

  //         cout << "File path: " << absolutePath.toStdString() << endl;

  //         MainWindow *window = new MainWindow();
  //         window->setAttribute(Qt::WA_DeleteOnClose);
  //         window->setAttribute(Qt::WA_QuitOnClose);

  //         // Show window first, then load file
  //         window->show();
  //         window->raise();
  //         window->activateWindow();

  //         // Load file
  //         cout << "Loading FEM file..." << endl;
  //         window->loadFEMFile(absolutePath);

  //         return app.exec();
  //     }
  //     catch (const std::exception &e)
  //     {
  //         std::cerr << "[ERROR] Exception: " << e.what() << std::endl;
  //         return 1;
  //     }
  //     catch (...)
  //     {
  //         std::cerr << "[ERROR] Unknown exception" << std::endl;
  //         return 1;
  //     }
  // }

  // // No arguments - show empty window
  // cout << "No file argument, showing empty window" << endl;
  // MainWindow *window = new MainWindow();
  // window->setAttribute(Qt::WA_DeleteOnClose);
  // window->setAttribute(Qt::WA_QuitOnClose);
  // window->show();
  // window->raise();
  // window->activateWindow();

  // return app.exec();
}
