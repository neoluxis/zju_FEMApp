#pragma once

#include <QVector>
#include <QWidget>

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef MULTIPRJWS_EXPORTS
#define MULTIPRJWS_API __declspec(dllexport)
#else
#define MULTIPRJWS_API __declspec(dllimport)
#endif
#else
#define MULTIPRJWS_API
#endif

#include "cc/neolux/fem/mpw/multi_project_workspace.h"

namespace Ui {
class MultiPrjWsWidget;
}

namespace cc::neolux::fem::mpw {

class MULTIPRJWS_API MultiPrjWsWidget : public QWidget {
    Q_OBJECT

public:
    explicit MultiPrjWsWidget(QWidget* parent = nullptr);
    ~MultiPrjWsWidget() override;

    bool loadWorkspaceFile(const QString& workspaceFilePath, QString* errorMessage = nullptr);
    void setWorkspaceData(const MultiProjectWorkspaceData& data, const QString& workspaceFilePath);

    QString currentProjectPath() const;
    QString firstEnabledProjectPath() const;
    void markProjectOpened(const QString& projectFilePath);

signals:
    void projectActivated(const QString& projectFilePath);
    void configRequested();

private:
    Ui::MultiPrjWsWidget* ui;
    MultiProjectWorkspaceData data_;
    QString workspaceFilePath_;
    QVector<QString> projectPaths_;
    int openedProjectIndex_ = -1;

    QString resolveProjectPath(const QString& projectPath) const;
    void refreshProjectList();
};

}  // namespace cc::neolux::fem::mpw
