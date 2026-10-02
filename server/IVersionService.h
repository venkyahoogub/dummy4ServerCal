#pragma once

// Dependencies
#include <string>

namespace ncs {
class IVersionService {
   public:
    ~IVersionService() = default;

    virtual std::string getVersion() const = 0;
};
}  // namespace ncs