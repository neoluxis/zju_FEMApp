#pragma once

#include <QString>
#include <QVBoxLayout>

#include "cc/neolux/fem/xlsxeditor/XLSXEditor.hpp"

namespace cc::neolux::fem {

/**
 * @brief XLSX 编辑器模块封装
 */
class XlsxEditorModule {
public:
  /**
   * @brief 绑定到指定布局
   */
  explicit XlsxEditorModule(QVBoxLayout *container);

  /**
   * @brief 释放资源
   */
  ~XlsxEditorModule();

  /**
   * @brief 加载 XLSX 文件与工作表范围
   */
  void load(const QString &filePath, const QString &sheetName, const QString &range);

  /**
   * @brief 清空当前显示
   */
  void clear();

private:
  QVBoxLayout *container_;
  cc::neolux::fem::xlsxeditor::XLSXEditor *editor_;
};

} // namespace cc::neolux::fem
