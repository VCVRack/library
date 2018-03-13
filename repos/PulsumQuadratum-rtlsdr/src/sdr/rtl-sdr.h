#pragma once

#include <rtl-sdr.h>
#include <pthread.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

struct RtlSdr {
  pthread_mutex_t* rack_mutex;
  int16_t* rack_buffer;
  long* rack_buffer_pos;
};
int RtlSdr_init(struct RtlSdr* radio, int engineSampleRate);
void RtlSdr_end(struct RtlSdr* radio);
void RtlSdr_tune(struct RtlSdr* radio, long freq);

#ifdef __cplusplus
}
#endif
