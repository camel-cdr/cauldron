#!/bin/sh

cpp=0


CFLAGS="-I../ -lm -Wall -Wextra -Werror=vla -Wno-unused -pedantic -ggdb "
CXXFLAGS="-I../ -lm -Wall -Wextra -Werror=vla -Wno-unused -pedantic -xc++ -ggdb3"

out=$(mktemp)
trap '{ rm -f -- "$out"; }' EXIT


in=$1
shift 1
for var in "$@"
do
	[ "$var" = "c++" ] && cpp=1
	[ "$var" = "c89" ] && CVER="-Dinline=  -std=c89 "
	[ "$var" = "c99" ] && CVER="-std=c99 "
done

CFLAGS=$CFLAGS$CVER

# Test with sanitizers
clang $CFLAGS -fsanitize=address,undefined,leak -ftrapv $in -o $out && $out

# Test with valgrind
clang $CFLAGS $in -o $out && valgrind -q $out

# Test on big endian mips if available
if which mips-linux-gnu-gcc qemu-mips >/dev/null
then
	mips-linux-gnu-gcc -I../ -ggdb -static $in -lm -o $out && qemu-mips $out
fi

# Test C++
[ $cpp -eq 1 ] && clang++ $CXXFLAGS -xc++ $in -o $out && $out


exit 0
