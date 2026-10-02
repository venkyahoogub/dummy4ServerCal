// Implements...
#include "VersionService.h"

using namespace ncs;

VersionService::VersionService(const std::string& version) : mVersion(version) {
    if (mVersion == "") {
        throw VersionService::InvalidVersionError();
    }
}