//
// Created by neolux on 2026/1/31.
//

#include "cc/neolux/fem/FileAssociation.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <iostream>

namespace cc::neolux::fem {

bool FileAssociation::IsAssociated(const std::string& extension) {
  HKEY hKey;
  std::string keyPath = "Software\\Classes\\." + extension;
  if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    RegCloseKey(hKey);
    return true;
  }
  return false;
}

bool FileAssociation::RegisterAssociation(const std::string& extension,
                                          const std::string& progId,
                                          const std::string& description) {
  HKEY hKey;
  std::string extKey = "Software\\Classes\\." + extension;
  std::string progKey = "Software\\Classes\\" + progId;

  // Register extension
  if (RegCreateKeyExA(HKEY_CURRENT_USER, extKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS) {
    return false;
  }
  RegSetValueExA(hKey, nullptr, 0, REG_SZ, (BYTE*)progId.c_str(), progId.size() + 1);
  RegCloseKey(hKey);

  // Register ProgID
  if (RegCreateKeyExA(HKEY_CURRENT_USER, progKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS) {
    return false;
  }
  RegSetValueExA(hKey, nullptr, 0, REG_SZ, (BYTE*)description.c_str(), description.size() + 1);
  RegCloseKey(hKey);

  // Register command
  std::string cmdKey = progKey + "\\shell\\open\\command";
  if (RegCreateKeyExA(HKEY_CURRENT_USER, cmdKey.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS) {
    return false;
  }
  char exePath[MAX_PATH];
  GetModuleFileNameA(nullptr, exePath, MAX_PATH);
  std::string command = std::string("\"") + exePath + "\" \"%1\"";
  RegSetValueExA(hKey, nullptr, 0, REG_SZ, (BYTE*)command.c_str(), command.size() + 1);
  RegCloseKey(hKey);

  // Notify shell
  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
  return true;
}

bool FileAssociation::UnregisterAssociation(const std::string& extension) {
  std::string extKey = "Software\\Classes\\." + extension;
  std::string progId;
  HKEY hKey;
  if (RegOpenKeyExA(HKEY_CURRENT_USER, extKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (RegQueryValueExA(hKey, nullptr, nullptr, nullptr, (BYTE*)buffer, &size) == ERROR_SUCCESS) {
      progId = buffer;
    }
    RegCloseKey(hKey);
  }

  RegDeleteKeyA(HKEY_CURRENT_USER, extKey.c_str());
  if (!progId.empty()) {
    std::string progKey = "Software\\Classes\\" + progId;
    RegDeleteKeyA(HKEY_CURRENT_USER, (progKey + "\\shell\\open\\command").c_str());
    RegDeleteKeyA(HKEY_CURRENT_USER, (progKey + "\\shell\\open").c_str());
    RegDeleteKeyA(HKEY_CURRENT_USER, (progKey + "\\shell").c_str());
    RegDeleteKeyA(HKEY_CURRENT_USER, progKey.c_str());
  }

  SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
  return true;
}

} // namespace cc::neolux::fem
