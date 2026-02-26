#include "cc/neolux/fem/global_menu_controller.h"

#include <QAction>
#include <QCoreApplication>
#include <QMenu>
#include <QMenuBar>
#include <QSignalBlocker>

namespace cc::neolux::fem {

GlobalMenuController::GlobalMenuController(QMenuBar* menuBar, QObject* parent) : QObject(parent) {
    auto* fileMenu = menuBar->addMenu(QCoreApplication::translate("GlobalMenuController", "File"));
    auto* newMenu = fileMenu->addMenu(QCoreApplication::translate("GlobalMenuController", "New"));
    auto* actionNewProject =
        newMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Project"));
    auto* actionNewWorkspace =
        newMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Workspace"));
    fileMenu->addSeparator();

    auto* actionOpenProject =
        fileMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Open Project..."));
    workspaceConfigAction_ = fileMenu->addAction(
        QCoreApplication::translate("GlobalMenuController", "Workspace Config..."));
    recentMenu_ = fileMenu->addMenu(QCoreApplication::translate("GlobalMenuController", "Recent"));
    auto* actionSaveProject =
        fileMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Save"));
    auto* actionSaveProjectAs =
        fileMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Save As..."));
    fileMenu->addSeparator();
    auto* actionExit =
        fileMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Exit"));

    auto* viewMenu = menuBar->addMenu(QCoreApplication::translate("GlobalMenuController", "View"));
    toggleProjectTabsAction_ = viewMenu->addAction(
        QCoreApplication::translate("GlobalMenuController", "Show Project Tabs"));
    toggleProjectTabsAction_->setCheckable(true);
    toggleProjectTabsAction_->setChecked(true);

    auto* appMenu = menuBar->addMenu(QCoreApplication::translate("GlobalMenuController", "App"));
    auto* actionAbout =
        appMenu->addAction(QCoreApplication::translate("GlobalMenuController", "About"));
    auto* actionClearCache =
        appMenu->addAction(QCoreApplication::translate("GlobalMenuController", "Clear Cache"));

    connect(actionNewProject, &QAction::triggered, this,
            &GlobalMenuController::newProjectRequested);
    connect(actionNewWorkspace, &QAction::triggered, this,
            &GlobalMenuController::newWorkspaceRequested);
    connect(actionOpenProject, &QAction::triggered, this, &GlobalMenuController::openRequested);
    connect(workspaceConfigAction_, &QAction::triggered, this,
            &GlobalMenuController::workspaceConfigRequested);
    connect(actionSaveProject, &QAction::triggered, this, &GlobalMenuController::saveRequested);
    connect(actionSaveProjectAs, &QAction::triggered, this, &GlobalMenuController::saveAsRequested);
    connect(actionExit, &QAction::triggered, this, &GlobalMenuController::exitRequested);
    connect(actionAbout, &QAction::triggered, this, &GlobalMenuController::aboutRequested);
    connect(actionClearCache, &QAction::triggered, this,
            &GlobalMenuController::clearCacheRequested);
    connect(toggleProjectTabsAction_, &QAction::toggled, this,
            &GlobalMenuController::projectTabsToggled);
}

void GlobalMenuController::setWorkspaceConfigEnabled(bool enabled) {
    if (workspaceConfigAction_) {
        workspaceConfigAction_->setEnabled(enabled);
    }
}

void GlobalMenuController::setProjectTabsChecked(bool checked, bool emitSignal) {
    if (!toggleProjectTabsAction_) {
        return;
    }

    if (emitSignal) {
        toggleProjectTabsAction_->setChecked(checked);
        return;
    }

    const QSignalBlocker blocker(toggleProjectTabsAction_);
    toggleProjectTabsAction_->setChecked(checked);
}

bool GlobalMenuController::isProjectTabsChecked() const {
    if (!toggleProjectTabsAction_) {
        return false;
    }
    return toggleProjectTabsAction_->isChecked();
}

void GlobalMenuController::setRecentProjects(const QStringList& workspacePaths,
                                             const QStringList& projectPaths) {
    if (!recentMenu_) {
        return;
    }

    recentMenu_->clear();

    auto addGroup = [this](const QString& title, const QStringList& paths) {
        if (paths.isEmpty()) {
            return;
        }

        QAction* header = recentMenu_->addAction(title);
        header->setEnabled(false);
        for (const QString& path : paths) {
            QAction* action = recentMenu_->addAction(path);
            connect(action, &QAction::triggered, this,
                    [this, path]() { emit recentPathRequested(path); });
        }
    };

    addGroup(QCoreApplication::translate("GlobalMenuController", "Workspace"), workspacePaths);
    if (!workspacePaths.isEmpty() && !projectPaths.isEmpty()) {
        recentMenu_->addSeparator();
    }
    addGroup(QCoreApplication::translate("GlobalMenuController", "Project"), projectPaths);

    recentMenu_->addSeparator();
    QAction* clearAction =
        recentMenu_->addAction(QCoreApplication::translate("GlobalMenuController", "Clear"));
    clearAction->setEnabled(!(workspacePaths.isEmpty() && projectPaths.isEmpty()));
    connect(clearAction, &QAction::triggered, this, &GlobalMenuController::clearRecentRequested);

    recentMenu_->setEnabled(true);
}

}  // namespace cc::neolux::fem
