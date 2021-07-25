RANDOM_X16(PRNG16Msws, prng16_msws, prng16_msws_randomize)
RANDOM_X16(PRNG16Sfc, prng16_sfc, prng16_sfc_randomize)

RANDOM_X32(PRNG32Msws, prng32_msws, prng32_msws_randomize)
RANDOM_X32(PRNG32Sfc, prng32_sfc, prng32_sfc_randomize)
RANDOM_X32(PRNG32Jfs, prng32_jfs, prng32_jfs_randomize)
RANDOM_X32(PRNG32JavaUtilRandom, prng32_java_util_random, prng32_java_util_random_randomize)

#if __SIZEOF_INT128__
RANDOM_X64(PRNG64Msws, prng64_msws, prng64_msws_randomize)
#endif
RANDOM_X64(PRNG64Msws64_2x32, prng64_msws_2x32bit, prng64_msws_2x32bit_randomize)
RANDOM_X64(PRNG64Sfc, prng64_sfc, prng64_sfc_randomize)
RANDOM_X64(PRNG64Tylo, prng64_tylo, prng64_tylo_randomize)
RANDOM_X64(PRNG64Jfs, prng64_jfs, prng64_jfs_randomize)
RANDOM_X64(PRNG64Xorshift128p, prng64_xorshift128p, prng64_xorshift128p_randomize)
