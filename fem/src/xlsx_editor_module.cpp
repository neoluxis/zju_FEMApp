#include "cc/neolux/fem/xlsx_editor_module.h"

#include <QWidget>
#include <QSizePolicy>

namespace cc::neolux::fem {

XlsxEditorModule::XlsxEditorModule(QVBoxLayout *container)
  : container_(container), editor_(nullptr) {
  if (!container_) {
    return;
  }

  QWidget *parent = container_->parentWidget();
  editor_ = new cc::neolux::fem::xlsxeditor::XLSXEditor(parent);
  editor_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  container_->addWidget(editor_);
}

XlsxEditorModule::~XlsxEditorModule() {
  if (editor_) {
    editor_->deleteLater();
    editor_ = nullptr;
  }
}

void XlsxEditorModule::load(const QString &filePath, const QString &sheetName, const QString &range) {
  if (!editor_) {
    return;
  }
  if (filePath.isEmpty() || sheetName.isEmpty() || range.isEmpty()) {
    clear();
    return;
  }
  editor_->setVisible(true);
  editor_->loadXLSX(filePath, sheetName, range);
}

void XlsxEditorModule::clear() {
  if (!editor_) {
    return;
  }
  editor_->setVisible(false);
}

} // namespace cc::neolux::fem
