#pragma once

#include <QFileInfo>
#include <memory>

#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "cc/neolux/fem/xlsx_editor_module.h"
#include "ui_mainwindow.h"

class FemApp : public QWidget {
    Q_OBJECT

public:
    static void showWarning(QWidget* parent, const QString& text);
    static void showError(QWidget* parent, const QString& text);
    static void showInfo(QWidget* parent, const QString& text);

    explicit FemApp(QWidget* parent = nullptr);

    QFileInfo* femc_info;

    /**
     * Load a FEMConfig file and switch working dir to the file
     */
    void loadFEMConfig();

    void loadFEMConfig(const QString& filePath);

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

    // ----------- Slots can be defined here if needed -----------
private slots:
    void onLoadClicked();
    void onSaveClicked();
    void onSaveAsClicked();
    void onBrowseClicked();
    void onMatchClicked();

    void onFolderEdited(const QString& text);

    void onFileChanged(const QString& text);
    void onSheetChanged(const QString& text);

    void onDModeChanged(const QString& text);
    void onDUnitChanged(const QString& text);
    void onDCenterChanged(double value);
    void onDStepChanged(double value);
    void onDNoChanged(int value);
    void onDColsEdited();

    void onFModeChanged(const QString& text);
    void onFUnitChanged(const QString& text);
    void onFCenterChanged(double value);
    void onFStepChanged(double value);
    void onFNoChanged(int value);
    void onFRowsEdited();

    void onFEMModeChanged(const QString& text);
    void onFEMUnitChanged(const QString& text);
    void onFEMTargChanged(double value);
    void onFEMSpecChanged(double value);

    void onRawFileEdited();
    void onTxtResetClicked();
    void onTxtApplyClicked();
    void onRefreshEditorClicked();

private:
    Ui::FemApp ui;
    cc::neolux::femconfig::FEMData femdata;  // Instance of FEMConfig
    QString currentFilePath;                 // Track current opened file path
    bool isModified = false;                 // Track if file has been modified
    bool isLoading = false;                  // Flag to prevent marking as modified during file load
    std::unique_ptr<cc::neolux::fem::XlsxEditorModule> xlsxEditorModule;

    void updateFileLabel();    // Update labelFEMFile with current filename and modification status
    void markAsModified();     // Mark file as modified and update label
    void clearModifiedFlag();  // Clear modified flag and update label
    void refreshXlsxEditor();  // Refresh XLSX editor content
};
