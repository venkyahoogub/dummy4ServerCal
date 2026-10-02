#include "projector.h"
#include "alp.h"

void flip_image_horizontally(Frame& frame, int width, int height) {
    uint8_t* data = frame.get();
    for (int y = 0; y < height; ++y) {
        uint8_t* row = data + y * width;
        for (int x = 0; x < width / 2; ++x) {
            std::swap(row[x], row[width - 1 - x]);
        }
    }
}

void invert_image(Frame& frame, int width, int height) {
  uint8_t* data = frame.get();
  for (int i = 0; i < (width * height); i++) {
    data[i] = data[i] ? 0x00 : 0xFF;
  }
}

uint8_t convert_to_binary_top_down(uint8_t *input_buffer) {
  return ((input_buffer[7] & 0x80) >> 7) | ((input_buffer[6] & 0x80) >> 6) |
         ((input_buffer[5] & 0x80) >> 5) | ((input_buffer[4] & 0x80) >> 4) |
         ((input_buffer[3] & 0x80) >> 3) | ((input_buffer[2] & 0x80) >> 2) |
         ((input_buffer[1] & 0x80) >> 1) | ((input_buffer[0] & 0x80));
}

void copy_image_to_output_buffer_binary_top_down(uint32_t width,
                                                 uint32_t height,
                                                 uint8_t *input_buffer,
                                                 uint8_t *output_buffer) {
  uint8_t *in = input_buffer;
  for (int i = 0; i < (width * height / 8); i++) {
    output_buffer[i] = convert_to_binary_top_down(in);
    in += 8;
  }
}

static inline ALP_ID to_alp_id(int32_t id) { return (ALP_ID) id;}
static inline ALP_ID *to_alp_id_ptr(int32_t *id) { return (ALP_ID*) id;}

Projector::Projector() {
  configure_sequence_queue();
}

void Projector::configure_sequence_queue() {
  int32_t illuminate_time = ALP_DEFAULT;
  int32_t picture_time_us = 100;
  int32_t synch_delay = ALP_DEFAULT;
  int32_t synch_pulse_width_us = (picture_time_us - 1);
  int32_t trigger_in_delay = ALP_DEFAULT;

  AlpDevAlloc(0, 0, to_alp_id_ptr(&device_id_));
  AlpProjControl(device_id_, ALP_PROJ_QUEUE_MODE,
                           ALP_PROJ_SEQUENCE_QUEUE);

  // allocated and configure two sequences containing one frame each
  for (int i = 0; i < 2; i++) {
    ALP_ID *sequence_id = (i == 0) ? to_alp_id_ptr(&sequence_0_id_) : to_alp_id_ptr(&sequence_1_id_);
    AlpSeqAlloc(device_id_, 1, 1, sequence_id);
    AlpSeqControl(device_id_, *sequence_id, ALP_BIN_MODE,
                            ALP_BIN_UNINTERRUPTED);
    AlpSeqControl(device_id_, *sequence_id, ALP_DATA_FORMAT,
                            ALP_DATA_BINARY_TOPDOWN);
    AlpSeqTiming(device_id_, *sequence_id, illuminate_time,
                           picture_time_us, synch_delay, synch_pulse_width_us,
                           trigger_in_delay);
  }
  current_running_sequence_id_ = sequence_1_id_;
}

void Projector::update_projection(Frame &frame) {
  // ping-pong between sequence 0 and sequence 1
  if (current_running_sequence_id_ == sequence_0_id_) {
    next_sequence_id_to_run_ = sequence_1_id_;
  } else {
    next_sequence_id_to_run_ = sequence_0_id_;
  }
  // the aurora optics flip the image along the y-axis (horizontal flip)
  flip_image_horizontally(frame, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  invert_image(frame, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  // convert from raw format to binary top down
  Frame frame_bin_topdown = std::make_unique<uint8_t[]>(FRAME_SIZE_BIN_TOPDOWN);
  copy_image_to_output_buffer_binary_top_down(
      DISPLAY_WIDTH, DISPLAY_HEIGHT, frame.get(), frame_bin_topdown.get());
  // queue up the next sequence
  AlpSeqPut(device_id_, next_sequence_id_to_run_, 0, 1,
                            frame_bin_topdown.get());
  AlpProjStartCont(device_id_, next_sequence_id_to_run_);
  /* abort projection at the end of the current frame display. this will
   * automatically terminate the currently running sequence and start projection
   * seamlessly with the next queued projection. */
  AlpProjControl(device_id_, ALP_PROJ_ABORT_FRAME, ALP_DEFAULT);
  current_running_sequence_id_ = next_sequence_id_to_run_;
}

Projector::~Projector() {
  AlpDevHalt(device_id_);
  AlpDevFree(device_id_);
}
