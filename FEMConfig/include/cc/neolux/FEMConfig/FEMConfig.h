#ifndef __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__
#define __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__

#include <string>
#include <vector>
#include <map>

namespace cc::neolux::femconfig
{

    // Dose configuration
    struct DoseConfig
    {
        std::string mode; // LowHigh, HighLow, etc.
        std::string unit; // mJ/cm2, etc.
        double center;    // Center value
        double step;      // Step size
        int no;           // Number of steps
        std::string cols; // Excel column range e.g. "B:K"
    };

    // Focus configuration
    struct FocusConfig
    {
        std::string mode; // NegPos2, PosNeg2, etc.
        std::string unit; // μm, etc.
        double center;    // Center value
        double step;      // Step size
        int no;           // Number of steps
        std::string rows; // Excel row range e.g. "3:60"
    };

    // FEM configuration
    struct FEMConfigData
    {
        std::string mode; // Focus2DoseLinear, etc.
        std::string unit; // mJ/cm2, etc.
        double target;    // Target value
        double spec;      // Specification
    };

    // Complete FEM project data
    struct FEMData
    {
        std::string folderPattern;   // folder = *data*
        std::string filenamePattern; // filename = *AT00580.27*
        std::string sheetPattern;    // sheet = *DNo.3*

        DoseConfig dose;
        FocusConfig focus;
        FEMConfigData fem;

        std::string rawContent; // Raw file content
    };

    class FEMConfig
    {
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

    private:
        // Parse content to FEMData
        static bool ParseContent(const std::string &content, FEMData &data);

        // Parse JSON-style config string
        static std::map<std::string, std::string> ParseJsonConfig(const std::string &jsonStr);

        // Trim whitespace from string
        static std::string Trim(const std::string &str);
    };

} // namespace cc::neolux::femconfig

#endif // __CC_NEOLUX_FEMCONFIG_FEMCONFIG_H__
