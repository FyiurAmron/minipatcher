gcc vaxpatcher.c -ovaxpatcher.exe -std=c99 -Ofast -Wall -flto
strip -s vaxpatcher.exe
