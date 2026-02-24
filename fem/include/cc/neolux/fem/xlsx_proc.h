//
// Created by neolux on 2026/1/31.
//

#ifndef FEMAPP_XLSX_PROC_H
#define FEMAPP_XLSX_PROC_H

#include <string>
#include <vector>

#include "cc/neolux/utils/MiniXLSX/OpenXLSXWrapper.hpp"

namespace cc::neolux::fem {
class XlsxProc {
public:
    static std::vector<std::string> GetSheetNames(const std::string& filePath);

private:
};

}  // namespace cc::neolux::fem

#endif  // FEMAPP_XLSX_PROC_H
