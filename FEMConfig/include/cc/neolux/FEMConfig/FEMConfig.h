#ifndef __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__
#define __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__

#include <string>
#include <vector>
#include <map>
#include <vector>

#if defined(_WIN32) || defined(__CYGWIN__)
#ifdef FEMCONFIG_EXPORTS
#define FEMCONFIG_API __declspec(dllexport)
#else
#define FEMCONFIG_API __declspec(dllimport)
#endif
#else
#define FEMCONFIG_API
#endif

namespace cc::neolux::femconfig {
  // Dose configuration
  struct FEMCONFIG_API DoseConfig {
    std::string mode; // LowHigh, HighLow, etc.
    std::string unit; // mJ/cm2, etc.
    double center; // Center value
    double step; // Step size
    int no; // Number of steps
    std::string cols; // Excel column range e.g. "B:K"
  };

  // Focus configuration
  struct FEMCONFIG_API FocusConfig {
    std::string mode; // NegPos2, PosNeg2, etc.
    std::string unit; // μm, etc.
    double center; // Center value
    double step; // Step size
    int no; // Number of steps
    std::string rows; // Excel row range e.g. "3:60"
  };

  // FEM configuration
  struct FEMCONFIG_API FEMConfigData {
    std::string mode; // Focus2DoseLinear, etc.
    std::string unit; // mJ/cm2, etc.
    double target; // Target value
    double spec; // Specification
  };

  // Complete FEM project data
  struct FEMCONFIG_API FEMData {
    std::string folderPattern; // folder = *data*
    std::string filenamePattern; // filename = *AT00580.27*
    std::string sheetPattern; // sheet = *DNo.3*

    DoseConfig dose;
    FocusConfig focus;
    FEMConfigData fem;

    std::string rawContent; // Raw file content
  };

  class FEMCONFIG_API FEMConfig {
  public:
    // Read and parse .fem file
    static bool ReadFile(const std::string &filePath, FEMData &data);

    // Read and parse .fem file (wide char path version)
    static bool ReadFileW(const wchar_t *filePath, FEMData &data);

    // Get file content
    static std::string GetFileContent(const std::string &filePath);

    // Get file content (wide char path version)
    static std::string GetFileContentW(const wchar_t *filePath);

    // Validate file format
    static bool IsValidFEMFile(const std::string &filePath);

    // Validate file format (wide char path version)
    static bool IsValidFEMFileW(const wchar_t *filePath);

    // static bool isValidConfig(const std::string &text);
    // static bool isValidConfigW(const wchar_t *text);
    /**
     * 展开文件夹通配符
     *
     * @param cwd 当前工作目录
     * @param data FEM配置信息
     * @return std::vector<std::string> 展开的文件夹列表
     */
    static std::vector<std::string> ExpandFolderPattern(FEMData &data);

    /**
     * 展开文件名通配符
     *
     * @param folder 文件夹路径
     * @param data FEM配置信息
     * @return std::vector<std::string> 展开的文件名列表
     */
    static std::vector<std::string> ExpandFilenamePattern(FEMData &data);

    static std::vector<std::string> ExpandFilenamePattern(const std::string &folder, const FEMData &data);

    /**
     * 展开工作表通配符
     *
     * @param data FEM配置信息
     * @return std::vector<std::string> 展开的工作表列表
     */
    static std::vector<std::string> ExpandSheetPattern(const std::string &filepath, const FEMData &data);

    static int columnLetterToNumber(const std::string &col);

    static int calculateNo(const std::string &str);

    static bool dumpFEMData(const FEMData &data, std::ostream &os);

    static bool dumpFEMData(const FEMData &data, const std::string &path);

    static bool ParseContent(const std::string &content, FEMData &data);

  private:
    // Parse content to FEMData

    // Parse JSON-style config string
    static std::map<std::string, std::string> ParseJsonConfig(const std::string &jsonStr);

    // Trim whitespace from string
    static std::string Trim(const std::string &str);
  };
} // namespace cc::neolux::femconfig

#endif // __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__
