#pragma once

#include <QString>
#include <QStringList>

namespace cc::neolux::fem::recent {

class RecentProjectHistory {
public:
    explicit RecentProjectHistory(int maxItems = 8);

    void addProject(const QString& filePath);
    QStringList recentProjects() const;
    QString latestProject() const;

private:
    int maxItems_;
};

}  // namespace cc::neolux::fem::recent
