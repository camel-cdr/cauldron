# cauldron

A collection of single-file C libraries and tools with the goal to be portable and modifiable.

[![Tests](https://github.com/camel-cdr/cauldron/workflows/Tests/badge.svg)](https://github.com/camel-cdr/cauldron/actions?workflow=Tests)


## Libraries

 library                                             | description                                                                                | language
:-------                                             | :-----------                                                                               | :--------:
 **[arena-allocator.h](cauldron/arena-allocator.h)** | drop in arena allocator                                                                    | C
 **[arg.h](cauldron/arg.h)**                         | POSIX compliant argument parser based on plan9's arg(3)                                    | C/C++
 **[bench.h](cauldron/bench.h)**                     | micro benchmarking framework                                                               | C/C++
 **[random.h](cauldron/random.h)**                   | literate random number library and tutorial [(related talk)](https://youtu.be/VHJUlRiRDCY) | C/C++
 **[stretchy-buffer.h](cauldron/stretchy-buffer.h)** | generic dynamic array                                                                      | C
 **[test.h](cauldron/test.h)**                       | minimal unit testing                                                                       | C/C++

## Tools

### Bithacks
* [unsigned division by constants](tools/bithacks/unsigned-division-by-constant.c)

### Random
* [RNG benchmark](tools/random/bench.c)
* RNG cli tools: [rng](tools/random/rng.c), [dist](tools/random/dist.c)
* [generate ziggurat constants](tools/random/ziggurat-constants.c)
* [Improving Andrew Kensler's permute(): A function for stateless, constant-time pseudorandom-order array iteration](tools/random/permute)

## Similar projects
* [klib](https://github.com/attractivechaos/klib)
* [portable-snippets](https://github.com/nemequ/portable-snippets)
* [stb](https://github.com/nothings/stb)

## Licensing
For all files without a integrated license [LICENSE](LICENSE) applies.
