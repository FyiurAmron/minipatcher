gcc vaxpatcher.c -ovaxpatcher.exe -std=c99 -Ofast -Wall -flto
strip -s vaxpatcher.exe
upx -9 vaxpatcher.exe
copy vaxpatcher.exe vp.exe
vp patches\upx_obfusc.patch.txt vaxpatcher.exe
del vp.exe
