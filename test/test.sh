#!/bin/sh

cpp=0

if [ "$#" -eq 1 ]
then
	in=$1
elif [ "$#" -eq "2" ] && [ "$1" = "c++" ]
then
	cpp=1
	in=$2
else
	echo "usage: $0 [c++] FILE"
	exit 1
fi


CFLAGS="-I../ -lm -Wall -Wextra -Wno-unused -pedantic -std=c89 -Dinline=  -ggdb -Werror=vla"
CXXFLAGS="-I../ -lm -Wall -Wextra -Wno-unused -pedantic -xc++ -ggdb3"


out=$(mktemp)
trap '{ rm -f -- "$out"; }' EXIT

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
