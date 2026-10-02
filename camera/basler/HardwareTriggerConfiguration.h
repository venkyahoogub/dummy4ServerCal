//-----------------------------------------------------------------------------
//  Basler pylon SDK
//  Copyright (c) 2010-2018 Basler AG
//  http://www.baslerweb.com
//  Author:  Andreas Gau
//-----------------------------------------------------------------------------
/*!
\file
\brief An instant camera configuration for software trigger,
       Use together with Pylon::CInstantCamera::WaitForFrameTriggerReady() and
Pylon::CInstantCamera::ExecuteSoftwareTrigger(). This  instant camera
configuration is provided as header-only file. The code can be copied and
modified for creating own configuration classes.
*/

#pragma once

#include "pylon/InstantCamera.h"
#include "pylon/Platform.h"

namespace ncs {
class HardwareTriggerConfiguration : public Pylon::CConfigurationEventHandler {
   public:
    /// Apply software trigger configuration.
    static void ApplyConfiguration(GENAPI_NAMESPACE::INodeMap& nodemap) {
        using namespace GENAPI_NAMESPACE;
        using namespace Pylon;

        // Disable all trigger types except the trigger type used for triggering
        // the acquisition of frames.
        {
            // Get required enumerations.
            CEnumerationPtr triggerSelector(nodemap.GetNode("TriggerSelector"));
            CEnumerationPtr triggerMode(nodemap.GetNode("TriggerMode"));

            // Check the available camera trigger mode(s) to select the
            // appropriate one: acquisition start trigger mode (used by older
            // cameras, i.e. for cameras supporting only the legacy image
            // acquisition control mode; do not confuse with acquisition start
            // command) or frame start trigger mode (used by newer cameras, i.e.
            // for cameras using the standard image acquisition control mode;
            // equivalent to the acquisition start trigger mode in the legacy
            // image acquisition control mode).
            String_t triggerName("FrameStart");
            if (!IsAvailable(triggerSelector->GetEntryByName(triggerName))) {
                triggerName = "AcquisitionStart";
                if (!IsAvailable(
                        triggerSelector->GetEntryByName(triggerName))) {
                    throw RUNTIME_EXCEPTION(
                        "Could not select trigger. Neither FrameStart nor "
                        "AcquisitionStart is available.");
                }
            }

            // Get all enumeration entries of trigger selector.
            GENAPI_NAMESPACE::NodeList_t triggerSelectorEntries;
            triggerSelector->GetEntries(triggerSelectorEntries);

            // Turn trigger mode off for all trigger selector entries except for
            // the frame trigger given by triggerName.
            for (GENAPI_NAMESPACE::NodeList_t::iterator it =
                     triggerSelectorEntries.begin();
                 it != triggerSelectorEntries.end(); ++it) {
                // Set trigger mode to off if the trigger is available.
                GENAPI_NAMESPACE::CEnumEntryPtr pEntry(*it);
                if (IsAvailable(pEntry)) {
                    String_t triggerNameOfEntry(pEntry->GetSymbolic());
                    triggerSelector->FromString(triggerNameOfEntry);
                    if (triggerName == triggerNameOfEntry) {
                        // Activate trigger.
                        triggerMode->FromString("On");

                        //// The trigger source must be set to 'Software'.
                        // CEnumerationPtr(nodemap.GetNode("TriggerSource"))->FromString("Software");

                        // Alternative hardware trigger configuration:
                        // This configuration can be copied and modified to
                        // create a hardware trigger configuration. Remove
                        // setting the 'TriggerSource' to 'Software' (see above)
                        // and use the commented lines as a starting point. The
                        // camera user's manual contains more information about
                        // available configurations. The Basler pylon Viewer
                        // tool can be used to test the selected settings first.

                        // The trigger source must be set to the trigger input,
                        // e.g. 'Line1'.
                        CEnumerationPtr(nodemap.GetNode("TriggerSource"))
                            ->FromString("Line3");

                        // The trigger activation must be set to e.g.
                        // 'RisingEdge'.
                        CEnumerationPtr(nodemap.GetNode("TriggerActivation"))
                            ->FromString("RisingEdge");
                    } else {
                        triggerMode->FromString("Off");
                    }
                }
            }
            // Finally select the frame trigger type (resp. acquisition start
            // type for older cameras). Issuing a software trigger will now
            // trigger the acquisition of a frame.
            triggerSelector->FromString(triggerName);
        }

        // Set acquisition mode to "continuous"
        auto node = nodemap.GetNode("AcquisitionMode");
        CEnumerationPtr acquisitionMode(node);

        if (IsWritable(acquisitionMode) &&
            IsAvailable(acquisitionMode->GetEntryByName("Continuous"))) {
            acquisitionMode->FromString("Continuous");
        }
    }

    // Set basic camera settings.
    // Pulled this from uvc, but cppcheck had issues. Better left untouched.
    // cppcheck-suppress unusedFunction
    virtual void OnOpened(Pylon::CInstantCamera& camera) {
        try {
            ApplyConfiguration(camera.GetNodeMap());
        } catch (const Pylon::GenericException& e) {
            throw RUNTIME_EXCEPTION(
                "Could not apply configuration. Pylon::GenericException caught "
                "in "
                "OnOpened method msg=%hs",
                e.what());
        } catch (const std::exception& e) {
            throw RUNTIME_EXCEPTION(
                "Could not apply configuration. std::exception caught in "
                "OnOpened "
                "method msg=%hs",
                e.what());
        } catch (...) {
            throw RUNTIME_EXCEPTION(
                "Could not apply configuration. Unknown exception caught in "
                "OnOpened "
                "method.");
        }
    }
};
}  // namespace ncs
