#pragma once

#include <QMenu>
#include <memory>

#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "cc/neolux/fem/mpw/multi_prj_ws_config_dialog.h"
#include "cc/neolux/fem/mpw/multi_prj_ws_widget.h"
#include "cc/neolux/fem/recent/recent_project_history.h"
#include "cc/neolux/fem/xlsx_editor_module.h"
#include "cc/neolux/projectcontrol/project_control_widget.h"
#include "ui_mainwindow.h"

class FemApp : public QWidget {
    Q_OBJECT

public:
    static void showWarning(QWidget* parent, const QString& text);
    static void showError(QWidget* parent, const QString& text);
    static void showInfo(QWidget* parent, const QString& text);

    explicit FemApp(QWidget* parent = nullptr);

    /**
     * Load a FEMConfig file and switch working dir to the file
     */
    bool loadFEMConfig();

    bool loadFEMConfig(const QString& filePath);
    bool openSingleProject(const QString& filePath);
    bool loadMultiProjectWorkspace(const QString& filePath);
    void openMultiProjectWorkspaceConfig();

    std::string getFolderMatched();

    std::string getFileMatched(std::string folder);

    QString getCurrentSelectedFile();

    void updateFileList(const std::string& folder);

    std::string getSheetMatched(std::string filename);

    void updateSheetList(const std::string& filename);

    void onLoadFile();

    ~FemApp() override;

protected:
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::FemApp ui;
    cc::neolux::femconfig::FEMData femdata;  // Instance of FEMConfig
    QString currentFilePath;                 // Track current opened file path
    bool isModified = false;                 // Track if file has been modified
    bool isLoading = false;                  // Flag to prevent marking as modified during file load
    std::unique_ptr<cc::neolux::fem::XlsxEditorModule> xlsxEditorModule;
    cc::neolux::projectcontrol::ProjectControlWidget* projectControlWidget = nullptr;
    cc::neolux::fem::mpw::MultiPrjWsWidget* multiPrjWsWidget = nullptr;
    cc::neolux::fem::recent::RecentProjectHistory recentProjectHistory;
    QMenu* recentMenu = nullptr;
    QAction* workspaceConfigAction = nullptr;
    QAction* toggleProjectTabsAction = nullptr;
    bool workspaceMode = false;
    QString currentWorkspaceFilePath;
    bool relaxPatternMatchValidation = false;
    bool skipAutoRefreshEditorOnce = false;

    void updateFileLabel();  // Update labelFEMFile with current filename and modification status
    void setWorkspaceMode(bool enabled);
    void setProjectTabsVisible(bool visible);
    bool isProjectTabsVisible() const;
    void restoreWindowGeometryState();
    void saveWindowGeometryState();
    void refreshRecentMenu();
    void markAsModified();     // Mark file as modified and update label
    void clearModifiedFlag();  // Clear modified flag and update label
    void refreshXlsxEditor();  // Refresh XLSX editor content
    void applyRawConfigText();
    void createNewProject();
    void createNewWorkspace();
    void loadConfigFromDialog();
    void saveCurrentConfig();
    void saveCurrentConfigAs();
    void browseFolder();
    void matchFolderPattern();
    void setFolderPattern(const QString& text);
    void setFilenamePattern(const QString& text);
    void setSheetPattern(const QString& text);
};
