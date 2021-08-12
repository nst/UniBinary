//
//  main.c
//  unibinary
//
//  Created by Nicolas Seriot on 29/12/13.
//  Copyright (c) 2013 Nicolas Seriot. All rights reserved.
//

#include "unibinary.h"
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdio.h>

int display_usage() {
    printf("Usage: unibinary [-ed] [-sf] [-b num] [-h]\n");
    printf("\n");
    printf("UniBinary encodes and decodes data into printable Unicode characters.\n");
    printf("\n");
    printf("  -e, --encode\n");
    printf("  -d, --decode\n");
    printf("  -s, --string    to be encoded or decoded\n");
    printf("  -f, --filepath  to be encoded or decoded\n");
    printf("  -b, --break     break encoded string into num characters lines\n");
    printf("  -h, --help      show this help message and exit\n");
    return EXIT_SUCCESS;
}

static const struct option long_options[] =
{
    { "encode", no_argument, 0, 'e' },
    { "decode", no_argument, 0, 'd' },
    { "string", required_argument, 0, 's' },
    { "path", required_argument, 0, 'f' },
    { "break", required_argument, 0, 'b' },
    { "help", no_argument, 0, 'h' },
    { NULL, 0, NULL, 0 }
};

struct global_args_t {
    short encode;
    short decode;
    char *string;
    const char *path;
    short wrap;
} global_args;

int main(int argc, char * const argv[]) {

    //    $ echo test | ./unibinary -e | ./unibinary -d
    //    test

    char *old_locale = setlocale(LC_ALL, NULL);
    char *saved_locale = strdup(old_locale);
    if(saved_locale == NULL) return EXIT_FAILURE;
    
    setlocale(LC_CTYPE, "");
    
    static const char *opt_string = "eds:f:b:h";

    int opt = getopt_long( argc, argv, opt_string, long_options, NULL);
    while( opt != -1 ) {
        switch( opt ) {
            case 'e':
                global_args.encode = 1;
                break;
            case 'd':
                global_args.decode = 1;
                break;
            case 's':
                global_args.string = optarg;
                break;
            case 'f':
                global_args.path = optarg;
                break;
            case 'b':
                global_args.wrap = atoi(optarg);
                break;
//            case 'h':
//                display_usage();
//                goto exit_failure;
                break;
            default:
                break;
        }
        
        opt = getopt_long( argc, argv, opt_string, long_options, NULL);
    }
    
    if(global_args.encode) {
        // encode
        
        if(global_args.string != NULL) {
            // encode string
            wchar_t *wcs;
            unibinary_encode_string(global_args.string, &wcs, global_args.wrap);
            fwprintf(stdout, wcs);
            free(wcs);
        } else if (global_args.path != NULL) {
            // encode path
            FILE *fd_in = fopen(global_args.path, "rb");
            if(fd_in == NULL) goto exit_failure;
            
            int status = unibinary_encode(fd_in, stdout, global_args.wrap);
            fclose(fd_in);
            
            if(status != 0) goto exit_failure;
        } else {
            // encode stdin
            int status = unibinary_encode(stdin, stdout, global_args.wrap);
            if(status != 0) goto exit_failure;
        }
    } else if (global_args.decode) {
        // decode
        
        if(global_args.string != NULL) {
            // decode string
            size_t max_wchar_bytes = strlen(global_args.string) * MB_CUR_MAX;
            wchar_t wcsout[max_wchar_bytes];
            size_t nb_wc = mbstowcs(wcsout, global_args.string, max_wchar_bytes);
            if(nb_wc == -1) goto exit_failure;
            
            char* data;
            long dst_len;
            unibinary_decode_string(wcsout, &data, &dst_len);
            size_t written = fwrite(data, sizeof(char), dst_len, stdout);
            free(data);
            
            if(written != dst_len) goto exit_failure;
        
        } else if (global_args.path != NULL) {
            // decode path
            FILE *fd_in = fopen(global_args.path, "rb");
            if(fd_in == NULL) goto exit_failure;
            
            int status = unibinary_decode(fd_in, stdout);
            fclose(fd_in);
            
            if(status != 0) goto exit_failure;
        } else {
            // decode stdin
            int status = unibinary_decode(stdin, stdout);
            if(status != 0) goto exit_failure;

        }
        
    } else {
        display_usage();
    }
    
    if(global_args.encode || global_args.decode) {
        // add a newline if stdout is not piped
        if (isatty(fileno(stdout)) == 1) {
            fflush(stdout);
            fprintf(stderr, "\n");
        }
    }
    
    goto exit_success;

exit_success:
    setlocale(LC_ALL, saved_locale);
    free(saved_locale);
    
    return EXIT_SUCCESS;

exit_failure:
    setlocale(LC_ALL, saved_locale);
    free(saved_locale);
    
    return EXIT_FAILURE;
}
