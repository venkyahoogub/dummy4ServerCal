// Implements
#include "AboutRoute.h"

// Dependencies
#include <common/SerializationUtils.h>

using namespace ncs;
using namespace CalibrationApi;

AboutRoute::AboutRoute(const IVersionService& versionService)
    : mVersionService(versionService) {}

ByteString AboutRoute::createResponse(const GetAbout& aboutRequest) {
    auto version = mVersionService.getVersion();
    FromServer outgoing;
    outgoing.mutable_about()->set_version(version);

    return outgoing.SerializeAsString();
}