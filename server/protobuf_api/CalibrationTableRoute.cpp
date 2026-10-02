#include "CalibrationTableRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

CalibrationTableRoute::CalibrationTableRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString CalibrationTableRoute::uploadCalibrationTable(
    const CalibrationApi::UploadCalibrationTable& request) {
    
    FromServer response;
    auto* uploadResponse = response.mutable_upload_calibration_table_response();

    try {
        const auto& table = request.table();
        
        // Log the upload
        std::cout << "Received calibration table with " << table.entries_size() 
                  << " entries\n";

        // TODO: Store the calibration table persistently
        // (database, file, possibly HBC storage based on discussions)

        // For now, I'll just acknowledge receipt and see what I can do with it later.
        uploadResponse->set_success(true);
        uploadResponse->set_message("Calibration table uploaded successfully");

    } catch (const std::exception& e) {
        uploadResponse->set_success(false);
        uploadResponse->set_message(
            std::string("Error uploading calibration table: ") + e.what());
        std::cerr << "Exception in uploadCalibrationTable: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}