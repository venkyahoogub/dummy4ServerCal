#pragma once

#include "dmd_common.h"

class IProjector {
public:
  virtual ~IProjector() = default;
  virtual void update_projection(Frame &frame) = 0;
};