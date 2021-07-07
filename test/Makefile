.POSIX:

all: arena-allocator random-target streachy-buffer-target

arena-allocator:
	./test.sh arena-allocator.c

random-target: random-shuf random-jump random-dist-normal
random-shuf:
	./test.sh c++ random/shuf.c
random-jump:
	./test.sh c++ random/jump.c
random-dist-normal:
	./test.sh c++ random/dist_normal.c
random-dist-uniform:
	./test.sh c++ random/dist_uniform.c

streachy-buffer-target:
	./test.sh stretchy-buffer/test.c
