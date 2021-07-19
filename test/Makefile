.POSIX:

all: arena-allocator random-target streachy-buffer-target

arena-allocator:
	./test.sh arena-allocator.c c89

random-target: random-shuf random-jump random-dist-normal random-dist-uniform \
               random-dist-uniform-dense
random-shuf:
	./test.sh random/shuf.c c++ c89
random-jump:
	./test.sh random/jump.c c++ c89
random-dist-normal:
	./test.sh random/dist_normal.c c++ c89
random-dist-uniform:
	./test.sh random/dist_uniform.c c++ c89
random-dist-uniform-dense:
	./test.sh random/dist_uniform_dense.c c++ c99

streachy-buffer-target:
	./test.sh stretchy-buffer/test.c c89
