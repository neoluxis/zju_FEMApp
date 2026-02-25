#pragma once

#include <QDialog>

#include "cc/neolux/fem/mpw/multi_project_workspace.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef MULTIPRJWS_EXPORTS
#define MULTIPRJWS_API __declspec(dllexport)
#else
#define MULTIPRJWS_API __declspec(dllimport)
#endif
#else
#define MULTIPRJWS_API
#endif

namespace Ui {
class MultiPrjWsConfigDialog;
}

namespace cc::neolux::fem::mpw {

class MULTIPRJWS_API MultiPrjWsConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit MultiPrjWsConfigDialog(QWidget* parent = nullptr);
    ~MultiPrjWsConfigDialog() override;

    bool loadWorkspace(const QString& workspaceFilePath, QString* errorMessage = nullptr);

signals:
    void workspaceSaved(const QString& workspaceFilePath);

private:
    Ui::MultiPrjWsConfigDialog* ui;
    MultiProjectWorkspaceData data_;
    QString workspaceFilePath_;

    void refreshProjectList();
    void refreshEditors();
    void addProject();
    void removeSelectedProject();
    bool saveWorkspace();
    int currentIndex() const;
};

}  // namespace cc::neolux::fem::mpw
