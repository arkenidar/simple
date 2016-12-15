Debug, Release: simple.c
	gcc -o "simple" "simple.c"

win: simple.c
	x86_64-w64-mingw32-gcc -o "simple.exe" "simple.c"
