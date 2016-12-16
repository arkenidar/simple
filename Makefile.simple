all: simple.c
	gcc -o "simple" "simple.c"

optimized: simple.c
	gcc -o "simple" -O3 "simple.c"

debug: simple.c
	gcc -o "simple" -g "simple.c"

clean:
	rm simple || true
	rm simple.exe || true

cross: simple.c
	x86_64-w64-mingw32-gcc -o "simple.exe" "simple.c"
