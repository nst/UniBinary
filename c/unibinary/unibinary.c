//
//  main.c
//  unibinary
//
//  Created by Nicolas Seriot on 24/12/13.
//  Copyright (c) 2013 Nicolas Seriot. All rights reserved.
//

#include "unibinary.h"
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>

// encodes ascii 7-bits characters
wchar_t U12a_0_0_start = 0x5E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 0,0
wchar_t U12a_0_1_start = 0x6E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 0,1
wchar_t U12a_1_0_start = 0x7E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 1,0
wchar_t U12a_1_1_start = 0x8E00; // CJK Unified Ideographs (subset) - encodes 12 bits (2 ascii) - MSB 1,1
wchar_t U12a_length = 0x1000;

// encodes arbitrary bits
wchar_t U12b_start = 0x4E00; // CJK Unified Ideographs (subset) - encodes 12 bits
wchar_t U12b_length = 0x1000;
wchar_t U8_start = 0x0400;   // Cyrillic                        - encodes 8 bits
wchar_t U8_length = 0x0100;

int is_in_U08b(wchar_t i) {
    return i >= U8_start && i < (U8_start + U8_length);
}

int is_in_U12a(wchar_t u) {
    
    wchar_t starts[4] = {U12a_0_0_start, U12a_0_1_start, U12a_1_0_start, U12a_1_1_start};
    
    for (int i = 0; i < 4; i++) {
        wchar_t start = starts[i];
        if(u >= start && u < (start + U12a_length)) {
            return 1;
        }
    }
    
    return EXIT_SUCCESS;
}

int is_in_U12b(wchar_t u) {
    return u >= U12b_start && u < (U12b_start + U12b_length);
}

int two_unichr_to_repeat_byte_ntimes(unsigned char c, int n, wchar_t *u0, wchar_t *u1) {
    
    if (c > 0xFF) return EXIT_FAILURE;
    if (n > 0xFFF) return EXIT_FAILURE;
    
    *u0 = U8_start + c;
    *u1 = U12b_start + n;
    
    return EXIT_SUCCESS;
}

int two_twelve_bits_values_from_three_bytes(unsigned char c0, unsigned char c1, unsigned char c2, wchar_t *u0, wchar_t *u1) {
    
    // (0x12, 0x34, 0x56) -> (0x123, 0x456)
    
    if (c0 > 0xFF || c1 > 0xFF || c2 > 0xFF) return EXIT_FAILURE;
    
    *u0 = (c0 << 4) + (c1 >> 4);
    *u1 = ((c1 & 0xF) << 8) + c2;
    
    return EXIT_SUCCESS;
}

int unichr_12a_from_two_ascii(unsigned char c0, unsigned char c1, wchar_t *u0) {
    
    int unicode_start = 0;
    
    if (c0 < 64 && c1 < 64) {
        unicode_start = U12a_0_0_start;
    } else if (c0 < 64 && c1 >= 64) {
        c1 -= 64;
        unicode_start = U12a_0_1_start;
    } else if (c0 >= 64 & c1 < 64) {
        c0 -= 64;
        unicode_start = U12a_1_0_start;
    } else if (c0 >= 64 & c1 >= 64) {
        c0 -= 64;
        c1 -= 64;
        unicode_start = U12a_1_1_start;
    }
    
    *u0 = unicode_start + (c0 << 6) + c1;
    
    return EXIT_SUCCESS;
}

int number_of_repeated_characters_at_index(const char* src, size_t i, size_t srcSize, int *n) {
    
    int repeats_count = 0;
    
    unsigned char c = src[i];
    
    while(i < srcSize && (c == (unsigned char)src[i])) {
        repeats_count += 1;
        i++;
    }
    
    *n = repeats_count;
    
    return EXIT_SUCCESS;
}

int to_U08(uint8_t i, wchar_t *o) {
    
    if (i > (U8_start + U8_length)) return EXIT_FAILURE;
    
    *o = U8_start + i;
    
    return EXIT_SUCCESS;
}

int to_U12(wchar_t i, wchar_t *o) {
    
    if(i > (U12b_start + U12b_length)) return EXIT_FAILURE;
    
    *o = U12b_start + i;
    
    return EXIT_SUCCESS;
}

int from_U12b(wchar_t i, wchar_t *o) {
    
    if(i < U12b_start || i > (U12b_start + U12b_length)) return EXIT_FAILURE;
    
    *o = i - U12b_start;
    
    return EXIT_SUCCESS;
}

int U12a_to_8_8(wchar_t u, uint8_t *b0, uint8_t *b1) {
    
    wchar_t unicode_start = 0;
    wchar_t starts[4] = {U12a_0_0_start, U12a_0_1_start, U12a_1_0_start, U12a_1_1_start};
    
    for (int i = 0; i < 4; i++) {
        wchar_t start = starts[i];
        if(u >= start && u < (start + U12a_length)) {
            unicode_start = start;
            break;
        }
    }
    
    if(unicode_start == 0) return EXIT_FAILURE;
    
    wchar_t value = u - unicode_start;
    *b0 = (value & 0xFC0) >> 6;
    *b1 = u & 0x3F;
    
    if(unicode_start == U12a_0_0_start) {
        // pass
    } else if (unicode_start == U12a_0_1_start) {
        *b1 += 64;
    } else if (unicode_start == U12a_1_0_start) {
        *b0 += 64;
    } else if (unicode_start == U12a_1_1_start) {
        *b0 += 64;
        *b1 += 64;
    }
    
    return EXIT_SUCCESS;
}

int int_from_u08b(wchar_t u, uint8_t* i) {
    if(u < U8_start || u > (U8_start + U8_length)) return EXIT_FAILURE;
    
    *i = u - U8_start;
    
    return EXIT_SUCCESS;
}

int int_from_u12b(wchar_t u, wchar_t* i) {
    if(u < U12b_start || u > (U12b_start + U12b_length)) return EXIT_FAILURE;
    
    *i = u - U12b_start;
    
    return EXIT_SUCCESS;
}

int repeated_bytes_from_unichars(wchar_t u1, wchar_t u2, uint8_t **dst, size_t *dstSize) {
    
    uint8_t b;
    if(int_from_u08b(u1, &b) != 0) return EXIT_FAILURE;
    
    wchar_t n;
    if(int_from_u12b(u2, &n) != 0) return EXIT_FAILURE;
    
    if(n > 0xFFF) {
        fprintf(stderr, "-- bad number of repeats: 0x%x\n", n);
        return EXIT_FAILURE;
    }
    
    uint8_t *out = malloc(n * sizeof(uint8_t));
    if(out == NULL) {
        return EXIT_FAILURE;
    }
    
    memset(out, b, n);
    
    *dstSize = n;
    *dst = out;
    
    return EXIT_SUCCESS;
}

int three_bytes_from_two_twelve_bits_values(wchar_t i1, wchar_t i2, uint8_t *b1, uint8_t *b2, uint8_t *b3) {
    // (0x123, 0x456) -> (0x12, 0x34, 0x56)
    
    if(i1 > 0xFFF) return EXIT_FAILURE;
    if(i2 > 0xFFF) return EXIT_FAILURE;
    
    *b1 = i1 >> 4;
    *b2 = ((i1 & 0xF) << 4) + ((i2 & 0xF00) >> 8);
    *b3 = (i2 & 0x0FF);
    
    return EXIT_SUCCESS;
}

int three_bytes_from_unichars(wchar_t u1, wchar_t u2, uint8_t *b1, uint8_t *b2, uint8_t *b3) {
    
    wchar_t i1, i2;
    
    if(int_from_u12b(u1, &i1) != 0) return EXIT_FAILURE;
    if(int_from_u12b(u2, &i2) != 0) return EXIT_FAILURE;
    
    int error = three_bytes_from_two_twelve_bits_values(i1, i2, b1, b2, b3);
    if(error != 0) return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}

int two_bytes_from_unichars(wchar_t u1, wchar_t u2, uint8_t *b1, uint8_t *b2) {
    
    int status = int_from_u08b(u1, b1);
    if(status == EXIT_FAILURE) return EXIT_FAILURE;

    int status2 = int_from_u08b(u2, b2);
    if(status2 == EXIT_FAILURE) return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}

int bytes_from_u1_u2(wchar_t u1, wchar_t u2, uint8_t **buffer, size_t *bufferSize) {
    
    int u1_in_U12b = is_in_U12b(u1);
    int u2_in_U12b = is_in_U12b(u2);
    
    int u1_in_U8b = is_in_U08b(u1);
    int u2_in_U8b = is_in_U08b(u2);
    
    if(u1_in_U12b && u2_in_U12b) {
        
        *buffer = (uint8_t *)malloc(3 * sizeof(uint8_t));
        if(buffer == NULL) {
            fprintf(stderr, "-- malloc error\n");
            return EXIT_FAILURE;
        }
        
        *bufferSize = 3;
        
        uint8_t b0, b1, b2;
        
        int status = three_bytes_from_unichars(u1, u2, &b0, &b1, &b2);
        if(status != 0) return EXIT_FAILURE;
        
        *(*buffer+0) = b0;
        *(*buffer+1) = b1;
        *(*buffer+2) = b2;
        
        return EXIT_SUCCESS;
        
    } else if (u1_in_U8b && u2_in_U12b) {
        
        size_t dstSize;
        
        uint8_t *out;
        
        int status = repeated_bytes_from_unichars(u1, u2, &out, &dstSize);
        if(status != 0) return EXIT_FAILURE;
        
        *bufferSize = dstSize;
        
        *buffer = out;
        
        return EXIT_SUCCESS;
    } else if (u1_in_U8b && u2_in_U8b) {
        
        uint8_t b0, b1;
        int status = two_bytes_from_unichars(u1, u2, &b0, &b1);
        if(status != 0) return EXIT_FAILURE;
        
        uint8_t *out = malloc(2 * sizeof(uint8_t));
        if(out == NULL) {
            fprintf(stderr, "-- malloc error\n");
            return EXIT_FAILURE;
        }
        
        *bufferSize = 2;
        
        out[0] = b0;
        out[1] = b1;
        
        *buffer = out;
        
        return EXIT_SUCCESS;
    } else if (u1_in_U8b && u2 == '\n') {
        
        *buffer = (uint8_t *)malloc(1 * sizeof(uint8_t));
        if(buffer == NULL) {
            fprintf(stderr, "-- malloc error\n");
            return EXIT_FAILURE;
        }
        
        *bufferSize = 1;
        
        uint8_t b0;
        
        int success = int_from_u08b(u1, &b0);
        if(success == EXIT_FAILURE) {
            fprintf(stderr, "-- error\n");
            return EXIT_FAILURE;
        }
        
        *buffer[0] = b0;
        return EXIT_SUCCESS;
    }
    
    fprintf(stderr, "-- bytes_from_u1_u2() cannot deal with u1:0x%x u2:0x%x\n", u1, u2);
    fprintf(stderr, "   u1 in U8b:%d U12b:%d, u2 in U8b:%d U12b:%d\n", u1_in_U8b, u1_in_U12b, u2_in_U8b, u2_in_U12b);
    return EXIT_FAILURE;
}

int next_non_newline_char(FILE *src, wchar_t *wc) {
    do {
        *wc = fgetwc(src);
    } while (*wc == '\n');
    return EXIT_SUCCESS;
}

int unibinary_decode(FILE *src, FILE *dst) {
    
    int i = 0;
    
    wchar_t c, c_next;
    
    int fwd_chars_read = 0;
    
    while (1) {
        
        if(fwd_chars_read == 0) {
            next_non_newline_char(src, &c);
            next_non_newline_char(src, &c_next);
            fwd_chars_read = 2;
        } else if (fwd_chars_read == 1) {
            c = c_next;
            next_non_newline_char(src, &c_next);
            fwd_chars_read = 2;
        } else {
            // pass
        }
        
        if(is_in_U12a(c)) {
            uint8_t b0;
            uint8_t b1;
            
            int error = U12a_to_8_8(c, &b0, &b1);
            if(error != 0) {
                fprintf(stderr, "-- error in U12a_to_8_8()\n");
                return EXIT_FAILURE;
            }
            
            fwrite(&b0, 1, 1, dst);
            fwrite(&b1, 1, 1, dst);
            
            i += 1;
            
            fwd_chars_read -= 1;
            
        } else if (c != WEOF && c_next != WEOF) {
            wchar_t u1 = c;
            wchar_t u2 = c_next;
            
            uint8_t *outBuffer;
            size_t outBufferSize;
            
            int error = bytes_from_u1_u2(u1, u2, &outBuffer, &outBufferSize);
            if(error != 0) {
                fprintf(stderr, "-- error in bytes_from_u1_u2()\n");
                return EXIT_FAILURE;
            }
            
            if(outBuffer == NULL) {
                fprintf(stderr, "-- outBuffer == NULL\n");
            }
            
            fwrite(outBuffer, 1, outBufferSize, dst);
            
            i += 2;
            
            fwd_chars_read -= 2;
            
        } else if (is_in_U08b(c)) {
            uint8_t b0;
            int status = int_from_u08b(c, &b0);
            if(status != 0) {
                fprintf(stderr, "-- error in int_from_u08b()\n");
                return EXIT_FAILURE;
            }
            
            fwrite(&b0, 1, 1, dst);
            
            i += 1;
            
            fwd_chars_read -= 1;
            
        } else if (c == '\n') {
            i += 1;
            
            if(fwd_chars_read > 0) {
                fwd_chars_read -= 1;
            }
        } else if (c == WEOF) {
            break;
        } else {
            fprintf(stderr, "-- cannot decode character at index %u\n", i);
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

int put_wc(FILE *fd_out, wchar_t wc, size_t *count, size_t wrap_length) {
    if(fputwc(wc, fd_out) == EOF) return EXIT_FAILURE;

    *count += 1;
    if(wrap_length > 0) {
        *count = *count % wrap_length;
    }
    
    if(*count == 0) {
        if(fputwc('\n', fd_out) == EOF) return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int unibinary_encode_string(const char *src, wchar_t **dst, size_t wrap_length) {
    
    // 1. write src into a temporary file
    
    FILE *fd_in = tmpfile();
    if(fd_in == NULL) return EXIT_FAILURE;
    
    size_t src_len = strlen(src);
    size_t written = fwrite(src, 1, src_len, fd_in);
    if(written != src_len) {
        fclose(fd_in);
        return EXIT_FAILURE;
    }
    rewind(fd_in);
    
    // 2. open another temporary file to write the encoded string
    
    FILE *fd_out = tmpfile();
    if(fd_in == NULL) {
        fclose(fd_in);
        return EXIT_FAILURE;
    }
    
    int status = unibinary_encode(fd_in, fd_out, wrap_length);
    fclose(fd_in);
    
    if(status != 0) return EXIT_FAILURE;
    
    // 3. read the encoded string and fill *dst
    
    long file_size = ftell(fd_out);
    
    rewind(fd_out);
    
    long max_wchar_bytes_possible = file_size * MB_CUR_MAX;
    
    if(max_wchar_bytes_possible > INTMAX_MAX) {
        fclose(fd_out);
        return EXIT_FAILURE;
    }
    
    char *map = mmap(0, file_size, PROT_READ, MAP_SHARED, fileno(fd_out), 0);

    fclose(fd_out);

    if(map == NULL) {
        return EXIT_FAILURE;
    }
    
    *dst = (wchar_t *)malloc(file_size * MB_CUR_MAX);
    if(dst == NULL) {
        fprintf(stderr, "-- malloc error\n");
        return EXIT_FAILURE;
    }
    
    size_t length = mbstowcs(*dst, map, file_size * MB_CUR_MAX);

    if(length == -1) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int unibinary_decode_string(const wchar_t *src, char **dst, long *dst_len) {
    
    // 1. write src into a temporary file
    
    FILE *fd_in = tmpfile();
    if(fd_in == NULL) return EXIT_FAILURE;
    
    int status = fputws(src, fd_in);
    if(status != 0) {
        fclose(fd_in);
        return EXIT_FAILURE;
    }
    
    rewind(fd_in);
    
    // 2. open another temporary file to write decoded data
    
    FILE *fd_out = tmpfile();
    if(fd_in == NULL) {
        fclose(fd_in);
        return EXIT_FAILURE;
    }
    
    int status2 = unibinary_decode(fd_in, fd_out);
    fclose(fd_in);
    
    if(status2 != 0) return EXIT_FAILURE;
    
    // 3. read the resulting string and fill **dst
    
    long file_size = ftell(fd_out);
    
    *dst_len = file_size;
    
    rewind(fd_out);
    
    *dst = (char *)malloc(file_size * sizeof(char));
    if(dst == NULL) {
        fprintf(stderr, "-- malloc error\n");
        return EXIT_FAILURE;
    }
    
    size_t read = fread(*dst, sizeof(char), file_size, fd_out);
    fclose(fd_out);

    if(read != file_size) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int unibinary_encode(FILE *fd_in, FILE *fd_out, size_t wrap_length) {
    
    size_t out_count = 0;
    
    while(1) {
        
        unsigned char c0, c1, c2;
        size_t read_c0 = fread(&c0, 1, 1, fd_in);
        if(read_c0 == 0) break;
        
        size_t read_c1 = 0;
        
        long number_of_repeats = 1;
        unsigned char cc;
        size_t read_cc = fread(&cc, 1, 1, fd_in);
        
        while(read_cc != 0 && cc == c0 && number_of_repeats < 0xFFF) {
            number_of_repeats += 1;
            read_cc = fread(&cc, 1, 1, fd_in);
        }
        
        /**/
        
        if(number_of_repeats >= 3) {
            if(read_cc != 0) {
                if(ungetc(cc, fd_in) == EOF) return EXIT_FAILURE;
            }

            long n = number_of_repeats;
            
            wchar_t u0, u1;
            int error = two_unichr_to_repeat_byte_ntimes(c0, (int)n, &u0, &u1);
            if(error) return EXIT_FAILURE;
            
            put_wc(fd_out, u0, &out_count, wrap_length);
            put_wc(fd_out, u1, &out_count, wrap_length);
            
            continue;
        } else if (number_of_repeats == 2) {
            if(read_cc != 0) {
                if(ungetc(cc, fd_in) == EOF) return EXIT_FAILURE;
            }

            read_c0 = 1;
            read_c1 = 1;
//            c0 = c0;
            c1 = c0;

        } else if (read_cc != 0) {
            read_c1 = 1;
            c1 = cc;
        }
        
        /**/
        
        size_t read_c2 = fread(&c2, 1, 1, fd_in);
        
        int two_ASCII_7bits_chars_available = read_c0 != 0 && read_c1 != 0 && c0 < 128 && c1 < 128;
        int three_bytes_available = read_c0 != 0 && read_c1 != 0 && read_c2 != 0;
        
        if(two_ASCII_7bits_chars_available) {
            
            // put 2 x 7 bits into a unichar
            wchar_t u0;
            int error = unichr_12a_from_two_ascii(c0, c1, &u0);
            if(error) return EXIT_FAILURE;
            
            put_wc(fd_out, u0, &out_count, wrap_length);
            
            if(read_c2 != 0) {
                if(ungetc(c2, fd_in) == EOF) return EXIT_FAILURE;
            }
            
        } else if (three_bytes_available) {
            // read 3 bytes, yield 2 unichars
            wchar_t u0, u1;
            int error = two_twelve_bits_values_from_three_bytes(c0, c1, c2, &u0, &u1);
            if(error) return EXIT_FAILURE;
            
            wchar_t o0, o1;
            error = to_U12(u0, &o0);
            if(error != 0) return EXIT_FAILURE;
            error = to_U12(u1, &o1);
            if(error != 0) return EXIT_FAILURE;
            
            put_wc(fd_out, o0, &out_count, wrap_length);
            put_wc(fd_out, o1, &out_count, wrap_length);
            
        } else if (read_c0 != 0) {
            // read 1 byte, encode 1 unichar
            wchar_t u0;
            int error = to_U08(c0, &u0);
            if(error) return EXIT_FAILURE;
            
            put_wc(fd_out, u0, &out_count, wrap_length);
            
            if(read_c1 != 0) {
                if(ungetc(c1, fd_in) == EOF) return EXIT_FAILURE;
            }
            
        } else {
            break;
        }
        
    }
    
    return EXIT_SUCCESS;
}
