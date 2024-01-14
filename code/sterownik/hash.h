#pragma once

#include <stddef.h>

// constants for sha-512
#define MAX_PASSWORD_SIZE (128) // own limit
#define MAX_HASH_SIZE (512 / 8) // bytes
#define SALT_SIZE (3 * 96 / 8)  // bytes. There's parameters to account for

bool hash(const char *password, const char *salt, char *hash, size_t hash_size);

bool gen_random_salt(char *salt, size_t buffer_size);

void strip_newline(char *string);
