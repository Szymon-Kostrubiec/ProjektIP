#include <iso646.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "../attach_to_shared.h"
#include "../shared_memory.h"

extern const char *shm_name;
extern const char *sem_init_name; // potentially unused
extern const char *sem_mem_name;

volatile bool time_to_quit;
void sigint_handler(__attribute_maybe_unused__ int signal) {
  time_to_quit = true;
}

int main() {
  printf("Program: krancowki\n");
  SharedMemory_t *shared_memory = attach_to_shared_memory();
  sem_t *sem_mem = get_semaphore();

  if (not shared_memory) {
    exit(EXIT_FAILURE);
  }
  if (not sem_mem) {
    exit(EXIT_FAILURE);
  }

  if (signal(SIGINT, &sigint_handler) == SIG_ERR) {
    perror("Signal ");
    return EXIT_FAILURE;
  }
  while (true) {
    if (time_to_quit) {
      break;
    }

    sem_wait(sem_mem);

    shared_memory->krancowka_max = (shared_memory->polozenie_szyby > 99.9);
    shared_memory->krancowka_min = (shared_memory->polozenie_szyby < 0.1);

    if (shared_memory->krancowka_max) {
      printf("Endstop max active\n");
    } else if (shared_memory->krancowka_min) {
      printf("Endstop min active\n");
    }

    sem_post(sem_mem);
  }
}
