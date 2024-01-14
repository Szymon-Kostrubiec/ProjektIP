#include <fcntl.h>
#include <sys/cdefs.h>
#include <sys/mman.h> // librt
#include <sys/stat.h>
#include <unistd.h>

#include <signal.h>

#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

// shared memory
#include "../shared_memory.h"
int shm_descriptor;
extern const char *shm_name;
extern const char *sem_init_name;
extern const char *sem_mem_name;
void *shm_ptr;
int shm_size;

// semaphore
#include <semaphore.h>
sem_t *sem_descriptor;
int dbg_semvalue;

// passwords
#include "password_check.h"

// endstop signal
void endstop_signal_handler([[maybe_unused]] int signal);
volatile bool collision;

volatile bool time_to_quit = false;
void sigint_handler(__attribute_maybe_unused__ int signal) {
  time_to_quit = true;
}

// cleanup
void cleanup();

int main(int argc, char *argv[]) {
  printf("Program: sterownik\n");

  if (argc > 1) {
    if (strcmp(argv[1], "create") == 0) {
      return create_password_file() ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    fprintf(stderr, "To create the password file, use ./sterownik create\n");
    return EXIT_SUCCESS;
  }

  if (atexit(&cleanup)) {
    perror("atexit error");
    _exit(EXIT_FAILURE);
  }

  // todo: turned off for debugging

  // check passwords
  // if (not check_password()) {
  //   return EXIT_FAILURE;
  // } else {
  //   printf("Authenticated. Welcome\n");
  // }

  // initialize shared memory

  if ((shm_descriptor = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL,
                                 S_IRWXU | S_IRWXG)) == -1) {
    // open read-write
    // create if not exists
    // throw error if created already
    //
    // inherit file permissions for owner from parent
    // inherit file permissions for group from parent
    perror("shm_open error");
    exit(EXIT_FAILURE);
  }

  shm_size = sysconf(_SC_PAGESIZE); // returns 4096 bytes

  // if (shm_size < (int)sizeof(SharedMemory_t)) {
  //   fprintf(stderr, "Shared memory is not big enough\n");
  //   return EXIT_FAILURE;
  // }
  if (ftruncate(shm_descriptor, shm_size) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }
  shm_ptr =
      mmap(0, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);

  if (shm_ptr == MAP_FAILED) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

  SharedMemory_t *shared_memory = (SharedMemory_t *)shm_ptr;

  // create the semaphore
  sem_descriptor =
      sem_open(sem_mem_name, O_CREAT, 0644, 0); // last arg is initial value

  if (sem_descriptor == (void *)-1) {
    perror("sem_open: ");
    exit(EXIT_FAILURE);
  }
  // initialize shared memory here
  shared_memory->sterownik_pid = getpid();
  shared_memory->polozenie_szyby = 50.0;

  // prepare for the signal
  if (signal(SIGUSR1, &endstop_signal_handler) == SIG_ERR) {
    perror("Signal sigusr1");
    exit(EXIT_FAILURE);
  }

  if (signal(SIGINT, &sigint_handler) == SIG_ERR) {
    perror("Signal sigint");
    exit(EXIT_FAILURE);
  }

  sem_post(sem_descriptor);

  while (true) {

    if (time_to_quit) {
      break;
    }

    sem_wait(sem_descriptor);
    if (collision) {
      collision = false;
      shared_memory->silnik = 0;

      printf("Object blocking the window has been acknowledged by the control "
             "unit\n");
    }

    // the user may force the motor to operate, no matter the state of the
    // window!

    printf("Szyba: %f", shared_memory->polozenie_szyby);
    // turn off autoopen/autoclose
    if (shared_memory->krancowka_min) {
      shared_memory->autootwieranie = false;
    }
    if (shared_memory->krancowka_max) {
      shared_memory->autozamykanie = false;
    }

    // if pressed to cancel
    if (shared_memory->autozamykanie and shared_memory->autootwieranie) {
      shared_memory->autozamykanie = false;
      shared_memory->autootwieranie = false;
    }

    if (not(shared_memory->autootwieranie or shared_memory->autozamykanie)) {
      shared_memory->silnik = false;
    }

    if (shared_memory->przycisk_gora) {
      shared_memory->silnik = 5;
    } else if (shared_memory->przycisk_dol) {
      shared_memory->silnik = -5;
    }

    if (shared_memory->autozamykanie) {
      shared_memory->silnik = 10;
    }

    if (shared_memory->autootwieranie) {
      shared_memory->silnik = -10;
    }

    sem_post(sem_descriptor);
  }

  return EXIT_SUCCESS;
}

void cleanup() {
  // close shared memory
  if (shm_descriptor not_eq 0) {
    close(shm_descriptor);
    if (shm_unlink(shm_name) == -1) {
      perror("shm_unlink");
    }
  }

  if (shm_ptr not_eq (void *) - 1 and shm_ptr not_eq NULL) {
    munmap(shm_ptr, shm_size);
  }

  // clear semaphore
  if (sem_descriptor not_eq NULL) {
    sem_close(sem_descriptor);
  }
}

void endstop_signal_handler([[maybe_unused]] int signal) { collision = true; }
