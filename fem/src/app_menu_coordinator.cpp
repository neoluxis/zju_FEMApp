#include "cc/neolux/fem/app_menu_coordinator.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>

#include "cc/neolux/fem/mpw/multi_project_workspace.h"
#include "cc/neolux/fem/version/app_info.h"

namespace cc::neolux::fem {

namespace {
QString T(const char* text) {
    return QCoreApplication::translate("AppMenuCoordinator", text);
}
}  // namespace

AppMenuCoordinator::AppMenuCoordinator(QMenuBar* menuBar, QWidget* dialogParent, QObject* parent)
    : QObject(parent), menuController_(menuBar, this), dialogParent_(dialogParent) {
    connect(&menuController_, &GlobalMenuController::newProjectRequested, this,
            &AppMenuCoordinator::newProjectRequested);
    connect(&menuController_, &GlobalMenuController::newWorkspaceRequested, this,
            &AppMenuCoordinator::newWorkspaceRequested);
    connect(&menuController_, &GlobalMenuController::openRequested, this,
            &AppMenuCoordinator::openRequested);
    connect(&menuController_, &GlobalMenuController::workspaceConfigRequested, this,
            &AppMenuCoordinator::workspaceConfigRequested);
    connect(&menuController_, &GlobalMenuController::saveRequested, this,
            &AppMenuCoordinator::saveRequested);
    connect(&menuController_, &GlobalMenuController::saveAsRequested, this,
            &AppMenuCoordinator::saveAsRequested);
    connect(&menuController_, &GlobalMenuController::exitRequested, this,
            &AppMenuCoordinator::exitRequested);
    connect(&menuController_, &GlobalMenuController::projectTabsToggled, this,
            &AppMenuCoordinator::projectTabsToggled);
    connect(&menuController_, &GlobalMenuController::recentPathRequested, this,
            &AppMenuCoordinator::recentPathRequested);

    connect(&menuController_, &GlobalMenuController::aboutRequested, this,
            [this]() { showAboutDialog(); });
    connect(&menuController_, &GlobalMenuController::clearCacheRequested, this,
            [this]() { clearAppCache(); });
    connect(&menuController_, &GlobalMenuController::clearRecentRequested, this, [this]() {
        recentHistory_.clear();
        refreshRecentMenu();
    });

    refreshRecentMenu();
}

void AppMenuCoordinator::setWorkspaceMode(bool enabled) {
    menuController_.setWorkspaceConfigEnabled(enabled);
}

void AppMenuCoordinator::setProjectTabsChecked(bool checked, bool emitSignal) {
    menuController_.setProjectTabsChecked(checked, emitSignal);
}

bool AppMenuCoordinator::isProjectTabsChecked() const {
    return menuController_.isProjectTabsChecked();
}

void AppMenuCoordinator::addRecentPath(const QString& path) {
    if (path.isEmpty()) {
        return;
    }

    recentHistory_.addProject(path);
    refreshRecentMenu();
}

void AppMenuCoordinator::refreshRecentMenu() {
    const QStringList projects = recentHistory_.recentProjects();

    QStringList workspacePaths;
    QStringList projectPaths;
    for (const QString& path : projects) {
        if (mpw::MultiProjectWorkspace::IsValidWorkspaceFile(path)) {
            workspacePaths.append(path);
        } else {
            projectPaths.append(path);
        }
    }

    menuController_.setRecentProjects(workspacePaths, projectPaths);
}

void AppMenuCoordinator::showAboutDialog() {
    const QString appName = QString::fromUtf8(version::kAppName);
    const QString version = QString::fromUtf8(version::kVersion);
    const QString company = QString::fromUtf8(version::kCompany);
    const QString description = QString::fromUtf8(version::kDescription);
    const QString copyright = QString::fromUtf8(version::kCopyright);

    const QString aboutText = T("%1\nVersion: %2\nCompany: %3\n\n%4\n\n%5")
                                  .arg(appName, version, company, description, copyright);
    QMessageBox::about(dialogParent_, T("About %1").arg(appName), aboutText);
}

void AppMenuCoordinator::clearAppCache() {
    const int button =
        QMessageBox::question(dialogParent_, T("Clear Cache"),
                              T("Clear application cache (recent list and UI settings)?"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (button != QMessageBox::Yes) {
        return;
    }

    recentHistory_.clear();
    refreshRecentMenu();

    QSettings settings("neolux", "FemApp");
    settings.clear();

    QMessageBox::information(dialogParent_, T("Info"), T("Application cache has been cleared."));
}

}  // namespace cc::neolux::fem
