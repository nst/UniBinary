//
//  tests.c
//  unibinary
//
//  Created by Nicolas Seriot on 29/12/13.
//  Copyright (c) 2013 Nicolas Seriot. All rights reserved.
//

#include "unibinary.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
#include <stdio.h>

int number_of_repeated_characters_at_index(const char* src, size_t i, size_t srcSize, int *n);
int unichr_12a_from_two_ascii(unsigned char c0, unsigned char c1, wchar_t *u0);
int two_twelve_bits_values_from_three_bytes(uint8_t c0, uint8_t c1, uint8_t c2, wchar_t *u0, wchar_t *u1);
int two_unichr_to_repeat_byte_ntimes(unsigned char c, int n, wchar_t *u0, wchar_t *u1);
int to_U08(uint8_t i, wchar_t *o);
int to_U12(wchar_t i, wchar_t *o);
int from_U12b(wchar_t i, wchar_t *o);
int is_in_U08b(wchar_t i);
int is_in_U12a(wchar_t i);
int bytes_from_u1_u2(wchar_t u1, wchar_t u2, uint8_t **buffer, size_t *bufferSize);
int U12a_to_8_8(wchar_t u, uint8_t *b0, uint8_t *b1);
int two_bytes_from_unichars(wchar_t u1, wchar_t u2, uint8_t *b1, uint8_t *b2);
int three_bytes_from_unichars(wchar_t u1, wchar_t u2, uint8_t *b1, uint8_t *b2, uint8_t *b3);

int compareFiles(const char* filename1, const char* filename2) {
    // 0 if same contents
    
    FILE *f1 = fopen(filename1, "r");
    FILE *f2 = fopen(filename2, "r");
    
    // obtain file size:
    fseek (f1 , 0 , SEEK_END);
    long size1 = ftell (f1);
    rewind (f1);
    
    // obtain file size:
    fseek (f2 , 0 , SEEK_END);
    long size2 = ftell (f2);
    rewind (f2);
    
    if (size1 != size2) {
        printf("File sizes differ, %ld vs. %ld\n", size1, size2);
        fclose(f1);
        fclose(f2);
        return EXIT_FAILURE;
    }
    
    char tmp1, tmp2;
    
    int files_are_equal = 1;
    
    for (int i=0;i<size2;i++) {
        fread(&tmp1, 1, 1, f1);
        fread(&tmp2, 1, 1, f2);
        if (tmp1 != tmp2) {
            printf("%x: tmp1 0x%x != tmp2 0x%x\n",i , tmp1, tmp2);
            files_are_equal = 0;
            break;
        }
    }

    fclose(f1);
    fclose(f2);

    return files_are_equal ? 0 : -1;
}

void testRepeats() {
    
    printf("== %s ==\n", __func__);
    
    {
        const char *s = "aaaba";
        
        int n = 0;
        int status = number_of_repeated_characters_at_index(s, 0, strlen(s), &n);
        
        assert(status == EXIT_SUCCESS);
        assert(n == 3);
    }
    
    {
        const char *s = "";
        
        int n = 0;
        int status = number_of_repeated_characters_at_index(s, 0, strlen(s), &n);
        
        assert(status == EXIT_SUCCESS);
        assert(n == 0);
    }
    
    {
        const char *s = "asdfg";
        
        int n = 0;
        int status = number_of_repeated_characters_at_index(s, 0, strlen(s), &n);
        
        assert(status == EXIT_SUCCESS);
        assert(n == 1);
    }
}

void test_7_7_to_12() {
    
    printf("== %s ==\n", __func__);
    
    {
        wchar_t u0;
        int status = unichr_12a_from_two_ascii('Z', 'E', &u0);
        assert(status == EXIT_SUCCESS);
        assert(u0 == 0x9485);
    }
    
    {
        wchar_t u0;
        int status = unichr_12a_from_two_ascii('z', ',', &u0);
        assert(status == EXIT_SUCCESS);
        assert(u0 == 0x8CAC);
    }
}

void test_8_8_8_to_12_12() {
    
    printf("== %s ==\n", __func__);
    
    wchar_t u0, u1 = 0;
    
    uint8_t c0 = 0xAB;
    uint8_t c1 = 0xCD;
    uint8_t c2 = 0xEF;
    
    int status = two_twelve_bits_values_from_three_bytes(c0, c1, c2, &u0, &u1);
    
    assert(status == EXIT_SUCCESS);
    
    assert(u0 == 0xABC);
    assert(u1 == 0xDEF);
}

void test_ascii_encoding() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_ascii_encoding_src", "wb+");
    char *s = "abc";
    fwrite(s, 1, strlen(s), fd_in);
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_ascii_encoding", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_ascii_encoding_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_ascii_encoding", "rb");
    
    wchar_t ctrl0 = fgetwc(fd_in3);
    wchar_t ctrl1 = fgetwc(fd_in3);
    
    assert(ctrl0 == 0x9662);
    assert(ctrl1 == 0x0463);
    
    fclose(fd_in3);
}

void test_encode_3_bytes() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_encode_3_bytes_src", "wb+");
    char *s = "\xAB\xCD\xEF";
    fwrite(s, 1, strlen(s), fd_in);
    fclose(fd_in);
    
    FILE *fd_out = fopen("/tmp/test_encode_3_bytes", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_encode_3_bytes_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_encode_3_bytes", "rb");
    
    wchar_t ctrl0 = fgetwc(fd_in3);
    wchar_t ctrl1 = fgetwc(fd_in3);
    
    assert(EOF == fgetwc(fd_in3));
    
    wchar_t file0, file1;
    
    assert(0 == to_U12(0xABC, &file0));
    assert(0 == to_U12(0xDEF, &file1));
    
    assert(ctrl0 == file0);
    assert(ctrl1 == file1);
    
    fclose(fd_in3);
}

void test_encode_5_bytes() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_encode_5_bytes_src", "wb+");
    char *s = "\xAB\xCD\xEF\xAB\xCD";
    fwrite(s, 1, strlen(s), fd_in);
    fclose(fd_in);
    
    FILE *fd_out = fopen("/tmp/test_encode_5_bytes", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_encode_5_bytes_src", "r");
    
    int error = unibinary_encode(fd_in2, fd_out, 0);
    assert(error == 0);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_encode_5_bytes", "rb");
    
    wchar_t ctrl0 = fgetwc(fd_in3);
    wchar_t ctrl1 = fgetwc(fd_in3);
    wchar_t ctrl2 = fgetwc(fd_in3);
    wchar_t ctrl3 = fgetwc(fd_in3);
    
    assert(EOF == fgetwc(fd_in3));
    
    wchar_t file0, file1, file2, file3;
    
    assert(0 == to_U12(0xABC, &file0));
    assert(0 == to_U12(0xDEF, &file1));
    assert(0 == to_U08(0xAB, &file2));
    assert(0 == to_U08(0xCD, &file3));
    
    assert(ctrl0 == file0);
    assert(ctrl1 == file1);
    assert(ctrl2 == file2);
    assert(ctrl3 == file3);
    
    fclose(fd_in3);
}

void test_unichr_12_encoding_decoding() {
    
    printf("== %s ==\n", __func__);
    
    static const unsigned int SIZE = 8;
    
    wchar_t unichr[] = {0x0, 0x1, 0xAB, 0x123, 0xABC, 0xF, 0xFF, 0xFFF};
    
    for(size_t i = 0; i < SIZE; i++) {
        wchar_t o = 0;
        assert(0 == to_U12(unichr[i], &o));
        
        wchar_t o2 = 0;
        assert(0 == from_U12b(o, &o2));
        
        assert(unichr[i] == o2);
    }
}

void test_is_in_U8b() {
    
    printf("== %s ==\n", __func__);
    
    assert(!is_in_U08b(0x03FF));
    
    assert(is_in_U08b(0x0400));
    assert(is_in_U08b(0x04FF));
    
    assert(!is_in_U08b(0x0500));
}

void test_decode_unichars() {
    
    printf("== %s ==\n", __func__);
    
    wchar_t u0, u1;
    assert(to_U12(0xABC, &u0) == 0);
    assert(to_U12(0xDEF, &u1) == 0);
    
    FILE *fd_in = fopen("/tmp/test_decode_unichars_in", "wb+");
    fputwc(u0, fd_in);
    fputwc(u1, fd_in);
    fclose(fd_in);
    
    /**/
    
    FILE *fd_in2 = fopen("/tmp/test_decode_unichars_in", "rb");
    assert(fd_in2 != NULL);
    
    FILE *fd_out = fopen("/tmp/test_decode_unichars_out", "wb+");
    assert(fd_out != NULL);
    
    int status = unibinary_decode(fd_in2, fd_out);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_out2 = fopen("/tmp/test_decode_unichars_out", "rb");
    assert(fd_out2 != NULL);
    
    assert(fgetc(fd_out2) == 0xAB);
    assert(fgetc(fd_out2) == 0xCD);
    assert(fgetc(fd_out2) == 0xEF);
    assert(fgetc(fd_out2) == EOF);
    
    fclose(fd_out2);
}

void test_encode_decode_file() {
    
    printf("== %s ==\n", __func__);

    const char *file_name = "/bin/date";

    {
        FILE *fd_in = fopen(file_name, "rb");
        
        if(fd_in == NULL) {
            fprintf(stderr, "-- skip this test, cannot open file to encode: %s\n", file_name);
        }
        
        FILE *fd_out = fopen("/tmp/date.txt", "wb+");
        
        int status = unibinary_encode(fd_in, fd_out, 0);
        assert(status == EXIT_SUCCESS);
        
        fclose(fd_out);
    }
    
    /**/
    
    {
        FILE *fd_in = fopen("/tmp/date.txt", "rb");
        assert(fd_in != NULL);
        
        FILE *fd_out = fopen("/tmp/date_encoded_decoded", "wb+");
        assert(fd_out != NULL);
        
        int status = unibinary_decode(fd_in, fd_out);
        assert(status == EXIT_SUCCESS);
        
        fclose(fd_out);
        fclose(fd_in);
    }
    
    int error = compareFiles(file_name, "/tmp/date_encoded_decoded");
    assert(error == 0);
}

void test_repeats_2() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_repeats_2_src", "wb+");
    fputc(0xAB, fd_in);
    fputc(0xCD, fd_in);
    fputc(0xEF, fd_in);
    fputc(0xFF, fd_in);
    fputc(0xFF, fd_in);
    fputc(0xFF, fd_in);
    fputc(0xFF, fd_in);
    fputc(0x00, fd_in);
    
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_repeats_2", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_repeats_2_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_repeats_2", "rb");
    
    assert(fgetwc(fd_in) == 0x58BC);
    assert(fgetwc(fd_in) == 0x5BEF);
    assert(fgetwc(fd_in) == 0x04FF);
    assert(fgetwc(fd_in) == 0x4E04);
    assert(fgetwc(fd_in) == 0x0400);
    
    assert(fgetwc(fd_in) == WEOF);
    
    fclose(fd_in3);
}

void test_one_char() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_one_char_src", "wb+");
    fputc('a', fd_in);
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_one_char", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_one_char_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_one_char", "rb");
    
    assert(fgetwc(fd_in) == 0x0461);
    
    assert(fgetwc(fd_in) == WEOF);
    
    fclose(fd_in3);
}

void test_empty_string() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_empty_string_src", "wb+");
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_empty_string", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_empty_string_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_empty_string", "rb");
    assert(fgetwc(fd_in) == WEOF);
    fclose(fd_in3);
}

void test_two_unichr_to_repeat_byte_ntimes() {
    
    printf("== %s ==\n", __func__);
    
    wchar_t u0, u1;
    int status = two_unichr_to_repeat_byte_ntimes('a', 10, &u0, &u1);
    assert(status == EXIT_SUCCESS);
    
    assert(u0 == 0x0461);
    assert(u1 == 0x4E0A);
}

void test_big_repeats_2000_minus_2() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_big_repeats_2000_minus_2_src", "wb+");
    size_t COUNT = 0x2000 - 2;
    size_t i = 0;
    for(i = 0; i < COUNT; i++) {
        fputc(0xAA, fd_in);
    }
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_big_repeats_2000_minus_2", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_big_repeats_2000_minus_2_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_big_repeats_2000_minus_2", "rb");
    
    assert(fgetwc(fd_in) == 0x04AA);
    assert(fgetwc(fd_in) == 0x5DFF);
    assert(fgetwc(fd_in) == 0x04AA);
    assert(fgetwc(fd_in) == 0x5DFF);
    
    assert(fgetwc(fd_in) == WEOF);
    
    fclose(fd_in3);
}

void test_big_repeats_2000() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_big_repeats_2000_src", "wb+");
    size_t COUNT = 0x2000;
    size_t i = 0;
    for(i = 0; i < COUNT; i++) {
        fputc(0xAA, fd_in);
    }
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_big_repeats_2000", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_big_repeats_2000_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_big_repeats_2000", "rb");
    
    assert(fgetwc(fd_in) == 0x04AA);
    assert(fgetwc(fd_in) == 0x5DFF);
    assert(fgetwc(fd_in) == 0x04AA);
    assert(fgetwc(fd_in) == 0x5DFF);
    assert(fgetwc(fd_in) == 0x04AA);
    assert(fgetwc(fd_in) == 0x04AA);
    
    assert(fgetwc(fd_in) == WEOF);
    
    fclose(fd_in3);
}

void test_encode_macho_header() {
    
    FILE *fd_in = fopen("/tmp/test_encode_macho_header_src", "wb+");
    //    const char* s = "\xCF\xFA\xED\xFE\x07\x00\x00\x01";
    fputc(0xCF, fd_in);
    fputc(0xFA, fd_in);
    fputc(0xED, fd_in);
    fputc(0xFE, fd_in);
    fputc(0x07, fd_in);
    fputc(0x00, fd_in);
    fputc(0x00, fd_in);
    fputc(0x01, fd_in);
    fclose(fd_in);
    
    FILE *fd_out = fopen("/tmp/test_encode_macho_header", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_encode_macho_header_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_encode_macho_header", "rb");
    
    assert(fgetwc(fd_in3) == 0x5AFF);
    assert(fgetwc(fd_in3) == 0x58ED);
    assert(fgetwc(fd_in3) == 0x5DE0);
    assert(fgetwc(fd_in3) == 0x5500);
    assert(fgetwc(fd_in3) == 0x5E01);
    
    assert(fgetwc(fd_in3) == WEOF);
    
    fclose(fd_in3);
}

void test_decode_bytes_from_string_4_ascii() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_out = fopen("/tmp/test_decode_bytes_from_string_4_ascii_in", "wb+");
    fputwc(0x9b25, fd_out);
    fputwc(0x9af4, fd_out);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in = fopen("/tmp/test_decode_bytes_from_string_4_ascii_in", "rb+");
    assert(fd_in != NULL);
    
    FILE *fd_out2 = fopen("/tmp/test_decode_bytes_from_string_4_ascii_out", "wb+");
    assert(fd_out2 != NULL);
    
    int status = unibinary_decode(fd_in, fd_out2);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in);
    fclose(fd_out2);
    
    /**/
    
    FILE *fd_out3 = fopen("/tmp/test_decode_bytes_from_string_4_ascii_out", "rb");
    assert(fd_out3 != NULL);
    
    assert(fgetc(fd_out3) == 't');
    assert(fgetc(fd_out3) == 'e');
    assert(fgetc(fd_out3) == 's');
    assert(fgetc(fd_out3) == 't');
    assert(fgetc(fd_out3) == EOF);
    
    fclose(fd_out3);
}

void test_3_bytes_from_unichar() {
    
    printf("== %s ==\n", __func__);
    
    wchar_t u1 = 0x58BC;
    wchar_t u2 = 0x5BEF;
    
    uint8_t b0, b1, b2;
    
    int status = three_bytes_from_unichars(u1, u2, &b0, &b1, &b2);
    assert(status == EXIT_SUCCESS);
    assert(b0 == 0xAB);
    assert(b1 == 0xCD);
    assert(b2 == 0xEF);
}

void test_repeat() {
    
    printf("== %s ==\n", __func__);
    
    FILE *fd_in = fopen("/tmp/test_repeat_src", "wb+");
    fwrite("xxx", sizeof(char), 3, fd_in);
    fclose(fd_in);
    
    /**/
    
    FILE *fd_out = fopen("/tmp/test_repeat", "wb+");
    assert(fd_out != NULL);
    
    FILE *fd_in2 = fopen("/tmp/test_repeat_src", "r");
    
    int status = unibinary_encode(fd_in2, fd_out, 0);
    assert(status == EXIT_SUCCESS);
    
    fclose(fd_in2);
    fclose(fd_out);
    
    /**/
    
    FILE *fd_in3 = fopen("/tmp/test_repeat", "rb");
    
    assert(fgetwc(fd_in) == 0x0478);
    assert(fgetwc(fd_in) == 0x4E03);
    
    assert(fgetwc(fd_in) == WEOF);
    
    fclose(fd_in3);
}

void test_string_encoding() {
    printf("== %s ==\n", __func__);

    wchar_t *wcs;
    unibinary_encode_string("test", &wcs, 0);
    
    assert(wcscmp(wcs, L"\u9B25\u9AF4") == 0);
    free(wcs);
}

void test_string_decoding() {
    printf("== %s ==\n", __func__);

    char* data;
    long dst_len;
    unibinary_decode_string(L"\u9B25\u9AF4", &data, &dst_len);
    assert(dst_len == 4);
    assert(strcmp("test", data) == 0);
    free(data);
}

void test_string_decoding_with_newline() {
    printf("== %s ==\n", __func__);
    
    char* data;
    long dst_len;
    unibinary_decode_string(L"\u9B25\n\u9AF4", &data, &dst_len);
    assert(dst_len == 4);
    assert(strcmp("test", data) == 0);
    free(data);
}

int main(int argc, const char * argv[]) {
    
    setlocale(LC_CTYPE, "UTF-8");
    setlocale(LC_CTYPE, "");
    
    printf("The current locale is %s.\n", setlocale(LC_CTYPE, ""));
    
    test_empty_string();
    test_one_char();
    test_repeats_2();
    test_unichr_12_encoding_decoding();
    testRepeats();
    test_7_7_to_12();
    test_8_8_8_to_12_12();
    test_encode_3_bytes();
    test_encode_5_bytes();
    test_is_in_U8b();
    test_ascii_encoding();
    test_two_unichr_to_repeat_byte_ntimes();
    test_encode_macho_header();
    test_decode_bytes_from_string_4_ascii();
    test_3_bytes_from_unichar();
    test_decode_unichars();
    test_big_repeats_2000_minus_2();
    test_big_repeats_2000();
    test_repeat();
    test_encode_decode_file();
    test_string_encoding();
    test_string_decoding();
    test_string_decoding_with_newline();

    printf("-- ALL TESTS ARE OK --\n");
    
    return 0;
}
