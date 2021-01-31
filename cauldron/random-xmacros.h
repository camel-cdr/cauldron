#if PRNG32_PCG_AVAILABLE
RANDOM_X32(PRNG32Pcg, prng32_pcg, prng32_pcg_randomize)
#endif
#if PRNG32_ROMU_AVAILABLE
RANDOM_X32(PRNG32RomuTrio, prng32_romu_trio, prng32_romu_trio_randomize)
RANDOM_X32(PRNG32RomuQuad, prng32_romu_quad, prng32_romu_quad_randomize)
#endif
#if PRNG32_XORSHIFT_AVAILABLE
RANDOM_X32(PRNG32Xoroshiro64, prng32_xoroshiro64s, prng32_xoroshiro64_randomize)
RANDOM_X32(PRNG32Xoroshiro64, prng32_xoroshiro64ss, prng32_xoroshiro64_randomize)
RANDOM_X32(PRNG32Xoshiro128, prng32_xoshiro128s, prng32_xoshiro128_randomize)
RANDOM_X32(PRNG32Xoshiro128, prng32_xoshiro128ss, prng32_xoshiro128_randomize)
#endif
#if CSPRNG32_CHACHA_AVAILABLE
RANDOM_X32(CSPRNG32Chacha, csprng32_chacha, csprng32_chacha_randomize)
#endif

#if PRNG64_PCG_AVAILABLE
RANDOM_X64(PRNG64Pcg, prng64_pcg, prng64_pcg_randomize)
#endif
#if PRNG64_ROMU_AVAILABLE
RANDOM_X64(PRNG64RomuDuo, prng64_romu_duo, prng64_romu_duo_randomize)
RANDOM_X64(PRNG64RomuDuo, prng64_romu_duo_jr, prng64_romu_duo_randomize)
RANDOM_X64(PRNG64RomuTrio, prng64_romu_trio, prng64_romu_trio_randomize)
RANDOM_X64(PRNG64RomuQuad, prng64_romu_quad, prng64_romu_quad_randomize)
#endif
#if PRNG64_XORSHIFT_AVAILABLE
RANDOM_X64(PRNG64Xoroshiro128, prng64_xoroshiro128p, prng64_xoroshiro128_randomize)
RANDOM_X64(PRNG64Xoroshiro128, prng64_xoroshiro128ss, prng64_xoroshiro128_randomize)
RANDOM_X64(PRNG64Xoshiro256, prng64_xoshiro256p, prng64_xoshiro256_randomize)
RANDOM_X64(PRNG64Xoshiro256, prng64_xoshiro256ss, prng64_xoshiro256_randomize)
#endif
