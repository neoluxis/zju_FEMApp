//
// Created by neolux on 2026/1/30.
//

#include <string>
#include <iostream>
#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "filesystem"

#include "OpenXLSX.hpp"


using std::string;
using std::cout;
using std::endl;
using namespace cc::neolux::femconfig;
namespace fs = std::filesystem;
namespace oxl = OpenXLSX;

void test_print_fem_data(const FEMData &data);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <path_to_fem_file>" << std::endl;
    return 1;
  }

  string filePath = argv[1];

  FEMData data;
  if (!FEMConfig::ReadFile(filePath, data)) {
    std::cerr << "Failed to read FEM file: " << filePath << std::endl;
    return 1;
  }
  test_print_fem_data(data);

  //  切换工作目录到 FEM 文件所在目录
  fs::path path = filePath;
  fs::path abspath = fs::absolute(path).parent_path();  // file -> folder
  cout << "Current FEM file absolute folder: " << abspath.string() << std::endl;
  fs::current_path(abspath);
  cout << "Changed working directory to: " << fs::current_path().string() << std::endl;

  // 展开文件夹通配符
  auto folders = FEMConfig::ExpandFolderPattern(data);
  cout << "Expanded Folders:" << folders.size() << std::endl;
  for (const auto &folder: folders) {
    cout << "  " << folder << std::endl;
  }

  // 展开文件名通配符
  auto filenames = FEMConfig::ExpandFilenamePattern(data);
  cout << "Expanded Filenames:" << filenames.size() << std::endl;
  for (const auto &filename: filenames) {
    cout << "  " << filename << std::endl;
  }

  auto filename = filenames.empty() ? "" : filenames[0];
  cout << "Using file " << filename << std::endl;

  auto sheets = FEMConfig::ExpandSheetPattern(filename, data);
  cout << "Expanded Sheets:" << sheets.size() << std::endl;
  for (const auto &sheet: sheets) {
    cout << "  " << sheet << std::endl;
  }

  FEMConfig::dumpFEMData(data, "./output.fem");

  FEMData data1;
  FEMConfig::ReadFile("./output.fem", data1);
  test_print_fem_data(data1);


  return 0;
}

void test_print_fem_data(const FEMData &data) {
  cout << "===== FEM Data =====" << std::endl;
  cout << "Folder Pattern: " << data.folderPattern << std::endl;
  cout << "Filename Pattern: " << data.filenamePattern << std::endl;
  cout << "Sheet Pattern: " << data.sheetPattern << std::endl;
  cout << "Dose" << std::endl;
  cout << "  Mode: " << data.dose.mode << std::endl;
  cout << "  Unit: " << data.dose.unit << std::endl;
  cout << "  Center: " << data.dose.center << std::endl;
  cout << "  Step: " << data.dose.step << std::endl;
  cout << "  No: " << data.dose.no << std::endl;
  cout << "  Cols: " << data.dose.cols << std::endl;
  cout << "Focus" << std::endl;
  cout << "  Mode: " << data.focus.mode << std::endl;
  cout << "  Unit: " << data.focus.unit << std::endl;
  cout << "  Center: " << data.focus.center << std::endl;
  cout << "  Step: " << data.focus.step << std::endl;
  cout << "  No: " << data.focus.no << std::endl;
  cout << "  Rows: " << data.focus.rows << std::endl;
  cout << "FEM" << std::endl;
  cout << "  Mode: " << data.fem.mode << std::endl;
  cout << "  Unit: " << data.fem.unit << std::endl;
  cout << "  Target: " << data.fem.target << std::endl;
  cout << "  Spec: " << data.fem.spec << std::endl;
  cout << "===========================" << std::endl;
}
