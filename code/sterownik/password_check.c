#include "password_check.h"

#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "hash.h"

const char *const password_file_name = "./password";

bool check_password() {
  FILE *file = fopen(password_file_name, "r");
  if (file == NULL) {
    fprintf(stderr, "No password file. Login failed.\n");
    return false;
  }

  printf("Please type the password: ");
  char *password_attempt = malloc(MAX_PASSWORD_SIZE * sizeof(char));
  char *success = fgets(password_attempt, MAX_PASSWORD_SIZE, stdin);

  if (success == NULL) {
    fprintf(stderr, "No password provided. Login failed.\n");
    fclose(file);
    free(password_attempt);
    return false;
  }

  char *salt = malloc(SALT_SIZE * sizeof(char));
  char *hashed_password = malloc(MAX_HASH_SIZE * sizeof(char));

  fgets(salt, SALT_SIZE, file);
  fgets(hashed_password, MAX_HASH_SIZE, file);
  fclose(file);

  strip_newline(salt);
  strip_newline(hashed_password);
  strip_newline(password_attempt);

  char *password_attempt_hashed = malloc(MAX_HASH_SIZE * sizeof(char));
  hash(password_attempt, salt, password_attempt_hashed, MAX_HASH_SIZE);

  bool result;

  if (strncmp(password_attempt_hashed, hashed_password, MAX_HASH_SIZE)) {
    result = false;
  } else {
    result = true;
  }

  free(password_attempt);
  free(salt);
  return result;
}

bool create_password_file() {
  char *password_attempt_1 = malloc(MAX_PASSWORD_SIZE * sizeof(char));
  char *password_attempt_2 = malloc(MAX_PASSWORD_SIZE * sizeof(char));

  if (password_attempt_1 == NULL or password_attempt_2 == NULL) {
    perror("malloc: ");
  }

  // stdin is FILE *
  // STDIN_FILENO is int - fd

  struct termios old, new;
  // potentially handle termios functions failing
  tcgetattr(STDIN_FILENO, &old);
  new = old;
  new.c_lflag &= ~(ECHO);

  printf("Please type the password: ");
  // hide the password
  tcsetattr(STDIN_FILENO, TCSANOW, &new);
  fgets(password_attempt_1, MAX_PASSWORD_SIZE, stdin);
  printf("\n");
  printf("Please retype the password: ");
  fgets(password_attempt_2, MAX_PASSWORD_SIZE, stdin);
  printf("\n");

  // restore the terminal settings - best practice
  tcsetattr(STDIN_FILENO, TCSANOW, &old);

  if (strncmp(password_attempt_1, password_attempt_2, MAX_PASSWORD_SIZE)) {
    fprintf(stderr,
            "Passwords provided were not the same. No file will be created.\n");
    free(password_attempt_1);
    free(password_attempt_2);
    return false;
  }

  char *hashed = malloc(MAX_HASH_SIZE * sizeof(char));
  char *random_salt = malloc((SALT_SIZE + 3) * sizeof(char));

  gen_random_salt(random_salt + 3, SALT_SIZE);

  random_salt[0] = '$';
  random_salt[1] = '6';
  random_salt[2] = '$';
  strip_newline(password_attempt_1);
  hash(password_attempt_1, random_salt, hashed, MAX_HASH_SIZE);

  FILE *file = fopen(password_file_name, "w");

  if (file == NULL) {
    perror("fopen: ");
    return false;
  }

  fprintf(file, "%s\n%s\n", random_salt, hashed);

  fclose(file);
  free(random_salt);
  free(hashed);
  free(password_attempt_1);
  free(password_attempt_2);

  return true;
}
