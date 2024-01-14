#pragma once

// this is an overlay of the shared memory

#include <sys/types.h>

struct SharedMemory {
  pid_t sterownik_pid; // so that it can be signaled
  bool krancowka_max;
  bool krancowka_min;
  double polozenie_szyby; // [0-100]
  double silnik;          // [-100 - 100]
  bool przycisk_dol;
  bool przycisk_gora;
  bool autozamykanie;
  bool autootwieranie;
};

typedef struct SharedMemory SharedMemory_t;
