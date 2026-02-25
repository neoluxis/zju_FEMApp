#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace cc::neolux::fem::mpw {

struct WorkspaceProjectItem {
    QString projectFilePath;
    QString displayName;
    bool enabled = true;
};

struct MultiProjectWorkspaceData {
    QString workspaceName;
    QString baseDir;
    QStringList tags;
    QVector<WorkspaceProjectItem> projects;
};

class MultiProjectWorkspace {
public:
    static bool ReadFile(const QString& filePath, MultiProjectWorkspaceData& data,
                         QString* errorMessage = nullptr);

    static bool WriteFile(const QString& filePath, const MultiProjectWorkspaceData& data,
                          QString* errorMessage = nullptr);

    static bool ParseContent(const QByteArray& content, MultiProjectWorkspaceData& data,
                             QString* errorMessage = nullptr);

    static QByteArray DumpContent(const MultiProjectWorkspaceData& data);

    static bool IsValidWorkspaceFile(const QString& filePath);

    static QString DefaultExtension();
};

}  // namespace cc::neolux::fem::mpw
