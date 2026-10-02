#pragma once

// Dependencies
#include "IVersionService.h"
#include "common/TracedError.h"

namespace ncs {
class VersionService : public IVersionService {
   public:
    explicit VersionService(const std::string& version);

    inline std::string getVersion() const override { return mVersion; }

    class InvalidVersionError : public TracedError {
       public:
        explicit InvalidVersionError()
            : TracedError("An invalid version was supplied.") {}
    };

   private:
    std::string mVersion;
};
}  // namespace ncs