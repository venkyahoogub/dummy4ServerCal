// Implements
#include "BaslerRegionOfInterest.h"

#include <algorithm>

namespace ncs {

BaslerRegionOfInterest::BaslerRegionOfInterest(
    Pylon::CInstantCamera& pylonDevice)
    : mPylonDevice(pylonDevice) {}

GenApi::INodeMap* BaslerRegionOfInterest::getNodeMap() {
    if (mPylonDevice.IsOpen()) {
        return &(mPylonDevice.GetNodeMap());
    }
    return nullptr;
}

// --- Width ---
std::int64_t BaslerRegionOfInterest::getWidth() {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return 0;

    GenApi::CIntegerPtr widthNode(nodemap->GetNode("Width"));
    if (GenApi::IsReadable(widthNode)) {
        return static_cast<std::int64_t>(widthNode->GetValue());
    }
    return 0;
}

void BaslerRegionOfInterest::setWidth(std::int64_t value) {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return;

    GenApi::CIntegerPtr widthNode(nodemap->GetNode("Width"));
    if (GenApi::IsWritable(widthNode)) {
        int64_t clampedValue = std::max(
            widthNode->GetMin(),
            std::min(static_cast<int64_t>(value), widthNode->GetMax()));

        int64_t inc = widthNode->GetInc();
        if (inc > 1) {
            clampedValue = (clampedValue / inc) * inc;
        }

        widthNode->SetValue(clampedValue);
    }
}

// --- Height ---
std::int64_t BaslerRegionOfInterest::getHeight() {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return 0;

    GenApi::CIntegerPtr heightNode(nodemap->GetNode("Height"));
    if (GenApi::IsReadable(heightNode)) {
        return static_cast<std::int64_t>(heightNode->GetValue());
    }
    return 0;
}

void BaslerRegionOfInterest::setHeight(std::int64_t value) {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return;

    GenApi::CIntegerPtr heightNode(nodemap->GetNode("Height"));
    if (GenApi::IsWritable(heightNode)) {
        int64_t clampedValue = std::max(
            heightNode->GetMin(),
            std::min(static_cast<int64_t>(value), heightNode->GetMax()));

        int64_t inc = heightNode->GetInc();
        if (inc > 1) {
            clampedValue = (clampedValue / inc) * inc;
        }

        heightNode->SetValue(clampedValue);
    }
}

// --- XOffset ---
std::int64_t BaslerRegionOfInterest::getXOffset() {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return 0;

    GenApi::CIntegerPtr offsetXNode(nodemap->GetNode("OffsetX"));
    if (GenApi::IsReadable(offsetXNode)) {
        return static_cast<std::int64_t>(offsetXNode->GetValue());
    }
    return 0;
}

void BaslerRegionOfInterest::setXOffset(std::int64_t value) {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return;

    GenApi::CIntegerPtr offsetXNode(nodemap->GetNode("OffsetX"));
    if (GenApi::IsWritable(offsetXNode)) {
        int64_t clampedValue = std::max(
            offsetXNode->GetMin(),
            std::min(static_cast<int64_t>(value), offsetXNode->GetMax()));

        int64_t inc = offsetXNode->GetInc();
        if (inc > 1) {
            clampedValue = (clampedValue / inc) * inc;
        }

        offsetXNode->SetValue(clampedValue);
    }
}

// --- YOffset ---
std::int64_t BaslerRegionOfInterest::getYOffset() {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return 0;

    GenApi::CIntegerPtr offsetYNode(nodemap->GetNode("OffsetY"));
    if (GenApi::IsReadable(offsetYNode)) {
        return static_cast<std::int64_t>(offsetYNode->GetValue());
    }
    return 0;
}

void BaslerRegionOfInterest::setYOffset(std::int64_t value) {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return;

    GenApi::CIntegerPtr offsetYNode(nodemap->GetNode("OffsetY"));
    if (GenApi::IsWritable(offsetYNode)) {
        int64_t clampedValue = std::max(
            offsetYNode->GetMin(),
            std::min(static_cast<int64_t>(value), offsetYNode->GetMax()));

        int64_t inc = offsetYNode->GetInc();
        if (inc > 1) {
            clampedValue = (clampedValue / inc) * inc;
        }

        offsetYNode->SetValue(clampedValue);
    }
}

bool BaslerRegionOfInterest::isReverseX() {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return false;

    GenApi::CBooleanPtr reverseXNode(nodemap->GetNode("ReverseX"));
    if (GenApi::IsReadable(reverseXNode)) {
        return reverseXNode->GetValue();
    }
    return false;
}

void BaslerRegionOfInterest::setReverseX(bool value) {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return;

    GenApi::CBooleanPtr reverseXNode(nodemap->GetNode("ReverseX"));
    if (GenApi::IsWritable(reverseXNode)) {
        reverseXNode->SetValue(value);
    }
}

bool BaslerRegionOfInterest::isReverseY() {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return false;

    GenApi::CBooleanPtr reverseYNode(nodemap->GetNode("ReverseY"));
    if (GenApi::IsReadable(reverseYNode)) {
        return reverseYNode->GetValue();
    }
    return false;
}

void BaslerRegionOfInterest::setReverseY(bool value) {
    GenApi::INodeMap* nodemap = getNodeMap();
    if (!nodemap) return;

    GenApi::CBooleanPtr reverseYNode(nodemap->GetNode("ReverseY"));
    if (GenApi::IsWritable(reverseYNode)) {
        reverseYNode->SetValue(value);
    }
}

}  // namespace ncs