#include "../shared_memory.h"
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <termios.h>
#include <unistd.h>

#include "../attach_to_shared.h"
#include "../shared_memory.h"

struct termios original;

volatile bool time_to_quit;
void sigint_handler(__attribute_maybe_unused__ int signal) {
  time_to_quit = true;
}

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

  SharedMemory_t *shared_memory = attach_to_shared_memory();
  sem_t *sem_mem = get_semaphore();
  enable_raw_mode();

  printf("To quit, press q lub Ctrl-C\n");
  printf("Autoopen: w\nAutoclose: s\n Open: e\nClose:d\n");

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

    if (c == 'w' or c == 's' or c == 'e' or c == 'd') {
      printf("Pressed: %c", c);
      sem_wait(sem_mem);

      if (c == 'w') {
        shared_memory->autozamykanie = true;
      }
      if (c == 's') {
        shared_memory->autootwieranie = true;
      }
      shared_memory->przycisk_gora = (c == 'e');
      shared_memory->przycisk_dol = (c == 'd');

      sem_post(sem_mem);
    }
  }
  return EXIT_SUCCESS;
}
