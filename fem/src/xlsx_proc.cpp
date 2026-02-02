//
// Created by neolux on 2026/1/31.
//

#include "cc/neolux/fem/xlsx_proc.h"
#include "iostream"
#include <filesystem>

namespace cc::neolux::fem {
  std::vector<std::string> XlsxProc::GetSheetNames(const std::string &filePath) {
    oxl::XLDocument doc;
    const auto normalizedPath = std::filesystem::u8path(filePath).u8string();
    doc.open(normalizedPath);
    std::cout << "Opened Excel file successfully." << std::endl;

    auto wbs = doc.workbook();
    std::vector<std::string> sheetNames = wbs.sheetNames();
    doc.close();
    return sheetNames;
  }
} // namespace cc::neolux::fem
