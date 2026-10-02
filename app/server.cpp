#include <iostream>

// Dependencies
#include <memory>
#include <vector>

#include "camera/CameraService.h"
#include "camera/basler/BaslerCameraArray.h"
#include "camera/mock/MockCameraArray.h"
#include "neo-dmd-driver/src/iProjector.h"
#include "neo-dmd-driver/src/projector.h"
#include "server/CalibrationServer.h"
#include "server/IServiceRouter.h"
#include "server/MockDmdProjector.h"
#include "server/VersionService.h"
#include "server/protobuf_api/AboutRoute.h"
#include "server/protobuf_api/CameraRoute.h"
#include "server/protobuf_api/DmdRoute.h"
#include "server/protobuf_api/NirLedRoute.h"
#include "server/protobuf_api/CameraTriggerRoute.h"
#include "server/protobuf_api/CalibrationRoute.h"
#include "server/protobuf_api/MotorRoute.h"
#include "server/protobuf_api/UvDiagnosticRoute.h"
#include "server/protobuf_api/CalibrationTableRoute.h"
#include "server/protobuf_api/DistanceSensorRoute.h"
#include "server/protobuf_api/UvDeviceSettingsRoute.h"
#include "server/protobuf_api/GetDeviceSettingsRoute.h"
#include "server/protobuf_api/ProtobufRouter.h"
#include "services/IHbcService.h"
#include "services/MockHbcService.h"
#include "services/hbcService.h"
#include "version.h"

#define DEFAULT_PORT 50051

// Helpers
uint16_t getPort(int argc, char* argv[]);

int main(int argc, char* argv[]) {
    uint16_t port = getPort(argc, argv);
    std::cout << "Starting Calibration Server...\n";
    std::vector<std::unique_ptr<ncs::ICameraArray>> cameraArrays;

    std::unique_ptr<ncs::ICameraArray> pBaslerArray =
        std::make_unique<ncs::BaslerCameraArray>();
    cameraArrays.push_back(std::move(pBaslerArray));
    std::cout << "Adding Basler Camera Array...\n";

#ifdef DEBUG
    std::unique_ptr<ncs::ICameraArray> pMockArray =
        std::make_unique<ncs::MockCameraArray>("assets");
    cameraArrays.push_back(std::move(pMockArray));
    std::cout << "Adding Fallback Mock Array...\n";
#endif

    ncs::CameraService cameraService(std::move(cameraArrays));
    ncs::CameraRoute cameraRoute(cameraService);

    ncs::VersionService versionService(VERSION);
    ncs::AboutRoute aboutRoute(versionService);

    std::cout << "Creating HBC Service\n";
    auto rxHandler = [](Hbc::FromHbc::MsgCase msgCase,
                        const Hbc::FromHbc& msg) {};

    std::unique_ptr<ncs::IHbcService> pHbcService;
    try {
        pHbcService = std::make_unique<ncs::HbcService>(rxHandler);
        std::cout << "HBC: Device Detected...\n";
    } catch (...) {
#ifdef DEBUG
        std::cout << "HBC: Fallback Mock HBC\n";
        pHbcService = std::make_unique<ncs::MockHbcService>();
#else
        std::cout << "Could not find HBC device...\n";
        throw;
#endif
    }
    ncs::NirLedRoute nirLedRoute(*pHbcService);
    ncs::CameraTriggerRoute cameraTriggerRoute(*pHbcService);
    ncs::CalibrationRoute calibrationRoute(*pHbcService);
    ncs::MotorRoute motorRoute(*pHbcService);
    ncs::UvDiagnosticRoute uvDiagnosticRoute(*pHbcService);
    ncs::DistanceSensorRoute distanceSensorRoute(*pHbcService);
    ncs::UvDeviceSettingsRoute uvDeviceSettingsRoute(*pHbcService);
    ncs::GetDeviceSettingsRoute getDeviceSettingsRoute(*pHbcService);

    // All required routes are added here.
    std::cout << "Creating DMD Service\n";
    std::unique_ptr<IProjector> projector;
    try {
        projector = std::make_unique<Projector>();
        std::cout << "DMD: Device Detected\n";
    } catch (...) {
#ifdef DEBUG
        std::cout << "DMD: Fallback Mock DMD\n";
        projector = std::make_unique<ncs::MockDmdProjector>();
#else
        std::cout << "Could not find DMD device...\n";
        throw;
#endif
    }
    ncs::DmdRoute dmdRoute(*projector);
    ncs::CalibrationTableRoute calibrationTableRoute(*pHbcService);
    
    ncs::ProtobufRouter router(aboutRoute, cameraRoute, nirLedRoute, dmdRoute, 
        cameraTriggerRoute, calibrationRoute, motorRoute, uvDiagnosticRoute,
        calibrationTableRoute, distanceSensorRoute, uvDeviceSettingsRoute,
        getDeviceSettingsRoute);

    ncs::CalibrationServer server(port, router);
    std::cout << "Starting camera service...\n";

    cameraService.startup();

    std::cout << (cameraService.isReady() ? "Camera service started...\n"
                                          : "There was an issue starting...\n");

    std::cout << "Using: " << cameraService.leftCamera()->deviceName() << ", "
              << cameraService.rightCamera()->deviceName();
    if (cameraService.wideCamera()) {
        std::cout << ", " << cameraService.wideCamera()->deviceName();
    }
    std::cout << std::endl;

    try {
        std::cout << "Listening for connections...\n";
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Listener thread has been terminated unexpectedly: "
                  << e.what() << "\n";
        return 1;
    }

    if (server.getStatus() == ncs::CalibrationServer::Status::Error) {
        std::cout << "The server had an uncaught error.\n";
    }

    std::cout << "Server is shutting down..." << std::endl;
    cameraService.shutdown();

    return 0;
}

uint16_t getPort(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--port" && i + 1 < args.size()) {
            try {
                int p = std::stoi(args[i + 1]);
                if (p > 0 && p <= 65535) {
                    return static_cast<uint16_t>(p);
                }
                std::cerr << "Port out of range. Using default: "
                          << DEFAULT_PORT << "\n";
            } catch (...) {
                std::cerr << "Invalid port value. Using default: "
                          << DEFAULT_PORT << "\n";
            }
        }
    }
    return DEFAULT_PORT;
}