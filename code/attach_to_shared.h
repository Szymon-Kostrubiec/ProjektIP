#pragma once

#include "semaphore.h"

// function to attach to to shared memory from 'slave' processes
#include "shared_memory.h"

SharedMemory_t *attach_to_shared_memory();

sem_t *get_semaphore();
