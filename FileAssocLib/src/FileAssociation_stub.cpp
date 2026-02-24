#ifdef FEMAPP_SKIP_WINDOWS_API

#include <iostream>
#include <string>

#include "cc/neolux/fem/FileAssociation.h"

namespace cc::neolux::fem {

bool FileAssociation::IsAssociated(const std::string& extension) {
    // Dummy: always return false
    std::cout << "FileAssociation::IsAssociated called with " << extension << " (dummy)"
              << std::endl;
    return false;
}

bool FileAssociation::RegisterAssociation(const std::string& extension, const std::string& progId,
                                          const std::string& description) {
    // Dummy: do nothing
    std::cout << "FileAssociation::RegisterAssociation called with " << extension << ", " << progId
              << ", " << description << " (dummy)" << std::endl;
    return true;
}

bool FileAssociation::UnregisterAssociation(const std::string& extension) {
    // Dummy: do nothing
    std::cout << "FileAssociation::UnregisterAssociation called with " << extension << " (dummy)"
              << std::endl;
    return true;
}

}  // namespace cc::neolux::fem

#endif
