#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef MULTIPRJWS_EXPORTS
#define MULTIPRJWS_API __declspec(dllexport)
#else
#define MULTIPRJWS_API __declspec(dllimport)
#endif
#else
#define MULTIPRJWS_API
#endif

namespace cc::neolux::fem::mpw {

struct WorkspaceProjectItem {
    QString projectFilePath;
    QString displayName;
    QString note;
    bool enabled = true;
};

struct MultiProjectWorkspaceData {
    QString workspaceName;
    QStringList tags;
    QVector<WorkspaceProjectItem> projects;
};

class MULTIPRJWS_API MultiProjectWorkspace {
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
