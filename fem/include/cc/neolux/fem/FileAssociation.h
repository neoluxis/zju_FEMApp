#ifndef __CC_NEOLUX_FEM_FILEASSOCIATION_H__
#define __CC_NEOLUX_FEM_FILEASSOCIATION_H__

#include <string>

namespace cc::neolux::fem
{

    class FileAssociation
    {
    public:
        /**
         * 检查指定扩展名是否已关联到某个程序
         *
         * @param extension 文件扩展名（包括点，例如 ".fem"）
         * @return true 如果已关联，false 否则
         */
        static bool IsAssociated(const std::string &extension);

        /**
         * 获取当前可执行文件的路径
         */
        static std::string GetExecutablePath();

        /**
         * 获取与指定扩展名关联的程序路径
         *
         * @param extension 文件扩展名（包括点，例如 ".fem"）
         * @return 关联的程序路径，如果没有关联则返回空字符串
         */
        static std::string GetAssociatedProgram(const std::string &extension);

        /**
         * 注册文件关联
         *
         * @param extension 文件扩展名（包括点，例如 ".fem"）
         * @param progId 程序标识符（例如 "FemApp.Document"
         * @param description 文件类型描述（例如 "FEM Analysis File"）
         * @return true 如果注册成功，false 否则
         */
        static bool RegisterAssociation(const std::string &extension, const std::string &progId, const std::string &description);

        /**
         * 删除文件关联
         * 
         * @param extension 文件扩展名（包括点，例如 ".fem"）
         * @return true 如果删除成功，false 否则
         */
        static bool UnregisterAssociation(const std::string &extension);
    };

} // namespace cc::neolux::fem

#endif // __CC_NEOLUX_FEM_FILEASSOCIATION_H__