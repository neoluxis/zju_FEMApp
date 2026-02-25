#include "cc/neolux/fem/recent/recent_project_history.h"

#include <QDateTime>
#include <QFileInfo>
#include <QSettings>
#include <QVariantMap>
#include <algorithm>

namespace {
constexpr const char* kOrg = "neolux";
constexpr const char* kApp = "FemApp";
constexpr const char* kRecentKey = "recent/recentProjects";
constexpr const char* kRecentTimesKey = "recent/recentProjectAccessTimes";
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

    QSettings settings(kOrg, kApp);
    QVariantMap accessTimes = settings.value(kRecentTimesKey).toMap();
    accessTimes.insert(normalized, static_cast<qlonglong>(QDateTime::currentMSecsSinceEpoch()));

    while (history.size() > maxItems_) {
        const QString removed = history.takeLast();
        accessTimes.remove(removed);
    }

    settings.setValue(kRecentKey, history);
    settings.setValue(kRecentTimesKey, accessTimes);
}

QStringList RecentProjectHistory::recentProjects() const {
    QSettings settings(kOrg, kApp);
    QStringList history = settings.value(kRecentKey).toStringList();
    const QVariantMap accessTimes = settings.value(kRecentTimesKey).toMap();

    struct RecentEntry {
        QString path;
        qlonglong accessTime = 0;
        int originalOrder = 0;
    };

    std::vector<RecentEntry> entries;
    entries.reserve(history.size());

    int order = 0;
    for (const QString& item : history) {
        QFileInfo fileInfo(item);
        if (fileInfo.exists() && fileInfo.isFile()) {
            RecentEntry entry;
            entry.path = fileInfo.absoluteFilePath();
            entry.accessTime = accessTimes.value(entry.path).toLongLong();
            entry.originalOrder = order;
            entries.push_back(entry);
        }
        ++order;
    }

    std::sort(entries.begin(), entries.end(),
              [](const RecentEntry& left, const RecentEntry& right) {
                  if (left.accessTime != right.accessTime) {
                      return left.accessTime > right.accessTime;
                  }
                  return left.originalOrder < right.originalOrder;
              });

    QStringList existingOnly;
    existingOnly.reserve(static_cast<qsizetype>(entries.size()));
    for (const RecentEntry& entry : entries) {
        existingOnly.append(entry.path);
    }

    while (existingOnly.size() > maxItems_) {
        existingOnly.removeLast();
    }

    if (existingOnly != history) {
        settings.setValue(kRecentKey, existingOnly);
    }

    QVariantMap cleanedTimes;
    for (const QString& path : existingOnly) {
        cleanedTimes.insert(path, accessTimes.value(path).toLongLong());
    }
    settings.setValue(kRecentTimesKey, cleanedTimes);

    return existingOnly;
}

QString RecentProjectHistory::latestProject() const {
    const QStringList history = recentProjects();
    return history.isEmpty() ? QString() : history.first();
}

void RecentProjectHistory::clear() {
    QSettings settings(kOrg, kApp);
    settings.remove(kRecentKey);
    settings.remove(kRecentTimesKey);
}

}  // namespace cc::neolux::fem::recent
