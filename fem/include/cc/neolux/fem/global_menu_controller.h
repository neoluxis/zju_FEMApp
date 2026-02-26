#pragma once

#include <QObject>
#include <QStringList>

class QAction;
class QMenu;
class QMenuBar;

namespace cc::neolux::fem {

/**
 * @brief 全局菜单控制器，负责创建菜单并转发菜单动作信号。
 */
class GlobalMenuController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造并绑定到目标菜单栏。
     * @param menuBar 目标菜单栏。
     * @param parent QObject 父对象。
     */
    explicit GlobalMenuController(QMenuBar* menuBar, QObject* parent = nullptr);

    /**
     * @brief 设置工作区配置菜单使能状态。
     * @param enabled 是否使能。
     */
    void setWorkspaceConfigEnabled(bool enabled);

    /**
     * @brief 设置“显示项目标签”勾选状态。
     * @param checked 勾选状态。
     * @param emitSignal 是否发送切换信号。
     */
    void setProjectTabsChecked(bool checked, bool emitSignal);

    /**
     * @brief 获取“显示项目标签”当前勾选状态。
     * @return 当前勾选状态。
     */
    bool isProjectTabsChecked() const;

    /**
     * @brief 通过路径列表刷新 Recent 子菜单。
     * @param workspacePaths 工作空间路径列表。
     * @param projectPaths 单项目路径列表。
     */
    void setRecentProjects(const QStringList& workspacePaths, const QStringList& projectPaths);

signals:
    /** @brief 请求创建新单项目。 */
    void newProjectRequested();
    /** @brief 请求创建新工作空间。 */
    void newWorkspaceRequested();
    /** @brief 请求打开项目或工作空间文件。 */
    void openRequested();
    /** @brief 请求打开工作空间配置。 */
    void workspaceConfigRequested();
    /** @brief 请求保存当前项目。 */
    void saveRequested();
    /** @brief 请求另存为。 */
    void saveAsRequested();
    /** @brief 请求退出应用。 */
    void exitRequested();
    /** @brief 请求显示关于对话框。 */
    void aboutRequested();
    /** @brief 请求清理缓存。 */
    void clearCacheRequested();
    /** @brief 请求切换项目标签显示状态。 */
    void projectTabsToggled(bool checked);
    /** @brief 请求打开历史路径。 */
    void recentPathRequested(const QString& path);
    /** @brief 请求清空最近历史。 */
    void clearRecentRequested();

private:
    QMenu* recentMenu_ = nullptr;
    QAction* workspaceConfigAction_ = nullptr;
    QAction* toggleProjectTabsAction_ = nullptr;
};

}  // namespace cc::neolux::fem
