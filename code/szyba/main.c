#include <ncurses.h>
#include <signal.h>
#include <stdlib.h>
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

int main() {
  printf("Program: szyba\n");

  SharedMemory_t *shared_memory = attach_to_shared_memory();
  sem_t *sem_mem = get_semaphore();

  if (signal(SIGINT, &sigint_handler) == SIG_ERR) {
    perror("Signal sigusr1");
    exit(EXIT_FAILURE);
  }

  initscr();

  while (true) {
    if (time_to_quit)
      break;

    sem_wait(sem_mem);
    if (shared_memory->polozenie_szyby < 0) {
      shared_memory->polozenie_szyby = 0;
    } else if (shared_memory->polozenie_szyby > 100) {
      shared_memory->polozenie_szyby = 100;
    }

    const int wysokosc_okna = (int)(shared_memory->polozenie_szyby / 2);
    sem_post(sem_mem);

    // creating a window
    WINDOW *win = newwin(wysokosc_okna, 50, 2, 2);
    refresh();

    // making box border with default border styles
    box(win, 0, 0);

    // move and print in window
    mvwprintw(win, 0, 1, "Szyba");

    // refreshing the window
    wrefresh(win);
    usleep(10000);
  }

  getch();
  endwin();
  return 0;
}
