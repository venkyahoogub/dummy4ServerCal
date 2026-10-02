#pragma once
#ifdef DEBUG
#include "neo-dmd-driver/src/iProjector.h"
namespace ncs {
class MockDmdProjector : public IProjector {
   public:
    explicit MockDmdProjector() {}
    void update_projection(Frame& frame) override {};
};
}  // namespace ncs
#endif
