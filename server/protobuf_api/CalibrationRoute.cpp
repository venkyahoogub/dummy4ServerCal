#include "CalibrationRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

CalibrationRoute::CalibrationRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString CalibrationRoute::startCalibration(
    const CalibrationApi::StartCalibration& request) {
    
    FromServer response;
    auto* calibrationResult = response.mutable_calibration_result();

    try {
        // Extract dac_count from the request
        uint32_t irradiance_mw_per_cm2 = request.irradiance_mw_per_cm2();
        const uint32_t dacCount = request.dac_count();

        // Default irradiance to 30 if not provided
        if (irradiance_mw_per_cm2 == 0) {
            irradiance_mw_per_cm2 = FIXED_IRRADIANCE_MW_PER_CM2;
        }

        // Validate dac_count is not zero
        if (dacCount == 0) {
            // Return error entry with zero values to indicate failure
            auto* entry = calibrationResult->mutable_entry();
            entry->set_irradiance_mw_per_cm2(0);
            entry->set_dac_count(0);
            std::cerr << "DAC count must be greater than 0\n";
            return response.SerializeAsString();
        }

        // Call IHbcService to start calibration with fixed irradiance and requested dac_count
        // This should return the calibration table entry
        CalibrationTableEntry entry = mHbcService.startCalibration(irradiance_mw_per_cm2, dacCount);

        // Populate the CalibrationResult with the entry
        auto* resultEntry = calibrationResult->mutable_entry();
        resultEntry->set_irradiance_mw_per_cm2(entry.irradiance_mw_per_cm2());
        resultEntry->set_dac_count(entry.dac_count());
        resultEntry->set_pd1_count(entry.pd1_count());
        resultEntry->set_pd2_count(entry.pd2_count());
        resultEntry->set_pd3_count(entry.pd3_count());
        resultEntry->set_current_count(entry.current_count());

    } catch (const std::exception& e) {
        std::cerr << "Exception in startCalibration: " << e.what() << "\n";
        // Return error entry with zero values
        auto* entry = calibrationResult->mutable_entry();
        entry->set_irradiance_mw_per_cm2(0);
        entry->set_dac_count(0);
    }

    return response.SerializeAsString();
}