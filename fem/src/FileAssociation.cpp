#include "cc/neolux/fem/FileAssociation.h"
#include <windows.h>
#include <iostream>
#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

namespace cc::neolux::fem
{

    std::string FileAssociation::GetExecutablePath()
    {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return std::string(path);
    }

    std::string FileAssociation::GetAssociatedProgram(const std::string &extension)
    {
        HKEY hKey;
        std::string regPath = extension;
        char progId[256] = {0};
        DWORD size = sizeof(progId);

        // 查询 HKEY_CURRENT_USER\Software\Classes\.fem
        std::string userClassesPath = "Software\\Classes\\" + extension;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, userClassesPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueExA(hKey, "", NULL, NULL, (LPBYTE)progId, &size) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);

                // 获取关联的程序路径
                std::string commandPath = "Software\\Classes\\" + std::string(progId) + "\\shell\\open\\command";
                if (RegOpenKeyExA(HKEY_CURRENT_USER, commandPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                {
                    char command[512] = {0};
                    size = sizeof(command);
                    if (RegQueryValueExA(hKey, "", NULL, NULL, (LPBYTE)command, &size) == ERROR_SUCCESS)
                    {
                        RegCloseKey(hKey);

                        // 提取可执行文件路径（去除引号和参数）
                        std::string cmdStr(command);
                        if (!cmdStr.empty())
                        {
                            if (cmdStr[0] == '"')
                            {
                                size_t endQuote = cmdStr.find('"', 1);
                                if (endQuote != std::string::npos)
                                {
                                    return cmdStr.substr(1, endQuote - 1);
                                }
                            }
                            else
                            {
                                size_t spacePos = cmdStr.find(' ');
                                if (spacePos != std::string::npos)
                                {
                                    return cmdStr.substr(0, spacePos);
                                }
                                return cmdStr;
                            }
                        }
                    }
                    RegCloseKey(hKey);
                }
            }
            else
            {
                RegCloseKey(hKey);
            }
        }

        // 如果用户注册表中没有，检查 HKEY_CLASSES_ROOT
        if (RegOpenKeyExA(HKEY_CLASSES_ROOT, extension.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            size = sizeof(progId);
            if (RegQueryValueExA(hKey, "", NULL, NULL, (LPBYTE)progId, &size) == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);

                std::string commandPath = std::string(progId) + "\\shell\\open\\command";
                if (RegOpenKeyExA(HKEY_CLASSES_ROOT, commandPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                {
                    char command[512] = {0};
                    size = sizeof(command);
                    if (RegQueryValueExA(hKey, "", NULL, NULL, (LPBYTE)command, &size) == ERROR_SUCCESS)
                    {
                        RegCloseKey(hKey);

                        std::string cmdStr(command);
                        if (!cmdStr.empty())
                        {
                            if (cmdStr[0] == '"')
                            {
                                size_t endQuote = cmdStr.find('"', 1);
                                if (endQuote != std::string::npos)
                                {
                                    return cmdStr.substr(1, endQuote - 1);
                                }
                            }
                            else
                            {
                                size_t spacePos = cmdStr.find(' ');
                                if (spacePos != std::string::npos)
                                {
                                    return cmdStr.substr(0, spacePos);
                                }
                                return cmdStr;
                            }
                        }
                    }
                    RegCloseKey(hKey);
                }
            }
            else
            {
                RegCloseKey(hKey);
            }
        }

        return "";
    }

    bool FileAssociation::IsAssociated(const std::string &extension)
    {
        std::string currentExe = GetExecutablePath();
        std::string associatedExe = GetAssociatedProgram(extension);

        if (associatedExe.empty())
        {
            return false;
        }

        // 比较路径（不区分大小写）
        return _stricmp(currentExe.c_str(), associatedExe.c_str()) == 0;
    }

    bool FileAssociation::RegisterAssociation(const std::string &extension, const std::string &progId, const std::string &description)
    {
        HKEY hKey;
        std::string exePath = GetExecutablePath();

        // 创建扩展名关联
        std::string extPath = "Software\\Classes\\" + extension;
        if (RegCreateKeyExA(HKEY_CURRENT_USER, extPath.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        {
            return false;
        }

        if (RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)progId.c_str(), progId.length() + 1) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return false;
        }
        RegCloseKey(hKey);

        // 创建 ProgId
        std::string progIdPath = "Software\\Classes\\" + progId;
        if (RegCreateKeyExA(HKEY_CURRENT_USER, progIdPath.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        {
            return false;
        }

        if (RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)description.c_str(), description.length() + 1) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return false;
        }
        RegCloseKey(hKey);

        // 创建打开命令
        std::string commandPath = progIdPath + "\\shell\\open\\command";
        if (RegCreateKeyExA(HKEY_CURRENT_USER, commandPath.c_str(), 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        {
            return false;
        }

        std::string command = "\"" + exePath + "\" \"%1\"";
        if (RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)command.c_str(), command.length() + 1) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return false;
        }
        RegCloseKey(hKey);

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        return true;
    }

    bool FileAssociation::UnregisterAssociation(const std::string &extension)
    {
        std::string extPath = "Software\\Classes\\" + extension;

        // 获取当前关联的 ProgId
        HKEY hKey;
        char progId[256] = {0};
        DWORD size = sizeof(progId);

        if (RegOpenKeyExA(HKEY_CURRENT_USER, extPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            RegQueryValueExA(hKey, "", NULL, NULL, (LPBYTE)progId, &size);
            RegCloseKey(hKey);
        }

        RegDeleteTreeA(HKEY_CURRENT_USER, extPath.c_str());

        if (strlen(progId) > 0)
        {
            std::string progIdPath = "Software\\Classes\\" + std::string(progId);
            RegDeleteTreeA(HKEY_CURRENT_USER, progIdPath.c_str());
        }

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
        return true;
    }

} // namespace cc::neolux::fem