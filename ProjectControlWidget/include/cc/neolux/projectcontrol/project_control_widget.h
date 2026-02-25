#pragma once

#include <QWidget>

namespace Ui {
class ProjectControlWidget;
}

namespace cc::neolux::projectcontrol {

class ProjectControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProjectControlWidget(QWidget* parent = nullptr);
    ~ProjectControlWidget() override;

    void setFileDisplayText(const QString& text);

    QString folderText() const;
    void setFolderText(const QString& text);

    QString currentFileData() const;
    QString currentSheetText() const;

    void clearFiles();
    void addFileItem(const QString& text, const QString& data);
    int findFileText(const QString& text) const;
    void setFileIndex(int index);

    void clearSheets();
    void addSheetItem(const QString& text);
    int findSheetText(const QString& text) const;
    void setSheetIndex(int index);

    bool isDryRunChecked() const;
    void setDryRunChecked(bool checked);

    void setDoseModeText(const QString& text);
    void setDoseUnitText(const QString& text);
    void setDoseCenterValue(double value);
    void setDoseStepValue(double value);
    void setDoseNoValue(int value);
    void setDoseColsText(const QString& text);
    QString doseColsText() const;

    void setFocusModeText(const QString& text);
    void setFocusUnitText(const QString& text);
    void setFocusCenterValue(double value);
    void setFocusStepValue(double value);
    void setFocusNoValue(int value);
    void setFocusRowsText(const QString& text);
    QString focusRowsText() const;

    void setFemModeText(const QString& text);
    void setFemUnitText(const QString& text);
    void setFemTargetValue(double value);
    void setFemSpecValue(double value);

    void setRawConfigText(const QString& text);
    QString rawConfigText() const;

signals:
    void loadClicked();
    void saveClicked();
    void saveAsClicked();
    void folderBrowseClicked();
    void folderMatchClicked();
    void folderEdited(const QString& text);
    void fileChanged(const QString& text);
    void sheetChanged(const QString& text);
    void refreshEditorClicked();
    void dryRunToggled(bool checked);

    void doseModeChanged(const QString& text);
    void doseUnitChanged(const QString& text);
    void doseCenterChanged(double value);
    void doseStepChanged(double value);
    void doseNoChanged(int value);
    void doseColsEdited();

    void focusModeChanged(const QString& text);
    void focusUnitChanged(const QString& text);
    void focusCenterChanged(double value);
    void focusStepChanged(double value);
    void focusNoChanged(int value);
    void focusRowsEdited();

    void femModeChanged(const QString& text);
    void femUnitChanged(const QString& text);
    void femTargetChanged(double value);
    void femSpecChanged(double value);

    void txtResetClicked();
    void txtApplyClicked();
    void rawTextEdited();

private:
    Ui::ProjectControlWidget* ui;
};

}  // namespace cc::neolux::projectcontrol
