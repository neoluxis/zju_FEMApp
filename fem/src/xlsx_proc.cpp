//
// Created by neolux on 2026/1/31.
//

#include "cc/neolux/fem/xlsx_proc.h"

#include <filesystem>

#include "iostream"

namespace cc::neolux::fem {
std::vector<std::string> XlsxProc::GetSheetNames(const std::string& filePath) {
    cc::neolux::utils::MiniXLSX::OpenXLSXWrapper wrapper;
    if (!wrapper.open(filePath)) {
        std::cerr << "Failed to open Excel file: " << filePath << std::endl;
        return {};
    }
    std::cout << "Opened Excel file successfully." << std::endl;

    std::vector<std::string> sheetNames;
    unsigned int count = wrapper.sheetCount();
    for (unsigned int i = 0; i < count; ++i) {
        sheetNames.push_back(wrapper.sheetName(i));
    }
    wrapper.close();
    return sheetNames;
}
}  // namespace cc::neolux::fem
