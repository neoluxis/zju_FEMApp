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
  /**
   * @brief Dose 配置
   */
  struct FEMCONFIG_API DoseConfig {
    std::string mode; // LowHigh, HighLow, etc.
    std::string unit; // mJ/cm2, etc.
    double center; // Center value
    double step; // Step size
    int no; // Number of steps
    std::string cols; // Excel column range e.g. "B:K"
  };

  /**
   * @brief Focus 配置
   */
  struct FEMCONFIG_API FocusConfig {
    std::string mode; // NegPos2, PosNeg2, etc.
    std::string unit; // μm, etc.
    double center; // Center value
    double step; // Step size
    int no; // Number of steps
    std::string rows; // Excel row range e.g. "3:60"
  };

  /**
   * @brief FEM 配置
   */
  struct FEMCONFIG_API FEMConfigData {
    std::string mode; // Focus2DoseLinear, etc.
    std::string unit; // mJ/cm2, etc.
    double target; // Target value
    double spec; // Specification
  };

  /**
   * @brief FEM 项目完整数据
   */
  struct FEMCONFIG_API FEMData {
    std::string folderPattern; // folder = *data*
    std::string filenamePattern; // filename = *AT00580.27*
    std::string sheetPattern; // sheet = *DNo.3*

    DoseConfig dose;
    FocusConfig focus;
    FEMConfigData fem;

    std::string rawContent; // Raw file content
  };

  /**
   * @brief FEM 配置解析与操作
   */
  class FEMCONFIG_API FEMConfig {
  public:
    /**
     * @brief 读取并解析 .fem 文件
     */
    static bool ReadFile(const std::string &filePath, FEMData &data);

    /**
     * @brief 读取并解析 .fem 文件（宽字符路径）
     */
    static bool ReadFileW(const wchar_t *filePath, FEMData &data);

    /**
     * @brief 读取文件内容
     */
    static std::string GetFileContent(const std::string &filePath);

    /**
     * @brief 读取文件内容（宽字符路径）
     */
    static std::string GetFileContentW(const wchar_t *filePath);

    /**
     * @brief 校验文件扩展名是否合法
     */
    static bool IsValidFEMFile(const std::string &filePath);

    /**
     * @brief 校验文件扩展名是否合法（宽字符路径）
     */
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

    /**
     * @brief 将 Excel 列字母转为数字
     */
    static int columnLetterToNumber(const std::string &col);

    /**
     * @brief 从范围表达式计算数量
     */
    static int calculateNo(const std::string &str);

    /**
     * @brief 将 FEM 数据写入流
     */
    static bool dumpFEMData(const FEMData &data, std::ostream &os);

    /**
     * @brief 将 FEM 数据写入文件
     */
    static bool dumpFEMData(const FEMData &data, const std::string &path);

    /**
     * @brief 解析 FEM 配置内容
     */
    static bool ParseContent(const std::string &content, FEMData &data);

  private:
    // Parse content to FEMData

    /**
     * @brief 解析 JSON 风格配置字符串
     */
    static std::map<std::string, std::string> ParseJsonConfig(const std::string &jsonStr);

    /**
     * @brief 去除字符串首尾空白
     */
    static std::string Trim(const std::string &str);
  };
} // namespace cc::neolux::femconfig

#endif // __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__
