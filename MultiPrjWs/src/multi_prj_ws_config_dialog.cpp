#include "cc/neolux/fem/mpw/multi_prj_ws_config_dialog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSignalBlocker>
#include <algorithm>

#include "ui_multiprjws_config.h"

namespace cc::neolux::fem::mpw {

namespace {
QString trConfig(const char* text) {
    return QCoreApplication::translate("MultiPrjWsConfigDialog", text);
}
}  // namespace

MultiPrjWsConfigDialog::MultiPrjWsConfigDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::MultiPrjWsConfigDialog) {
    ui->setupUi(this);

    connect(ui->lstProjects, &QListWidget::currentRowChanged, this,
            [this](int) { refreshEditors(); });
    connect(ui->lnDisplayName, &QLineEdit::textChanged, this, [this](const QString& text) {
        const int index = currentIndex();
        if (index < 0 || index >= data_.projects.size()) {
            return;
        }
        data_.projects[index].displayName = text;
        if (ui->lstProjects->currentItem()) {
            ui->lstProjects->currentItem()->setText(text);
        }
        saveWorkspace();
    });
    connect(ui->txtNote, &QPlainTextEdit::textChanged, this, [this]() {
        const int index = currentIndex();
        if (index < 0 || index >= data_.projects.size()) {
            return;
        }
        data_.projects[index].note = ui->txtNote->toPlainText();
        saveWorkspace();
    });

    connect(ui->btnAddProject, &QPushButton::clicked, this, [this]() { addProject(); });
    connect(ui->btnRemoveProject, &QPushButton::clicked, this,
            [this]() { removeSelectedProject(); });
    connect(ui->btnClose, &QPushButton::clicked, this, [this]() { close(); });
}

MultiPrjWsConfigDialog::~MultiPrjWsConfigDialog() {
    delete ui;
}

bool MultiPrjWsConfigDialog::loadWorkspace(const QString& workspaceFilePath,
                                           QString* errorMessage) {
    MultiProjectWorkspaceData data;
    if (!MultiProjectWorkspace::ReadFile(workspaceFilePath, data, errorMessage)) {
        return false;
    }

    data_ = data;
    workspaceFilePath_ = QFileInfo(workspaceFilePath).absoluteFilePath();
    refreshProjectList();
    return true;
}

void MultiPrjWsConfigDialog::refreshProjectList() {
    ui->lstProjects->clear();
    for (const WorkspaceProjectItem& item : data_.projects) {
        const QString name = item.displayName.isEmpty() ? QFileInfo(item.projectFilePath).fileName()
                                                        : item.displayName;
        ui->lstProjects->addItem(name);
    }

    if (!data_.projects.isEmpty()) {
        ui->lstProjects->setCurrentRow(0);
    } else {
        refreshEditors();
    }
}

void MultiPrjWsConfigDialog::refreshEditors() {
    const int index = currentIndex();
    if (index < 0 || index >= data_.projects.size()) {
        ui->lnProjectPath->clear();
        ui->lnDisplayName->clear();
        ui->lnDisplayName->setEnabled(false);
        ui->txtNote->clear();
        ui->txtNote->setEnabled(false);
        ui->btnRemoveProject->setEnabled(false);
        return;
    }

    const QFileInfo workspaceInfo(workspaceFilePath_);
    const QDir workspaceDir = workspaceInfo.absoluteDir();
    const QString absolutePath =
        QFileInfo(workspaceDir.filePath(data_.projects[index].projectFilePath)).absoluteFilePath();

    ui->lnProjectPath->setText(absolutePath);
    ui->lnDisplayName->setEnabled(true);
    ui->txtNote->setEnabled(true);
    ui->btnRemoveProject->setEnabled(true);

    const QSignalBlocker blockDisplayName(ui->lnDisplayName);
    ui->lnDisplayName->setText(data_.projects[index].displayName);

    const QSignalBlocker blocker(ui->txtNote);
    ui->txtNote->setPlainText(data_.projects[index].note);
}

void MultiPrjWsConfigDialog::addProject() {
    const QFileInfo workspaceInfo(workspaceFilePath_);
    const QDir workspaceDir = workspaceInfo.absoluteDir();

    const QString selectedPath = QFileDialog::getOpenFileName(
        this, trConfig("Select FEM Project"), workspaceDir.absolutePath(),
        trConfig("FEM Config Files (*.fem);;All Files (*)"));
    if (selectedPath.isEmpty()) {
        return;
    }

    const QString absolutePath = QFileInfo(selectedPath).absoluteFilePath();
    for (const WorkspaceProjectItem& item : data_.projects) {
        const QString existingPath =
            QFileInfo(workspaceDir.filePath(item.projectFilePath)).absoluteFilePath();
        if (existingPath == absolutePath) {
            return;
        }
    }

    WorkspaceProjectItem item;
    item.projectFilePath = workspaceDir.relativeFilePath(absolutePath);
    item.displayName = QFileInfo(absolutePath).fileName();
    item.note.clear();
    item.enabled = data_.projects.isEmpty();
    data_.projects.append(item);

    saveWorkspace();
    refreshProjectList();
    ui->lstProjects->setCurrentRow(data_.projects.size() - 1);
}

void MultiPrjWsConfigDialog::removeSelectedProject() {
    const int index = currentIndex();
    if (index < 0 || index >= data_.projects.size()) {
        return;
    }

    const bool removedWasEnabled = data_.projects[index].enabled;
    data_.projects.removeAt(index);

    if (removedWasEnabled && !data_.projects.isEmpty()) {
        data_.projects[0].enabled = true;
    }

    saveWorkspace();
    refreshProjectList();
    if (!data_.projects.isEmpty()) {
        const int lastIndex = static_cast<int>(data_.projects.size()) - 1;
        ui->lstProjects->setCurrentRow(std::min(index, lastIndex));
    }
}

bool MultiPrjWsConfigDialog::saveWorkspace() {
    QString errorMessage;
    if (!MultiProjectWorkspace::WriteFile(workspaceFilePath_, data_, &errorMessage)) {
        QMessageBox::critical(this, trConfig("Error"), errorMessage);
        return false;
    }

    emit workspaceSaved(workspaceFilePath_);
    return true;
}

int MultiPrjWsConfigDialog::currentIndex() const {
    return ui->lstProjects->currentRow();
}

}  // namespace cc::neolux::fem::mpw
