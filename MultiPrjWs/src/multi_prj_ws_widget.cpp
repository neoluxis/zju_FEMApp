#include "cc/neolux/fem/mpw/multi_prj_ws_widget.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardItem>
#include <QStandardItemModel>

#include "ui_multiprjws.h"

namespace cc::neolux::fem::mpw {

MultiPrjWsWidget::MultiPrjWsWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::MultiPrjWsWidget) {
    ui->setupUi(this);
    ui->lstProjects->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->lstProjects, &QListView::doubleClicked, this, [this](const QModelIndex& index) {
        if (!index.isValid() || index.row() < 0 || index.row() >= data_.projects.size()) {
            return;
        }

        for (int i = 0; i < data_.projects.size(); ++i) {
            data_.projects[i].enabled = (i == index.row());
        }
        openedProjectIndex_ = index.row();
        refreshProjectList();

        if (index.row() < projectPaths_.size()) {
            emit projectActivated(projectPaths_[index.row()]);
        }
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
    workspaceFilePath_ = QFileInfo(workspaceFilePath).absoluteFilePath();

    openedProjectIndex_ = -1;
    for (int i = 0; i < data_.projects.size(); ++i) {
        if (data_.projects[i].enabled) {
            openedProjectIndex_ = i;
            break;
        }
    }

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

void MultiPrjWsWidget::markProjectOpened(const QString& projectFilePath) {
    const QString absolutePath = QFileInfo(projectFilePath).absoluteFilePath();
    int matchedIndex = -1;
    for (int i = 0; i < data_.projects.size(); ++i) {
        if (resolveProjectPath(data_.projects[i].projectFilePath) == absolutePath) {
            matchedIndex = i;
            break;
        }
    }

    if (matchedIndex < 0) {
        return;
    }

    for (int i = 0; i < data_.projects.size(); ++i) {
        data_.projects[i].enabled = (i == matchedIndex);
    }
    openedProjectIndex_ = matchedIndex;
    refreshProjectList();
}

void MultiPrjWsWidget::addProject() {
    const QFileInfo workspaceInfo(workspaceFilePath_);
    const QDir workspaceDir = workspaceInfo.absoluteDir();
    const QString selectedPath =
        QFileDialog::getOpenFileName(this, tr("Select FEM Project"), workspaceDir.absolutePath(),
                                     tr("FEM Config Files (*.fem);;All Files (*)"));
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
    openedProjectIndex_ = data_.projects.size() - 1;
    refreshProjectList();
}

void MultiPrjWsWidget::removeSelectedProject() {
    const QModelIndex index = ui->lstProjects->currentIndex();
    if (!index.isValid() || index.row() < 0 || index.row() >= data_.projects.size()) {
        return;
    }

    data_.projects.removeAt(index.row());
    if (openedProjectIndex_ == index.row()) {
        openedProjectIndex_ = -1;
    } else if (openedProjectIndex_ > index.row()) {
        --openedProjectIndex_;
    }
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
    const QDir workspaceDir = workspaceInfo.absoluteDir();

    QFileInfo resolvedInfo(workspaceDir.filePath(projectPath));
    return resolvedInfo.absoluteFilePath();
}

void MultiPrjWsWidget::refreshProjectList() {
    projectPaths_.clear();
    auto* model = new QStandardItemModel(ui->lstProjects);

    for (int i = 0; i < data_.projects.size(); ++i) {
        const WorkspaceProjectItem& item = data_.projects[i];
        const QString resolvedPath = resolveProjectPath(item.projectFilePath);
        const QString displayName = item.displayName.isEmpty()
                                        ? QFileInfo(item.projectFilePath).fileName()
                                        : item.displayName;
        auto* listItem = new QStandardItem(displayName);
        listItem->setEditable(false);
        if (i == openedProjectIndex_) {
            listItem->setBackground(QBrush(QColor(220, 220, 220)));
        }
        model->appendRow(listItem);
        projectPaths_.append(resolvedPath);
    }

    ui->lstProjects->setModel(model);
}

}  // namespace cc::neolux::fem::mpw
