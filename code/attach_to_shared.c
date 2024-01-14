#include "attach_to_shared.h"

#include <semaphore.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>

#include <iso646.h>

extern const char *shm_name;
extern const char *sem_init_name;
extern const char *sem_mem_name;

int shm_descriptor = -1;
int shm_size = -1;
void *shm_ptr = (void *)-1;

static void cleanup();

SharedMemory_t *attach_to_shared_memory() {

  if (atexit(&cleanup)) {
    perror("Attach to shared memory: atexit");
    return NULL;
  }

  shm_descriptor = shm_open(shm_name, O_RDWR, 0);

  if (shm_descriptor == -1) {
    perror("Attach to shared memory: shm_open:");
    return NULL;
  }

  shm_size = sysconf(_SC_PAGESIZE);

  shm_ptr =
      mmap(0, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);

  if (shm_ptr == (void *)-1) {
    perror("Attach to shared memory: mmap");
    return NULL;
  }

  return (SharedMemory_t *)shm_ptr;
}

void cleanup() {
  if (shm_ptr not_eq (void *) - 1 and shm_ptr not_eq NULL) {
    munmap(shm_ptr, shm_size);
  }

  if (shm_descriptor not_eq -1) {
    close(shm_descriptor);
  }
}

sem_t *sem_mem;

void cleanup_semaphore() {
  if (sem_mem != SEM_FAILED) {
    sem_close(sem_mem);
  }
}

sem_t *get_semaphore() {
  sem_t *sem = sem_open(sem_mem_name, O_RDWR);

  if (sem == SEM_FAILED) {
    perror("Sem could not be opened: ");
    exit(EXIT_FAILURE);
  }
  sem_mem = sem;
  if (atexit(&cleanup_semaphore)) {
    perror("Could not attach to atexit: ");
    exit(EXIT_FAILURE);
  }
  return sem;
}
