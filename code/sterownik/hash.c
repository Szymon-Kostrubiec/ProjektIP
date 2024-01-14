#include "hash.h"

#include <assert.h>
#include <crypt.h>

#include <iso646.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPU_COST 1500

static void generate_secure_random_bytes(char *, size_t);

void strip_newline(char *string);

bool hash(const char *password, const char *salt, char *hash,
          size_t hash_size) {
  const size_t sha_512_hash_size = 512 / 8;
  if (hash_size < sha_512_hash_size) {
    hash[0] = '\0';
    return false;
  }
  const char *result = crypt(password, salt);
  if (result not_eq NULL) {
    strncpy(hash, result, hash_size);
    hash[hash_size - 1] = '\0'; // Ensure null termination
  } else {
    perror("crypt returned null");
    hash[0] = '\0';
    return false;
  }
  return true;
}

bool gen_random_salt(char *salt, size_t buffer_size) {
  char *secure_random_bytes = malloc(SALT_SIZE * sizeof(char));
  generate_secure_random_bytes(secure_random_bytes, SALT_SIZE);
  const char *salt_temp_buff =
      crypt_gensalt("$6$", CPU_COST, secure_random_bytes, SALT_SIZE);
  if (salt_temp_buff not_eq 0) {
    strncpy(salt, salt_temp_buff, buffer_size);
  } else {
    salt[0] = '\0';
    return false;
  }
  free(secure_random_bytes);
  return true;
}

// if this function throws the program must terminate because of the
// cryptographic failure
static void generate_secure_random_bytes(char *buffer, size_t length) {
  // binary data -- special function?
  FILE *file = fopen("/dev/urandom", "rb");
  if (file == NULL) {
    perror("Failed to open /dev/urandom");
    exit(EXIT_FAILURE);
  }

  if (fread(buffer, sizeof(unsigned char), length, file) != length) {
    perror("Failed to read random data");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fclose(file);
}

// very problem-specific function to strip newline from salt after it's read
// from file
void strip_newline(char *string) {
  assert(string not_eq NULL);

  for (size_t i = 0; string[i] not_eq '\0'; ++i) {
    if (string[i] == '\n') {
      string[i] = '\0';
      break;
    }
  }
}
