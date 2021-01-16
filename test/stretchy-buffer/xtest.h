TEST_BEGIN_NAME(FUNC, NAME)
{
	size_t i;
	T x;
	Sb(T) a = { 0 }, b = { 0 };

	for (i = 0; i < 32; ++i)
		sb_push(a, RAND(x));

	TEST_ASSERT(sb_len(a) == 32);

	for (i = 0; i < sb_len(a); ++i)
		sb_push(b, a.at[i]);

	TEST_ASSERT(sb_len(b) == 32);

	for (i = 0; i < sb_len(a); ++i)
		TEST_ASSERT(CMP(&a.at[i], &b.at[i]) == 0);

	x = RAND(x);
	sb_push(a, x);
	TEST_ASSERT(CMP(&sb_last(a), &x) == 0);
	TEST_ASSERT(sb_len(a) == 33);
	sb_pop(a);
	TEST_ASSERT(sb_len(a) == 32);

	sb_addn(a, 10);
	TEST_ASSERT(sb_len(a) == 42);

	sb_setlen(b, 0);
	sb_reserve(b, 10);
	TEST_ASSERT(sb_len(b) == 0);
	TEST_ASSERT(sb_cap(b) >= 10);

	/* the value argument should only be evaluated once */
	for (i = 32; i < sb_len(a); ++i)
		sb_push(b, (a.at[i] = RAND(x)));

	TEST_ASSERT(sb_len(b) == 10);

	for (i = 0; i < sb_len(b); ++i)
		TEST_ASSERT(CMP(&b.at[i], &a.at[i+32]) == 0);

	sb_rmn(a, 0, 32);
	TEST_ASSERT(sb_len(a) == 10);

	for (i = 0; i < sb_len(a); ++i)
		TEST_ASSERT(CMP(&a.at[i], &b.at[i]) == 0);

	sb_setcap(a, 420);
	TEST_ASSERT(sb_cap(a) >= 420);
	sb_shrink(a);

	sb_free(b);
	sb_initlen(b, 32);
	TEST_ASSERT(sb_len(b) == 32);
	TEST_ASSERT(sb_cap(b) >= 32);
	for (i = 0; i < sb_len(b); ++i)
		b.at[i] = RAND(x);

	sb_insn(b, 10, 10);
	TEST_ASSERT(sb_len(b) == 42);
	for (i = 0; i < 10; ++i)
		b.at[10 + i] = a.at[i];

	for (i = 0; i < 10; ++i) {
		x = b.at[19];
		sb_rm(b, 19);
		TEST_ASSERT(sb_len(b) == 41);
		sb_ins(b, 0, x);
		TEST_ASSERT(sb_len(b) == 42);
	}

	TEST_ASSERT(sb_len(b) == 42);
	for (i = 0; i < 10; ++i)
		TEST_ASSERT(CMP(&a.at[i], &b.at[i]) == 0);


	sb_free(a);
	sb_initcap(a, 42);
	TEST_ASSERT(sb_len(a) == 0);
	TEST_ASSERT(sb_cap(a) >= 42);

	for (i = 0; i < 42; ++i)
		sb_push(a, b.at[i]);

	TEST_ASSERT(sb_len(a) == 42);

	sb_popn(a, 9);
	TEST_ASSERT(sb_len(a) == 33);
	sb_pop(a);
	TEST_ASSERT(sb_len(a) == 32);

	for (i = 0; i < 32; ++i)
		TEST_ASSERT(CMP(&a.at[i], &b.at[i]) == 0);


	sb_free(a);
	sb_free(b);
} TEST_END
#undef T
#undef NAME
#undef FUNC
