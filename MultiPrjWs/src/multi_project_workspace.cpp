#include "cc/neolux/fem/mpw/multi_project_workspace.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr int kWorkspaceVersion = 1;
constexpr const char* kFormatKey = "format";
constexpr const char* kFormatValue = "femmpw";
constexpr const char* kVersionKey = "version";
constexpr const char* kNameKey = "name";
constexpr const char* kTagsKey = "tags";
constexpr const char* kProjectsKey = "projects";
constexpr const char* kPathKey = "path";
constexpr const char* kDisplayNameKey = "displayName";
constexpr const char* kNoteKey = "note";
constexpr const char* kEnabledKey = "enabled";
}  // namespace

namespace cc::neolux::fem::mpw {

bool MultiProjectWorkspace::ReadFile(const QString& filePath, MultiProjectWorkspaceData& data,
                                     QString* errorMessage) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QString("Cannot open workspace file: %1").arg(filePath);
        }
        return false;
    }

    return ParseContent(file.readAll(), data, errorMessage);
}

bool MultiProjectWorkspace::WriteFile(const QString& filePath,
                                      const MultiProjectWorkspaceData& data,
                                      QString* errorMessage) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists() && !dir.mkpath(".")) {
        if (errorMessage) {
            *errorMessage = QString("Cannot create workspace directory: %1").arg(dir.path());
        }
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Cannot write workspace file: %1").arg(filePath);
        }
        return false;
    }

    file.write(DumpContent(data));
    return true;
}

bool MultiProjectWorkspace::ParseContent(const QByteArray& content, MultiProjectWorkspaceData& data,
                                         QString* errorMessage) {
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(content, &parseError);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Invalid workspace JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    QJsonObject root = jsonDoc.object();
    if (root.value(kFormatKey).toString() != QString::fromUtf8(kFormatValue)) {
        if (errorMessage) {
            *errorMessage = QString("Invalid workspace format, expected '%1'.")
                                .arg(QString::fromUtf8(kFormatValue));
        }
        return false;
    }

    data.workspaceName = root.value(kNameKey).toString();

    data.tags.clear();
    for (const QJsonValue& tag : root.value(kTagsKey).toArray()) {
        data.tags.append(tag.toString());
    }

    data.projects.clear();
    for (const QJsonValue& itemValue : root.value(kProjectsKey).toArray()) {
        if (!itemValue.isObject()) {
            continue;
        }
        QJsonObject itemObject = itemValue.toObject();

        WorkspaceProjectItem item;
        item.projectFilePath = itemObject.value(kPathKey).toString();
        item.displayName = itemObject.value(kDisplayNameKey).toString();
        item.note = itemObject.value(kNoteKey).toString();
        item.enabled =
            itemObject.contains(kEnabledKey) ? itemObject.value(kEnabledKey).toBool() : true;

        if (item.projectFilePath.isEmpty()) {
            continue;
        }
        data.projects.append(item);
    }

    return true;
}

QByteArray MultiProjectWorkspace::DumpContent(const MultiProjectWorkspaceData& data) {
    QJsonObject root;
    root.insert(kFormatKey, QString::fromUtf8(kFormatValue));
    root.insert(kVersionKey, kWorkspaceVersion);
    root.insert(kNameKey, data.workspaceName);

    QJsonArray tags;
    for (const QString& tag : data.tags) {
        tags.append(tag);
    }
    root.insert(kTagsKey, tags);

    QJsonArray projects;
    for (const WorkspaceProjectItem& item : data.projects) {
        QJsonObject p;
        p.insert(kPathKey, item.projectFilePath);
        p.insert(kDisplayNameKey, item.displayName);
        p.insert(kNoteKey, item.note);
        p.insert(kEnabledKey, item.enabled);
        projects.append(p);
    }
    root.insert(kProjectsKey, projects);

    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool MultiProjectWorkspace::IsValidWorkspaceFile(const QString& filePath) {
    QFileInfo info(filePath);
    return info.suffix().compare("femmpw", Qt::CaseInsensitive) == 0;
}

QString MultiProjectWorkspace::DefaultExtension() {
    return QStringLiteral("femmpw");
}

}  // namespace cc::neolux::fem::mpw
