#pragma once

#include "dmd_common.h"
#include "iProjector.h"

class Projector : public IProjector {
public:
  Projector();
  ~Projector();
  void update_projection(Frame &frame) override;

private:
  void configure_sequence_queue();

private:
  int32_t device_id_;
  int32_t sequence_0_id_{};
  int32_t sequence_1_id_{};
  int32_t current_running_sequence_id_;
  int32_t next_sequence_id_to_run_{};
};
