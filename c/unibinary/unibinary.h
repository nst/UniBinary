//
//  unibinary.h
//  unibinary
//
//  Created by Nicolas Seriot on 29/12/13.
//  Copyright (c) 2013 Nicolas Seriot. All rights reserved.
//

#include <stdint.h>
#include <wchar.h>

#ifndef unibinary_unibinary_h
#define unibinary_unibinary_h

// encode

int unibinary_encode(FILE *fd_in, FILE *fd_out, size_t wrap_length);
int unibinary_encode_string(const char* src, wchar_t **dst, size_t wrap_length);

// decode

int unibinary_decode(FILE *src, FILE *dst);
int unibinary_decode_string(const wchar_t *src, char **dst, long *dst_len);

#endif
