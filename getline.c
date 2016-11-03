
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>

void flockfile ( FILE* file ) {}
void funlockfile ( FILE* file ) {}

ssize_t getdelim ( char** lineptr, size_t* n, int delimiter, FILE* fp ) {
    int result = 0;
    ssize_t cur_len = 0;
    //ssize_t len;

    if (lineptr == NULL || n == NULL || fp == NULL) {
        errno = EINVAL;
        return -1;
    }

    flockfile(fp);

    if (*lineptr == NULL || *n == 0) {
        *n = 128;
        *lineptr = (char*) malloc(*n);
        if (*lineptr == NULL) {
            result = -1;
            goto unlock_return;
        }
    }

    for(;;) {
        //char* t;
        int i;

        i = getc(fp);
        if (i == EOF) {
            result = -1;
            break;
        }

        /* Make enough space for len+1 (for final NUL) bytes.  */
        if (cur_len + 1 >= *n) {
            size_t needed = 2 * (cur_len + 1) + 1; /* Be generous. */
            char* new_lineptr;

            if (needed < cur_len) {
                result = -1;
                goto unlock_return;
            }

            new_lineptr = (char*)realloc(*lineptr, needed);
            if (new_lineptr == NULL) {
                result = -1;
                goto unlock_return;
            }

            *lineptr = new_lineptr;
            *n = needed;
        }

        (*lineptr)[cur_len] = i;
        cur_len++;

        if (i == delimiter)
            break;
    }
    (*lineptr)[cur_len] = '\0';
    result = cur_len ? cur_len : result;

unlock_return:
    funlockfile(fp);
    return result;
}

ssize_t getline ( char **lineptr, size_t *n, FILE *stream ) {
    return getdelim (lineptr, n, '\n', stream);
}
