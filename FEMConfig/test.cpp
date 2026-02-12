//
// Created by neolux on 2026/1/30.
//

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cc/neolux/FEMConfig/FEMConfig.h"
#include "filesystem"

#include "cc/neolux/utils/MiniXLSX/OpenXLSXWrapper.hpp"


using std::string;
using std::cout;
using std::endl;
using namespace cc::neolux::femconfig;
namespace fs = std::filesystem;
namespace minixlsx = cc::neolux::utils::MiniXLSX;

void test_print_fem_data(const FEMData &data);

// Test 1: Parse config with double quotes
void test_quoted_values() {
  cout << "\n========== Test 1: Quoted Values ==========" << endl;
  string config_quoted = R"(
folder = "*data*"
filename = "AT00229.01-s25-AA (IMAGE) E STEP 0.05.xlsx"
sheet = "TSK"
dose={"mode":"LowHigh", "unit":"mJ/cm2",
"center":4, "step":0.07, "no": 26,
"cols":"B:K"
}
focus={"mode":"NegPos2", "unit":"μm",
"center":0, "step":0.03, "no": 29,
"rows":"3:60"
}
fem={"mode":"Focus2DoseLinear", "unit":"mJ/cm2",
"target":80, "spec":5
}
)";

  FEMData data;
  FEMConfig::ParseContent(config_quoted, data);
  cout << "Parsed quoted config:" << endl;
  cout << "  folder: [" << data.folderPattern << "]" << endl;
  cout << "  filename: [" << data.filenamePattern << "]" << endl;
  cout << "  sheet: [" << data.sheetPattern << "]" << endl;
}

// Test 2: Parse config with single quotes
void test_single_quoted_values() {
  cout << "\n========== Test 2: Single Quoted Values ==========" << endl;
  string config_single = R"(
folder = '*data*'
filename = 'AT00229.01-s25-AA (IMAGE) E STEP 0.05.xlsx'
sheet = 'TSK'
dose={"mode":"LowHigh", "unit":"mJ/cm2",
"center":4, "step":0.07, "no": 26,
"cols":"B:K"
}
focus={"mode":"NegPos2", "unit":"μm",
"center":0, "step":0.03, "no": 29,
"rows":"3:60"
}
fem={"mode":"Focus2DoseLinear", "unit":"mJ/cm2",
"target":80, "spec":5
}
)";

  FEMData data;
  FEMConfig::ParseContent(config_single, data);
  cout << "Parsed single-quoted config:" << endl;
  cout << "  folder: [" << data.folderPattern << "]" << endl;
  cout << "  filename: [" << data.filenamePattern << "]" << endl;
  cout << "  sheet: [" << data.sheetPattern << "]" << endl;
}

// Test 3: Parse config without quotes
void test_unquoted_values() {
  cout << "\n========== Test 3: Unquoted Values ==========" << endl;
  string config_unquoted = R"(
folder = *data*
filename = AT00229.01-s25-AA (IMAGE) E STEP 0.05.xlsx
sheet = TSK
dose={"mode":"LowHigh", "unit":"mJ/cm2",
"center":4, "step":0.07, "no": 26,
"cols":"B:K"
}
focus={"mode":"NegPos2", "unit":"μm",
"center":0, "step":0.03, "no": 29,
"rows":"3:60"
}
fem={"mode":"Focus2DoseLinear", "unit":"mJ/cm2",
"target":80, "spec":5
}
)";

  FEMData data;
  FEMConfig::ParseContent(config_unquoted, data);
  cout << "Parsed unquoted config:" << endl;
  cout << "  folder: [" << data.folderPattern << "]" << endl;
  cout << "  filename: [" << data.filenamePattern << "]" << endl;
  cout << "  sheet: [" << data.sheetPattern << "]" << endl;
}

// Test 4: Mixed quoted and unquoted values
void test_mixed_values() {
  cout << "\n========== Test 4: Mixed Quoted/Unquoted Values ==========" << endl;
  string config_mixed = R"(
folder = "*data*"
filename = AT00229.01-s25-AA (IMAGE) E STEP 0.05.xlsx
sheet = 'TSK'
dose={"mode":"LowHigh", "unit":"mJ/cm2",
"center":4, "step":0.07, "no": 26,
"cols":"B:K"
}
focus={"mode":"NegPos2", "unit":"μm",
"center":0, "step":0.03, "no": 29,
"rows":"3:60"
}
fem={"mode":"Focus2DoseLinear", "unit":"mJ/cm2",
"target":80, "spec":5
}
)";

  FEMData data;
  FEMConfig::ParseContent(config_mixed, data);
  cout << "Parsed mixed-quoted config:" << endl;
  cout << "  folder: [" << data.folderPattern << "]" << endl;
  cout << "  filename: [" << data.filenamePattern << "]" << endl;
  cout << "  sheet: [" << data.sheetPattern << "]" << endl;
}

// Test 5: Dump and verify normalized output format
void test_dump_normalization() {
  cout << "\n========== Test 5: Dump Normalization ==========" << endl;

  FEMData data;
  data.folderPattern = "*data*";
  data.filenamePattern = "AT00229.01-s25-AA (IMAGE) E STEP 0.05.xlsx";
  data.sheetPattern = "TSK";
  data.dose.mode = "LowHigh";
  data.dose.unit = "mJ/cm2";
  data.dose.center = 4;
  data.dose.step = 0.07;
  data.dose.no = 26;
  data.dose.cols = "B:K";
  data.focus.mode = "NegPos2";
  data.focus.unit = "μm";
  data.focus.center = 0;
  data.focus.step = 0.03;
  data.focus.no = 29;
  data.focus.rows = "3:60";
  data.fem.mode = "Focus2DoseLinear";
  data.fem.unit = "mJ/cm2";
  data.fem.target = 80;
  data.fem.spec = 5;

  // Dump to temp file
  FEMConfig::dumpFEMData(data, "./test_dump_normalized.fem");

  // Read and display the dumped content
  string content = FEMConfig::GetFileContent("./test_dump_normalized.fem");
  cout << "Dumped content (first 300 chars):" << endl;
  cout << content.substr(0, 300) << "..." << endl;

  // Verify that quotes are present
  if (content.find("folder = \"") != string::npos &&
      content.find("filename = \"") != string::npos &&
      content.find("sheet = \"") != string::npos) {
    cout << "✓ Output is properly quoted" << endl;
  } else {
    cout << "✗ Output is NOT properly quoted" << endl;
  }
}

// Test 6: Round-trip test (parse -> dump -> parse)
void test_round_trip() {
  cout << "\n========== Test 6: Round-Trip Test ==========" << endl;

  string original_config = R"(
folder = "*data*"
filename = "test.xlsx"
sheet = "Sheet1"
dose={"mode":"LowHigh", "unit":"mJ/cm2",
"center":5, "step":0.1, "no": 30,
"cols":"A:Z"
}
focus={"mode":"NegPos2", "unit":"μm",
"center":0, "step":0.05, "no": 25,
"rows":"1:100"
}
fem={"mode":"Focus2DoseLinear", "unit":"mJ/cm2",
"target":100, "spec":10
}
)";

  // Parse original
  FEMData data1;
  FEMConfig::ParseContent(original_config, data1);
  cout << "Original parsed:" << endl;
  cout << "  folder: [" << data1.folderPattern << "]" << endl;
  cout << "  filename: [" << data1.filenamePattern << "]" << endl;
  cout << "  sheet: [" << data1.sheetPattern << "]" << endl;

  // Dump
  FEMConfig::dumpFEMData(data1, "./test_round_trip.fem");

  // Parse dumped
  FEMData data2;
  FEMConfig::ReadFile("./test_round_trip.fem", data2);
  cout << "After round-trip:" << endl;
  cout << "  folder: [" << data2.folderPattern << "]" << endl;
  cout << "  filename: [" << data2.filenamePattern << "]" << endl;
  cout << "  sheet: [" << data2.sheetPattern << "]" << endl;

  // Verify
  if (data1.folderPattern == data2.folderPattern &&
      data1.filenamePattern == data2.filenamePattern &&
      data1.sheetPattern == data2.sheetPattern) {
    cout << "✓ Round-trip successful" << endl;
  } else {
    cout << "✗ Round-trip failed" << endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <FEM file path>" << std::endl;
    return 1;
  }
  FEMData data;

  auto filePath = string(argv[1]);
  if (!FEMConfig::ReadFile(filePath, data)) {
    std::cerr << "Failed to read FEM file: " << filePath << std::endl;
    return 2;
  }

  // test_print_fem_data(data);
  fs::path path = filePath;
  fs::path abspath = fs::absolute(path).parent_path(); // file -> folder
  cout << "Current FEM file absolute folder: " << abspath.string() << std::endl;
  fs::current_path(abspath);
  cout << "Changed working directory to: " << fs::current_path().string() << endl;

  auto folders = FEMConfig::ExpandFolderPattern(data);
  if (folders.empty()) return 3;
  auto folder = folders[0];
  cout << "Using folder: " << folder << endl;

  auto filenames = FEMConfig::ExpandFilenamePattern(folder, data);
  if (filenames.empty()) return 4;
  auto filename = filenames[0];
  cout << "Using filename: " << filename << endl;

  auto sheets = FEMConfig::ExpandSheetPattern(filename, data);
  if (sheets.empty()) return 5;
  auto sheet = sheets[0];
  cout << "Using sheet: " << sheet << endl;

  minixlsx::OpenXLSXWrapper wrapper;
  if (!wrapper.open(filename)) {
    std::cerr << "[ERROR] Failed to open Excel file: " << filename << std::endl;
    return 6;
  }
  cout << "Opened Excel file successfully." << endl;

  auto indexOpt = wrapper.sheetIndex(sheet);
  if (!indexOpt.has_value()) {
    std::cerr << "[ERROR] Sheet not found: " << sheet << std::endl;
    wrapper.close();
    return 7;
  }
  cout << "Accessed worksheet successfully." << endl;

  auto cellValue = wrapper.getCellValue(indexOpt.value(), "J8");
  if (cellValue.has_value()) {
    cout << "Cell J8 value: " << cellValue.value() << endl;
  } else {
    cout << "Cell J8 value: <empty>" << endl;
  }

  wrapper.close();
  return 0;

  // Run embedded tests
  test_quoted_values();
  test_single_quoted_values();
  test_unquoted_values();
  test_mixed_values();
  test_dump_normalization();
  test_round_trip();

  // If argument provided, also test with file
  if (argc >= 2) {
    cout << "\n========== Test 7: File-based Test ==========" << endl;
    string filePath = argv[1];

    FEMData data;
    if (!FEMConfig::ReadFile(filePath, data)) {
      std::cerr << "Failed to read FEM file: " << filePath << std::endl;
      return 1;
    }
    test_print_fem_data(data);

    //  切换工作目录到 FEM 文件所在目录
    fs::path path = filePath;
    fs::path abspath = fs::absolute(path).parent_path(); // file -> folder
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
  }

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
