#include <iso646.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "../attach_to_shared.h"
#include "../shared_memory.h"

extern const char *shm_name;
extern const char *sem_init_name;
extern const char *sem_mem_name;

volatile bool time_to_quit;
void sigint_handler(__attribute_maybe_unused__ int signal) {
  time_to_quit = true;
}

struct termios original;

void reset_terminal() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &original); }

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &original);
  atexit(&reset_terminal);

  struct termios raw;
  tcgetattr(STDIN_FILENO, &raw);
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, // change settings only after writing
                                     // everything, dont discard the input
            &raw);

  // ECHO - turn off echoing
  // ICANON - turn off canonical mode - read byte by byte
}

int main() {
  printf("Program: silnik\n");
  printf("To quit, press q lub Ctrl-C\n");
  printf("Press any button to make an object stuck in the way\n");
  SharedMemory_t *shared_memory = attach_to_shared_memory();
  sem_t *sem_mem = get_semaphore();

  if (not shared_memory) {
    exit(EXIT_FAILURE);
  }
  if (not sem_mem) {
    exit(EXIT_FAILURE);
  }

  if (signal(SIGINT, &sigint_handler) == SIG_ERR) {
    perror("Signal sigint");
    return EXIT_FAILURE;
  }

  enable_raw_mode();

  while (true) {

    char c = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};
    if (select(1, &fds, NULL, NULL, &timeout) == -1) {
      perror("select: ");
      return EXIT_FAILURE;
    }

    if (FD_ISSET(STDIN_FILENO, &fds)) {
      c = getchar();
    }

    if (time_to_quit or c == 'q') {
      break;
    }

    sem_wait(sem_mem);

    if (c) {
      printf("Sending signal\n");
      if (shared_memory->sterownik_pid not_eq 0) {
        int kill_status = kill(shared_memory->sterownik_pid, SIGUSR1);

        printf((kill_status) ? "Signal sent\n"
                             : "kill returned nonzero value\n");
      }
    }

    shared_memory->polozenie_szyby += shared_memory->silnik * 0.001;
    sem_post(sem_mem);
  }
  return EXIT_SUCCESS;
}
