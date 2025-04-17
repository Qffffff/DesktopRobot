#pragma once

#include <stddef.h>


int url_encode(const unsigned char *src, size_t slen, size_t *olen, unsigned char *dst, size_t dlen);