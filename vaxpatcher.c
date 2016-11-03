// mini patcher v0.1

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

const int BUF_SIZE = 1024;
const int ARGS_MAX = 3;

FILE *fpTarget;
long int pos;

void assert ( bool cond, const char* format, ... ) {
    if ( cond ) {
        return;
    }
    va_list argptr;
    va_start( argptr, format );
    vfprintf( stderr, format, argptr );
    va_end( argptr );
    exit(1);
}

int hexToInt ( char hexChar ) {
    return ( hexChar >= '0' && hexChar <= '9' ) ? hexChar - '0'
         : ( hexChar >= 'A' && hexChar <= 'Z' ) ? 10 + hexChar - 'A'
         : ( hexChar >= 'a' && hexChar <= 'z' ) ? 10 + hexChar - 'a'
         : -1;
}

#include "getline.c"

char* replaceNewline ( char* inputAsciiz ) {
    for ( char* ptr = inputAsciiz; *ptr != '\0'; ptr++ ) {
        if ( *ptr == '\n' || *ptr == '\r' ) {
            *ptr = '\0';
        }
    }
    return inputAsciiz;
}

void openTarget( char* filename ) {
    if ( fpTarget != NULL ) {
        fclose( fpTarget );
    }
    fpTarget = fopen( filename, "r+b" );
    assert( fpTarget != NULL, "can't open target file '%s'\n", filename );
}

int main ( int argc, char *argv[] ) {
    char *buf = NULL;
    char *bufTmp;
    size_t bufSize;
    char *filename;
    if ( argc < 2 ) {
        filename = "patch.txt";
    } else {
        filename = argv[1];
        assert( argc <= ARGS_MAX, "too many arguments (max %d, found %d instead)\n", ARGS_MAX - 1, argc - 1 );
        if ( argc == 3 ) {
            openTarget( argv[2] );
        }
    }
    bool debug = false;
    FILE *fpPatch = fopen( filename, "r" );
    assert( fpPatch != NULL, "can't open input file '%s'\n", filename );
    while( !feof(fpPatch) ) {
        getline( &buf, &bufSize, fpPatch );
        replaceNewline( buf );
        if ( debug ) {
            printf( "%s\n", buf );
        }
        if ( strlen(buf) == 0 ) {
            continue;
        }
        bufTmp = buf + 1;
        switch ( buf[0] ) {
            case ':':
                printf( "%s\n", buf );
                debug = true;
            case '#':
            case ' ':
                continue;
            case '!':
                openTarget( bufTmp );
                break;
            case '^':
                if ( bufTmp[0] != '*' ) {
                    openTarget( bufTmp );
                } else {
                    assert( argc == 3, "no command-line filename was supplied, yet '^*' (make backup) sequence found\n" );
                    bufTmp = argv[2];
                }
                char* filenameBackup = malloc( strlen(bufTmp) + 1 + 4 );
                strcpy( filenameBackup, bufTmp );
                strcat( filenameBackup, ".bak" );
                FILE *fpBackup = fopen( filenameBackup, "wb" );
                assert( fpTarget != NULL, "can't create backup '%s'\n", filenameBackup );
                free( filenameBackup );
                for( int c = fgetc( fpTarget ); c != EOF; c = fgetc( fpTarget ) ) {
                    fputc( c, fpBackup );
                }
                fclose( fpBackup );
                fseek( fpTarget, 0, SEEK_SET );
                break;
            case '@':
                assert( fpTarget != NULL, "no target file open yet\n" );
                pos = strtol( bufTmp, NULL, 16 );
                fseek( fpTarget, pos, SEEK_SET );
                break;
            case '>': {
                assert( fpTarget != NULL, "no target file open yet\n" );
                bool gotFirst = false;
                int expected, found;
                for( int i = 0, bufTmpSize = bufSize - 1; i < bufTmpSize; i++ ) {
                    if ( bufTmp[i] == '\0' ) {
                        break;
                    }
                    if ( bufTmp[i] == ' ' ) {
                        continue;
                    }
                    assert( isxdigit( bufTmp[i] ), "invalid patch file: not a hexdigit: '%c' [%d]\n", bufTmp[i], bufTmp[i] );
                    if ( gotFirst ) {
                        expected *= 0x10;
                        expected += hexToInt( bufTmp[i] );
                        found = fgetc( fpTarget );
                        assert( found == expected, "expected: '%c' [0x%02X], but instead found '%c' [0x%02X] at offset 0x%08X"
                                                   " (file either invalid or patched already)",
                                    expected, expected, found, found, ftell( fpTarget ) - 1 );
                        gotFirst = false;
                    } else {
                        expected = hexToInt( bufTmp[i] );
                        gotFirst = true;
                    }
                }
                fseek( fpTarget, pos, SEEK_SET );
                break;
            }
            case '<': {
                assert( fpTarget != NULL, "no target file open yet\n" );
                bool gotFirst = false;
                char c;
                for( int i = 0, bufTmpSize = bufSize - 1; i < bufTmpSize; i++ ) {
                    if ( bufTmp[i] == '\0' ) {
                        break;
                    }
                    if ( bufTmp[i] == ' ' ) {
                        continue;
                    }
                    assert( isxdigit( bufTmp[i] ), "invalid patch file: not a hexdigit: '%c' [%d]\n", bufTmp[i], bufTmp[i] );
                    if ( gotFirst ) {
                        c *= 0x10;
                        c += hexToInt( bufTmp[i] );
                        fputc( c, fpTarget );
                        gotFirst = false;
                    } else {
                        c = hexToInt( bufTmp[i] );
                        gotFirst = true;
                    }
                }
                fseek( fpTarget, pos, SEEK_SET );
                break;
            }
            default:
                assert( false, "invalid patch file: unknown command '%s'\n", buf );
        }
    }

    if ( fpTarget != NULL ) {
         fclose( fpTarget );
    }
    fclose( fpPatch );

    return 0;
}