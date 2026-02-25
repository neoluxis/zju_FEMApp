#include "cc/neolux/fem/recent/recent_project_history.h"

#include <QFileInfo>
#include <QSettings>

namespace {
constexpr const char* kOrg = "neolux";
constexpr const char* kApp = "FemApp";
constexpr const char* kRecentKey = "recent/recentProjects";
}  // namespace

namespace cc::neolux::fem::recent {

RecentProjectHistory::RecentProjectHistory(int maxItems) : maxItems_(maxItems > 0 ? maxItems : 8) {}

void RecentProjectHistory::addProject(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    const QString normalized = fileInfo.absoluteFilePath();
    if (normalized.isEmpty()) {
        return;
    }

    QStringList history = recentProjects();
    history.removeAll(normalized);
    history.prepend(normalized);

    while (history.size() > maxItems_) {
        history.removeLast();
    }

    QSettings settings(kOrg, kApp);
    settings.setValue(kRecentKey, history);
}

QStringList RecentProjectHistory::recentProjects() const {
    QSettings settings(kOrg, kApp);
    QStringList history = settings.value(kRecentKey).toStringList();

    QStringList existingOnly;
    existingOnly.reserve(history.size());
    for (const QString& item : history) {
        QFileInfo fileInfo(item);
        if (fileInfo.exists() && fileInfo.isFile()) {
            existingOnly.append(fileInfo.absoluteFilePath());
        }
    }

    if (existingOnly != history) {
        settings.setValue(kRecentKey, existingOnly);
    }

    return existingOnly;
}

QString RecentProjectHistory::latestProject() const {
    const QStringList history = recentProjects();
    return history.isEmpty() ? QString() : history.first();
}

}  // namespace cc::neolux::fem::recent
