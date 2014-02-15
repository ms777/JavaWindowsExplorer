rem use gendef to generate the def files

rem dlltool options taken from mingw-w64-v3.1.0\mingw-w64-crt\Makefile.am

C:\MinGW64_TDM\x86_64-w64-mingw32\bin\dlltool -m i386:x86-64 -k --as-flags=--64 -d shell32_ms777-64.def -l libshell32_ms777.a -v
move libshell32_ms777.a ./lib

C:\MinGW64_TDM\x86_64-w64-mingw32\bin\dlltool -m i386        -k --as-flags=--32 -d shell32_ms777-32.def -l libshell32_ms777.a -v 
move libshell32_ms777.a ./lib32


rem --as C:\MinGW64_TDM\x86_64-w64-mingw32\bin\as.exe
