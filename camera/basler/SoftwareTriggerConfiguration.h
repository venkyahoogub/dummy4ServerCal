#pragma once

#include "pylon/InstantCamera.h"
#include "pylon/Platform.h"

namespace ncs {
class SoftwareTriggerConfiguration : public Pylon::CConfigurationEventHandler {
   public:
    // Applying software trigger configuration.
    static void ApplyConfiguration(GENAPI_NAMESPACE::INodeMap& nodemap) {
        using namespace GENAPI_NAMESPACE;
        using namespace Pylon;

        // Get required enumerations for trigger types
        CEnumerationPtr triggerSelector(nodemap.GetNode("TriggerSelector"));
        CEnumerationPtr triggerMode(nodemap.GetNode("TriggerMode"));

        // Determine appropriate trigger type
        String_t triggerName("FrameStart");
        if (!IsAvailable(triggerSelector->GetEntryByName(triggerName))) {
            triggerName = "AcquisitionStart";
            if (!IsAvailable(triggerSelector->GetEntryByName(triggerName))) {
                throw RUNTIME_EXCEPTION(
                    "Could not select trigger. Neither FrameStart nor "
                    "AcquisitionStart is available.");
            }
        }

        GENAPI_NAMESPACE::NodeList_t triggerSelectorEntries;
        triggerSelector->GetEntries(triggerSelectorEntries);

        // Turn trigger mode off for all except the frame trigger
        for (auto it = triggerSelectorEntries.begin();
             it != triggerSelectorEntries.end(); ++it) {
            CEnumEntryPtr pEntry(*it);
            if (IsAvailable(pEntry)) {
                String_t triggerNameOfEntry(pEntry->GetSymbolic());
                triggerSelector->FromString(triggerNameOfEntry);
                if (triggerName == triggerNameOfEntry) {
                    // Activate software trigger
                    triggerMode->FromString("On");
                    CEnumerationPtr(nodemap.GetNode("TriggerSource"))
                        ->FromString("Software");
                } else {
                    triggerMode->FromString("Off");
                }
            }
        }

        // Select the frame trigger type
        triggerSelector->FromString(triggerName);

        // Set acquisition mode to "continuous"
        auto node = nodemap.GetNode("AcquisitionMode");
        CEnumerationPtr acquisitionMode(node);

        if (IsWritable(acquisitionMode) &&
            IsAvailable(acquisitionMode->GetEntryByName("Continuous"))) {
            acquisitionMode->FromString("Continuous");
        }
    }

    virtual void OnOpened(Pylon::CInstantCamera& camera) {
        try {
            ApplyConfiguration(camera.GetNodeMap());
        } catch (const Pylon::GenericException& e) {
            throw RUNTIME_EXCEPTION(
                "Could not apply software trigger configuration. "
                "Pylon::GenericException caught in OnOpened method msg=%hs",
                e.what());
        } catch (const std::exception& e) {
            throw RUNTIME_EXCEPTION(
                "Could not apply software trigger configuration. "
                "std::exception caught in OnOpened method msg=%hs",
                e.what());
        } catch (...) {
            throw RUNTIME_EXCEPTION(
                "Could not apply software trigger configuration. Unknown "
                "exception caught in OnOpened method.");
        }
    }
};
}  // namespace ncs