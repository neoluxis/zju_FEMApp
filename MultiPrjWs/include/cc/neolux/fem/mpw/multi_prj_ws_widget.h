#pragma once

#include <QVector>
#include <QWidget>

#include "cc/neolux/fem/mpw/multi_project_workspace.h"

namespace Ui {
class MultiPrjWsWidget;
}

namespace cc::neolux::fem::mpw {

class MultiPrjWsWidget : public QWidget {
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

private:
    Ui::MultiPrjWsWidget* ui;
    MultiProjectWorkspaceData data_;
    QString workspaceFilePath_;
    QVector<QString> projectPaths_;
    int openedProjectIndex_ = -1;

    void addProject();
    void removeSelectedProject();
    QString resolveProjectPath(const QString& projectPath) const;
    void refreshProjectList();
};

}  // namespace cc::neolux::fem::mpw
