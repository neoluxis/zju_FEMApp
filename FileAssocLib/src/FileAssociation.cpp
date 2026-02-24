//
// Created by neolux on 2026/1/31.
//

#include "cc/neolux/fem/FileAssociation.h"

#ifndef FEMAPP_SKIP_WINDOWS_API
// Windows implementation
#include <iostream>
#include <shlobj.h>
#include <string>
#include <windows.h>

namespace cc::neolux::fem {

namespace {
std::string NormalizeExtension(const std::string& extension) {
    if (extension.empty()) return extension;
    if (extension[0] == '.') return extension.substr(1);
    return extension;
}

std::string ReadDefaultValueA(HKEY root, const std::string& subKey) {
    HKEY hKey = nullptr;
    if (RegOpenKeyExA(root, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return {};
    }

    DWORD type = 0;
    DWORD size = 0;
    if (RegQueryValueExA(hKey, nullptr, nullptr, &type, nullptr, &size) != ERROR_SUCCESS ||
        type != REG_SZ || size == 0) {
        RegCloseKey(hKey);
        return {};
    }

    std::string value(size, '\0');
    if (RegQueryValueExA(hKey, nullptr, nullptr, &type, reinterpret_cast<BYTE*>(&value[0]),
                         &size) != ERROR_SUCCESS ||
        type != REG_SZ) {
        RegCloseKey(hKey);
        return {};
    }
    RegCloseKey(hKey);

    if (!value.empty() && value.back() == '\0') value.pop_back();
    return value;
}
}  // namespace

bool FileAssociation::IsAssociated(const std::string& extension) {
    HKEY hKey;
    std::string ext = NormalizeExtension(extension);
    if (ext.empty()) return false;
    std::string keyPath = "Software\\Classes\\." + ext;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool FileAssociation::RegisterAssociation(const std::string& extension, const std::string& progId,
                                          const std::string& description) {
    HKEY hKey;
    std::string ext = NormalizeExtension(extension);
    if (ext.empty() || progId.empty()) return false;
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    char* fileName = strrchr(exePath, '\\');
    if (fileName)
        fileName++;
    else
        fileName = exePath;
    std::string command = std::string("\"") + exePath + "\" \"%1\"";

    // Create ProgID key
    std::string progIdKey = "Software\\Classes\\" + progId;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, progIdKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr,
                        &hKey, nullptr) != ERROR_SUCCESS) {
        return false;
    }
    if (!description.empty()) {
        RegSetValueExA(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(description.c_str()),
                       description.size() + 1);
    }
    RegCloseKey(hKey);

    // Create shell\\open\\command under ProgID
    std::string cmdKey = progIdKey + "\\shell\\open\\command";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, cmdKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey,
                        nullptr) != ERROR_SUCCESS) {
        return false;
    }
    RegSetValueExA(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(command.c_str()),
                   command.size() + 1);
    RegCloseKey(hKey);

    // Register extension -> ProgID
    std::string extKey = "Software\\Classes\\." + ext;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, extKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey,
                        nullptr) != ERROR_SUCCESS) {
        return false;
    }
    RegSetValueExA(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(progId.c_str()),
                   progId.size() + 1);
    RegCloseKey(hKey);

    // Notify shell
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return true;
}

bool FileAssociation::CleanRegister(const std::string& extension, const std::string& progId,
                                    const std::string& description) {
    UnregisterAssociation(extension);
    return RegisterAssociation(extension, progId, description);
}

bool FileAssociation::UnregisterAssociation(const std::string& extension) {
    std::string ext = NormalizeExtension(extension);
    if (ext.empty()) return false;
    std::string extKey = "Software\\Classes\\." + ext;
    std::string progId = ReadDefaultValueA(HKEY_CURRENT_USER, extKey);
    RegDeleteTreeA(HKEY_CURRENT_USER, extKey.c_str());

    if (!progId.empty()) {
        std::string progIdKey = "Software\\Classes\\" + progId;
        RegDeleteTreeA(HKEY_CURRENT_USER, progIdKey.c_str());
    }

    // Clean up legacy registration path used by older versions.
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    char* fileName = strrchr(exePath, '\\');
    if (fileName)
        fileName++;
    else
        fileName = exePath;
    std::string exeName = fileName;
    std::string exeKey = "Software\\Classes\\Applications\\" + exeName;
    RegDeleteTreeA(HKEY_CURRENT_USER, exeKey.c_str());

    // Notify shell
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return true;
}

}  // namespace cc::neolux::fem

#endif
