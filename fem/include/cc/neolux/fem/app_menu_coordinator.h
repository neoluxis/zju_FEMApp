#pragma once

#include <QObject>

#include "cc/neolux/fem/global_menu_controller.h"
#include "cc/neolux/fem/recent/recent_project_history.h"

class QMenuBar;
class QWidget;

namespace cc::neolux::fem {

/**
 * @brief 应用级菜单编排器。
 *
 * 负责协调全局菜单、最近项目历史以及 App 菜单行为（About / Clear Cache）。
 */
class AppMenuCoordinator : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造菜单编排器。
     * @param menuBar 主菜单栏。
     * @param dialogParent 弹窗父对象。
     * @param parent QObject 父对象。
     */
    explicit AppMenuCoordinator(QMenuBar* menuBar, QWidget* dialogParent,
                                QObject* parent = nullptr);

    /**
     * @brief 设置当前是否处于工作区模式。
     * @param enabled 工作区模式标记。
     */
    void setWorkspaceMode(bool enabled);

    /**
     * @brief 设置项目 Tab 勾选状态。
     * @param checked 勾选状态。
     * @param emitSignal 是否发送 toggled 信号。
     */
    void setProjectTabsChecked(bool checked, bool emitSignal);

    /**
     * @brief 获取项目 Tab 当前勾选状态。
     * @return 勾选状态。
     */
    bool isProjectTabsChecked() const;

    /**
     * @brief 新增最近路径并刷新 Recent 菜单。
     * @param path 路径。
     */
    void addRecentPath(const QString& path);

signals:
    void newProjectRequested();
    void newWorkspaceRequested();
    void openRequested();
    void workspaceConfigRequested();
    void saveRequested();
    void saveAsRequested();
    void exitRequested();
    void projectTabsToggled(bool checked);
    void recentPathRequested(const QString& path);

private:
    GlobalMenuController menuController_;
    recent::RecentProjectHistory recentHistory_;
    QWidget* dialogParent_ = nullptr;

    void refreshRecentMenu();
    void showAboutDialog();
    void clearAppCache();
};

}  // namespace cc::neolux::fem
