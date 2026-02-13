#pragma once

#include <string>

namespace cc::neolux::fem {

class FileAssociation {
public:
  static bool IsAssociated(const std::string& extension);
  static bool RegisterAssociation(const std::string& extension,
                                  const std::string& progId,
                                  const std::string& description);
  static bool CleanRegister(const std::string& extension,
                            const std::string& progId,
                            const std::string& description);
  static bool UnregisterAssociation(const std::string& extension);
};

} // namespace cc::neolux::fem
