#include "cc/neolux/fem/mpw/multi_prj_ws_widget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QStringListModel>

#include "ui_multiprjws.h"

namespace cc::neolux::fem::mpw {

MultiPrjWsWidget::MultiPrjWsWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::MultiPrjWsWidget) {
    ui->setupUi(this);

    connect(ui->lstProjects, &QListView::doubleClicked, this, [this](const QModelIndex& index) {
        if (!index.isValid() || index.row() < 0 || index.row() >= projectPaths_.size()) {
            return;
        }
        emit projectActivated(projectPaths_[index.row()]);
    });

    connect(ui->btnAddProject, &QPushButton::clicked, this, [this]() { addProject(); });
    connect(ui->btnRemoveProject, &QPushButton::clicked, this,
            [this]() { removeSelectedProject(); });
}

MultiPrjWsWidget::~MultiPrjWsWidget() {
    delete ui;
}

bool MultiPrjWsWidget::loadWorkspaceFile(const QString& workspaceFilePath, QString* errorMessage) {
    MultiProjectWorkspaceData data;
    if (!MultiProjectWorkspace::ReadFile(workspaceFilePath, data, errorMessage)) {
        return false;
    }

    setWorkspaceData(data, workspaceFilePath);
    return true;
}

void MultiPrjWsWidget::setWorkspaceData(const MultiProjectWorkspaceData& data,
                                        const QString& workspaceFilePath) {
    data_ = data;
    workspaceFilePath_ = workspaceFilePath;
    refreshProjectList();
}

QString MultiPrjWsWidget::currentProjectPath() const {
    const QModelIndex index = ui->lstProjects->currentIndex();
    if (!index.isValid() || index.row() < 0 || index.row() >= projectPaths_.size()) {
        return QString();
    }
    return projectPaths_[index.row()];
}

QString MultiPrjWsWidget::firstEnabledProjectPath() const {
    for (const WorkspaceProjectItem& project : data_.projects) {
        if (!project.enabled) {
            continue;
        }
        const QString resolved = resolveProjectPath(project.projectFilePath);
        if (!resolved.isEmpty()) {
            return resolved;
        }
    }
    return QString();
}

void MultiPrjWsWidget::addProject() {
    const QFileInfo workspaceInfo(workspaceFilePath_);
    const QString basePath = data_.baseDir.isEmpty() ? workspaceInfo.absolutePath() : data_.baseDir;
    const QString selectedPath = QFileDialog::getOpenFileName(
        this, tr("Select FEM Project"), basePath, tr("FEM Config Files (*.fem);;All Files (*)"));
    if (selectedPath.isEmpty()) {
        return;
    }

    const QString absolutePath = QFileInfo(selectedPath).absoluteFilePath();
    for (const WorkspaceProjectItem& item : data_.projects) {
        if (resolveProjectPath(item.projectFilePath) == absolutePath) {
            return;
        }
    }

    WorkspaceProjectItem item;
    item.projectFilePath = absolutePath;
    item.displayName = QFileInfo(absolutePath).completeBaseName();
    item.enabled = true;
    data_.projects.append(item);
    refreshProjectList();
}

void MultiPrjWsWidget::removeSelectedProject() {
    const QModelIndex index = ui->lstProjects->currentIndex();
    if (!index.isValid() || index.row() < 0 || index.row() >= data_.projects.size()) {
        return;
    }

    data_.projects.removeAt(index.row());
    refreshProjectList();
}

QString MultiPrjWsWidget::resolveProjectPath(const QString& projectPath) const {
    if (projectPath.isEmpty()) {
        return QString();
    }

    QFileInfo pathInfo(projectPath);
    if (pathInfo.isAbsolute()) {
        return pathInfo.absoluteFilePath();
    }

    const QFileInfo workspaceInfo(workspaceFilePath_);
    const QString basePath = data_.baseDir.isEmpty() ? workspaceInfo.absolutePath() : data_.baseDir;

    QFileInfo resolvedInfo(QFileInfo(basePath).absoluteFilePath() + "/" + projectPath);
    return resolvedInfo.absoluteFilePath();
}

void MultiPrjWsWidget::refreshProjectList() {
    QStringList listTexts;
    projectPaths_.clear();

    for (const WorkspaceProjectItem& item : data_.projects) {
        const QString resolvedPath = resolveProjectPath(item.projectFilePath);
        const QString displayName = item.displayName.isEmpty()
                                        ? QFileInfo(item.projectFilePath).fileName()
                                        : item.displayName;
        const QString flag = item.enabled ? QStringLiteral("[On]") : QStringLiteral("[Off]");

        listTexts.append(QString("%1 %2").arg(flag, displayName));
        projectPaths_.append(resolvedPath);
    }

    auto* model = new QStringListModel(listTexts, ui->lstProjects);
    ui->lstProjects->setModel(model);
}

}  // namespace cc::neolux::fem::mpw
