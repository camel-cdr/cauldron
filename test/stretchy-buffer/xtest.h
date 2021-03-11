void
FUNC(void)
{
	size_t i;
	T x;
	Sb(T) a = { 0 }, b = { 0 };

	TEST_BEGIN(NAME);

	for (i = 0; i < 32; ++i)
		sb_push(a, RAND(x));

	TEST_ASSERT(sb_len(a) == 32);

	for (i = 0; i < sb_len(a); ++i)
		sb_push(b, a.at[i]);

	TEST_ASSERT(sb_len(b) == 32);

	for (i = 0; i < sb_len(a); ++i)
		TEST_ASSERT(EQ(a.at[i], b.at[i]));

	x = RAND(x);
	sb_push(a, x);
	TEST_ASSERT(EQ(a.at[sb_len(a) - 1], x));
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
		TEST_ASSERT(EQ(b.at[i], a.at[i+32]));

	sb_rmn(a, 0, 32);
	TEST_ASSERT(sb_len(a) == 10);

	for (i = 0; i < sb_len(a); ++i)
		TEST_ASSERT(EQ(a.at[i], b.at[i]));

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
		TEST_ASSERT(EQ(a.at[i], b.at[i]));


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
		TEST_ASSERT(EQ(a.at[i], b.at[i]));

	sb_free(a);

	sb_addn(a, 10);

	for (i = 0; i < 10; ++i)
		a.at[i] = RAND(x);
	sb_free(b);
	sb_cpy(b, a);

	TEST_ASSERT(sb_len(a) == 10);
	for (i = 0; i < 10; ++i)
		TEST_ASSERT(EQ(a.at[i], b.at[i]));

	sb_rm(a, 9);
	sb_rm(a, 0);
	TEST_ASSERT(sb_len(a) == 8);
	for (i = 0; i < 8; ++i)
		TEST_ASSERT(EQ(a.at[i], b.at[i + 1]));

	sb_popn(a, 8);
	TEST_ASSERT(sb_len(a) == 0);

	sb_ins(a, 0, b.at[2]);
	sb_ins(a, 0, b.at[0]);
	sb_ins(a, 1, b.at[1]);
	sb_ins(a, 3, b.at[3]);
	TEST_ASSERT(sb_len(a) == 4);
	for (i = 0; i < 4; ++i)
		TEST_ASSERT(EQ(a.at[i], b.at[i]));

	sb_rmn(a, 2, 2);
	TEST_ASSERT(sb_len(a) == 2);
	for (i = 0; i < 2; ++i)
		TEST_ASSERT(EQ(a.at[i], b.at[i]));

	sb_rm(a, 0);
	sb_rm(a, 0);
	TEST_ASSERT(sb_len(a) == 0);

	sb_rmn_unstable(b, 5, 5);
	TEST_ASSERT(sb_len(b) == 5);
	sb_free(a);

	sb_cpy(a, b);
	sb_rm_unstable(b, 3);
	TEST_ASSERT(sb_len(b) == 4);
	TEST_ASSERT(EQ(a.at[0], b.at[0]));
	TEST_ASSERT(EQ(a.at[1], b.at[1]));
	TEST_ASSERT(EQ(a.at[2], b.at[2]));
	TEST_ASSERT(EQ(a.at[4], b.at[3]));
	TEST_ASSERT(EQ(a.at[5], b.at[4]));

	sb_free(a);
	sb_free(b);
	TEST_END();
}
#undef T
#undef NAME
#undef FUNC

