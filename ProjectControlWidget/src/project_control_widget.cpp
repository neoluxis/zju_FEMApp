#include "cc/neolux/projectcontrol/project_control_widget.h"

#include <QComboBox>
#include <QSizePolicy>

#include "ui_projectcontrolwidget.h"

namespace cc::neolux::projectcontrol {

ProjectControlWidget::ProjectControlWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::ProjectControlWidget) {
    ui->setupUi(this);

    ui->cbFile->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    ui->cbFile->setMinimumContentsLength(12);
    ui->cbFile->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->cbSheet->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    ui->cbSheet->setMinimumContentsLength(12);
    ui->cbSheet->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(ui->btnLoad, &QPushButton::clicked, this, &ProjectControlWidget::loadClicked);
    connect(ui->btnSave, &QPushButton::clicked, this, &ProjectControlWidget::saveClicked);
    connect(ui->btnSaveas, &QPushButton::clicked, this, &ProjectControlWidget::saveAsClicked);
    connect(ui->btnFolderBrowse, &QPushButton::clicked, this,
            &ProjectControlWidget::folderBrowseClicked);
    connect(ui->btnFolderMatch, &QPushButton::clicked, this,
            &ProjectControlWidget::folderMatchClicked);
    connect(ui->lnFolder, &QLineEdit::textEdited, this, &ProjectControlWidget::folderEdited);
    connect(ui->cbFile, &QComboBox::currentTextChanged, this, &ProjectControlWidget::fileChanged);
    connect(ui->cbSheet, &QComboBox::currentTextChanged, this, &ProjectControlWidget::sheetChanged);
    connect(ui->btnRefreshEditor, &QPushButton::clicked, this,
            &ProjectControlWidget::refreshEditorClicked);
    connect(ui->chkDryRun, &QCheckBox::toggled, this, &ProjectControlWidget::dryRunToggled);

    connect(ui->cbDMode, &QComboBox::currentTextChanged, this,
            &ProjectControlWidget::doseModeChanged);
    connect(ui->cbDUnit, &QComboBox::currentTextChanged, this,
            &ProjectControlWidget::doseUnitChanged);
    connect(ui->dspnDCenter, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ProjectControlWidget::doseCenterChanged);
    connect(ui->dspnDStep, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ProjectControlWidget::doseStepChanged);
    connect(ui->spnDNo, qOverload<int>(&QSpinBox::valueChanged), this,
            &ProjectControlWidget::doseNoChanged);
    connect(ui->lnDCols, &QLineEdit::editingFinished, this, &ProjectControlWidget::doseColsEdited);

    connect(ui->cbFMode, &QComboBox::currentTextChanged, this,
            &ProjectControlWidget::focusModeChanged);
    connect(ui->cbFUnit, &QComboBox::currentTextChanged, this,
            &ProjectControlWidget::focusUnitChanged);
    connect(ui->dspnFCenter, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ProjectControlWidget::focusCenterChanged);
    connect(ui->dspnFStep, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ProjectControlWidget::focusStepChanged);
    connect(ui->spnFNo, qOverload<int>(&QSpinBox::valueChanged), this,
            &ProjectControlWidget::focusNoChanged);
    connect(ui->lnFRows, &QLineEdit::editingFinished, this, &ProjectControlWidget::focusRowsEdited);

    connect(ui->cbFEMMode, &QComboBox::currentTextChanged, this,
            &ProjectControlWidget::femModeChanged);
    connect(ui->cbFEMUnit, &QComboBox::currentTextChanged, this,
            &ProjectControlWidget::femUnitChanged);
    connect(ui->dspnFEMTarg, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ProjectControlWidget::femTargetChanged);
    connect(ui->dspnFEMSpec, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            &ProjectControlWidget::femSpecChanged);

    connect(ui->btnTxtReset, &QPushButton::clicked, this, &ProjectControlWidget::txtResetClicked);
    connect(ui->btnTxtApply, &QPushButton::clicked, this, &ProjectControlWidget::txtApplyClicked);
    connect(ui->txtConfigRaw, &QPlainTextEdit::textChanged, this,
            &ProjectControlWidget::rawTextEdited);
}

ProjectControlWidget::~ProjectControlWidget() {
    delete ui;
}

void ProjectControlWidget::setFileDisplayText(const QString& text) {
    ui->labelFEMFile->setText(text);
}

QString ProjectControlWidget::folderText() const {
    return ui->lnFolder->text();
}

void ProjectControlWidget::setFolderText(const QString& text) {
    ui->lnFolder->setText(text);
}

QString ProjectControlWidget::currentFileData() const {
    return ui->cbFile->currentData().toString();
}

QString ProjectControlWidget::currentSheetText() const {
    return ui->cbSheet->currentText();
}

void ProjectControlWidget::clearFiles() {
    ui->cbFile->clear();
}

void ProjectControlWidget::addFileItem(const QString& text, const QString& data) {
    ui->cbFile->addItem(text, data);
}

int ProjectControlWidget::findFileText(const QString& text) const {
    return ui->cbFile->findText(text);
}

void ProjectControlWidget::setFileIndex(int index) {
    ui->cbFile->setCurrentIndex(index);
}

void ProjectControlWidget::clearSheets() {
    ui->cbSheet->clear();
}

void ProjectControlWidget::addSheetItem(const QString& text) {
    ui->cbSheet->addItem(text);
}

int ProjectControlWidget::findSheetText(const QString& text) const {
    return ui->cbSheet->findText(text);
}

void ProjectControlWidget::setSheetIndex(int index) {
    ui->cbSheet->setCurrentIndex(index);
}

bool ProjectControlWidget::isDryRunChecked() const {
    return ui->chkDryRun->isChecked();
}

void ProjectControlWidget::setDryRunChecked(bool checked) {
    ui->chkDryRun->setChecked(checked);
}

void ProjectControlWidget::setDoseModeText(const QString& text) {
    ui->cbDMode->setCurrentText(text);
}

void ProjectControlWidget::setDoseUnitText(const QString& text) {
    ui->cbDUnit->setCurrentText(text);
}

void ProjectControlWidget::setDoseCenterValue(double value) {
    ui->dspnDCenter->setValue(value);
}

void ProjectControlWidget::setDoseStepValue(double value) {
    ui->dspnDStep->setValue(value);
}

void ProjectControlWidget::setDoseNoValue(int value) {
    ui->spnDNo->setValue(value);
}

void ProjectControlWidget::setDoseColsText(const QString& text) {
    ui->lnDCols->setText(text);
}

QString ProjectControlWidget::doseColsText() const {
    return ui->lnDCols->text();
}

void ProjectControlWidget::setFocusModeText(const QString& text) {
    ui->cbFMode->setCurrentText(text);
}

void ProjectControlWidget::setFocusUnitText(const QString& text) {
    ui->cbFUnit->setCurrentText(text);
}

void ProjectControlWidget::setFocusCenterValue(double value) {
    ui->dspnFCenter->setValue(value);
}

void ProjectControlWidget::setFocusStepValue(double value) {
    ui->dspnFStep->setValue(value);
}

void ProjectControlWidget::setFocusNoValue(int value) {
    ui->spnFNo->setValue(value);
}

void ProjectControlWidget::setFocusRowsText(const QString& text) {
    ui->lnFRows->setText(text);
}

QString ProjectControlWidget::focusRowsText() const {
    return ui->lnFRows->text();
}

void ProjectControlWidget::setFemModeText(const QString& text) {
    ui->cbFEMMode->setCurrentText(text);
}

void ProjectControlWidget::setFemUnitText(const QString& text) {
    ui->cbFEMUnit->setCurrentText(text);
}

void ProjectControlWidget::setFemTargetValue(double value) {
    ui->dspnFEMTarg->setValue(value);
}

void ProjectControlWidget::setFemSpecValue(double value) {
    ui->dspnFEMSpec->setValue(value);
}

void ProjectControlWidget::setRawConfigText(const QString& text) {
    ui->txtConfigRaw->setPlainText(text);
}

QString ProjectControlWidget::rawConfigText() const {
    return ui->txtConfigRaw->toPlainText();
}

}  // namespace cc::neolux::projectcontrol
