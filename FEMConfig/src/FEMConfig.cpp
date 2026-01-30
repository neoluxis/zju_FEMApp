#include "cc/neolux/FEMConfig/FEMConfig.h"
#include <fstream>
#include <sstream>
#include <windows.h>
#include <cstring>
#include <algorithm>
#include <cctype>

#include <filesystem>
#include <regex>
#include "string"
#include "iostream"


namespace fs = std::filesystem;


namespace cc::neolux::femconfig {
  std::string FEMConfig::Trim(const std::string &str) {
    size_t start = 0;
    size_t end = str.length();

    while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
      start++;
    }

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
      end--;
    }

    return str.substr(start, end - start);
  }

  std::map<std::string, std::string> FEMConfig::ParseJsonConfig(const std::string &jsonStr) {
    std::map<std::string, std::string> result;

    // Find { and }
    size_t start = jsonStr.find('{');
    size_t end = jsonStr.rfind('}');

    if (start == std::string::npos || end == std::string::npos || start >= end) {
      return result;
    }

    std::string content = jsonStr.substr(start + 1, end - start - 1);

    // Simple key-value pair parsing
    size_t pos = 0;
    while (pos < content.length()) {
      // Skip whitespace and commas
      while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ',')) {
        pos++;
      }

      if (pos >= content.length())
        break;

      // Find key
      size_t keyStart = pos;
      if (content[pos] == '"') {
        keyStart = ++pos;
        while (pos < content.length() && content[pos] != '"')
          pos++;
      } else {
        while (pos < content.length() && content[pos] != ':' && !std::isspace(content[pos]))
          pos++;
      }

      if (pos >= content.length())
        break;

      std::string key = Trim(content.substr(keyStart, pos - keyStart));

      // Skip quotes and colons
      while (pos < content.length() && (content[pos] == '"' || content[pos] == ':' || std::isspace(content[pos]))) {
        pos++;
      }

      if (pos >= content.length())
        break;

      // Find value
      size_t valueStart = pos;
      bool inQuotes = false;

      if (content[pos] == '"') {
        inQuotes = true;
        valueStart = ++pos;
        while (pos < content.length() && content[pos] != '"')
          pos++;
      } else {
        while (pos < content.length() && content[pos] != ',' && content[pos] != '}')
          pos++;
      }

      std::string value = Trim(content.substr(valueStart, pos - valueStart));

      // Remove trailing quotes from value
      while (!value.empty() && value.back() == '"') {
        value.pop_back();
      }

      if (inQuotes && pos < content.length())
        pos++; // Skip closing quote

      result[key] = value;
    }

    return result;
  }

  bool FEMConfig::ParseContent(const std::string &content, FEMData &data) {
    data.rawContent = content;

    std::istringstream iss(content);
    std::string line;

    while (std::getline(iss, line)) {
      line = Trim(line);

      // Skip empty lines and comments
      if (line.empty() || line.substr(0, 2) == "//") {
        continue;
      }

      // Find equals sign
      size_t eqPos = line.find('=');
      if (eqPos == std::string::npos)
        continue;

      std::string key = Trim(line.substr(0, eqPos));
      std::string value = Trim(line.substr(eqPos + 1));

      // Handle multi-line config (with braces)
      if (value.find('{') != std::string::npos && value.find('}') == std::string::npos) {
        // Continue reading until closing brace
        std::string nextLine;
        while (std::getline(iss, nextLine)) {
          value += " " + nextLine;
          if (nextLine.find('}') != std::string::npos) {
            break;
          }
        }
      }

      if (key == "folder") {
        data.folderPattern = value;
      } else if (key == "filename") {
        data.filenamePattern = value;
      } else if (key == "sheet") {
        data.sheetPattern = value;
      } else if (key == "dose") {
        auto config = ParseJsonConfig(value);
        data.dose.mode = config["mode"];
        data.dose.unit = config["unit"];
        data.dose.center = config.count("center") ? std::stod(config["center"]) : 0.0;
        data.dose.step = config.count("step") ? std::stod(config["step"]) : 0.0;
        data.dose.no = config.count("no") ? std::stoi(config["no"]) : 0;
        data.dose.cols = config["cols"];
      } else if (key == "focus") {
        auto config = ParseJsonConfig(value);
        data.focus.mode = config["mode"];
        data.focus.unit = config["unit"];
        data.focus.center = config.count("center") ? std::stod(config["center"]) : 0.0;
        data.focus.step = config.count("step") ? std::stod(config["step"]) : 0.0;
        data.focus.no = config.count("no") ? std::stoi(config["no"]) : 0;
        data.focus.rows = config["rows"];
      } else if (key == "fem") {
        auto config = ParseJsonConfig(value);
        data.fem.mode = config["mode"];
        data.fem.unit = config["unit"];
        data.fem.target = config.count("target") ? std::stod(config["target"]) : 0.0;
        data.fem.spec = config.count("spec") ? std::stod(config["spec"]) : 0.0;
      }
    }

    return true;
  }

  bool FEMConfig::ReadFile(const std::string &filePath, FEMData &data) {
    std::string content = GetFileContent(filePath);
    if (content.empty()) {
      return false;
    }

    return ParseContent(content, data);
  }

  bool FEMConfig::ReadFileW(const wchar_t *filePath, FEMData &data) {
    std::string content = GetFileContentW(filePath);
    if (content.empty()) {
      return false;
    }

    return ParseContent(content, data);
  }

  std::string FEMConfig::GetFileContent(const std::string &filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  bool FEMConfig::IsValidFEMFile(const std::string &filePath) {
    if (filePath.length() < 4) {
      return false;
    }

    std::string ext = filePath.substr(filePath.length() - 4);
    return ext == ".fem";
  }

  bool FEMConfig::IsValidFEMFileW(const wchar_t *filePath) {
    if (!filePath)
      return false;

    int len = wcslen(filePath);
    if (len < 4)
      return false;

    return wcscmp(filePath + len - 4, L".fem") == 0;
  }

  std::string FEMConfig::GetFileContentW(const wchar_t *filePath) {
    if (!filePath)
      return "";

    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
      return "";
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
      CloseHandle(hFile);
      return "";
    }

    std::string content(fileSize, '\0');
    DWORD bytesRead = 0;

    if (!::ReadFile(hFile, &content[0], fileSize, &bytesRead, NULL)) {
      CloseHandle(hFile);
      return "";
    }

    CloseHandle(hFile);
    return content;
  }

  // 简单通配符匹配，* 匹配 0 个或多个字符
  inline bool WildcardMatch(const std::string &str, const std::string &pattern) {
    size_t s = 0, p = 0, star = std::string::npos, ss = 0;
    std::cout << "Matching string: " << str << " with pattern: " << pattern << std::endl;
    while (s < str.size()) {
      if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == str[s])) {
        s++;
        p++;
      } else if (p < pattern.size() && pattern[p] == '*') {
        star = p++;
        ss = s;
      } else if (star != std::string::npos) {
        p = star + 1;
        s = ++ss;
      } else {
        return false;
      }
    }

    while (p < pattern.size() && pattern[p] == '*') p++;
    return p == pattern.size();
  }

  std::vector<std::string> FEMConfig::ExpandFolderPattern(FEMData &data) {
    std::vector<std::string> result;

    fs::path folder = fs::current_path();

    if (!fs::exists(folder) || !fs::is_directory(folder))
      return result;
    for (const auto &entry: fs::directory_iterator(folder)) {
      if (!fs::is_directory(entry.status()))
        continue;

      std::string dirname = entry.path().filename().u8string(); // 支持 UTF-8
      if (WildcardMatch(dirname, data.folderPattern)) {
        result.push_back(entry.path().string());
      }
    }
    return result;
  }


  std::vector<std::string> FEMConfig::ExpandFilenamePattern(const std::string &folder, const FEMData &data)
  {
    std::vector<std::string> result;

    fs::path folderPath(folder);

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
      return result;

    for (const auto &entry : fs::directory_iterator(folderPath)) {
      if (!fs::is_regular_file(entry.status()))
        continue;

      std::string filename = entry.path().filename().u8string(); // 支持 UTF-8
      if (WildcardMatch(filename, data.filenamePattern)) {
        result.push_back(entry.path().string());
      }
    }

    return result;
  }
  std::vector<std::string> FEMConfig::ExpandFilenamePattern(FEMData &data)
  {
    auto folders = ExpandFolderPattern(data);
    std::vector<std::string> result;
    for (const auto &folder : folders) {
      auto filenames = ExpandFilenamePattern(folder, data);
      result.insert(result.end(), filenames.begin(), filenames.end());
    }
    return result;
  }

  // 展开工作表通配符
  // 这里暂时只是返回 pattern 本身或匹配列表（如果有 sheet 列表可用）
  std::vector<std::string> FEMConfig::ExpandSheetPattern(FEMData &data) {
    // 如果你有实际 sheet 名称列表，可以在这里匹配
    // 暂时返回 pattern 自己作为示例
    std::vector<std::string> sheets;
    sheets.push_back(data.sheetPattern);
    return sheets;
  }
} // namespace cc::neolux::femconfig
