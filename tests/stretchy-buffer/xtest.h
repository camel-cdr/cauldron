{
	size_t i;
	T x;
	Sb(T) a, b;
	sb_init(a);
	sb_init(b);

	for (i = 0; i < 32; ++i)
		sb_push(a, RAND(x));

	assert(sb_len(a) == 32);

	for (i = 0; i < sb_len(a); ++i)
		sb_push(b, a.at[i]);

	assert(sb_len(b) == 32);

	for (i = 0; i < sb_len(a); ++i)
		assert(CMP(&a.at[i], &b.at[i]) == 0);

	x = RAND(x);
	sb_push(a, x);
	assert(CMP(&sb_last(a), &x) == 0);
	assert(sb_len(a) == 33);
	sb_pop(a);
	assert(sb_len(a) == 32);

	sb_addn(a, 10);
	assert(sb_len(a) == 42);

	sb_setlen(b, 0);
	sb_reserve(b, 10);
	assert(sb_len(b) == 0);
	assert(sb_cap(b) >= 10);

	/* the value argument should only be evaluated once */
	for (i = 32; i < sb_len(a); ++i)
		sb_push(b, (a.at[i] = RAND(x)));

	assert(sb_len(b) == 10);

	for (i = 0; i < sb_len(b); ++i)
		assert(CMP(&b.at[i], &a.at[i+32]) == 0);

	sb_rmn(a, 0, 32);
	assert(sb_len(a) == 10);

	for (i = 0; i < sb_len(a); ++i)
		assert(CMP(&a.at[i], &b.at[i]) == 0);

	sb_setcap(a, 420);
	assert(sb_cap(a) >= 420);
	sb_shrink(a);

	sb_free(b);
	sb_initlen(b, 32);
	assert(sb_len(b) == 32);
	assert(sb_cap(b) >= 32);
	for (i = 0; i < sb_len(b); ++i)
		b.at[i] = RAND(x);

	sb_insn(b, 10, 10);
	assert(sb_len(b) == 42);
	for (i = 0; i < 10; ++i)
		b.at[10 + i] = a.at[i];

	for (i = 0; i < 10; ++i) {
		x = b.at[19];
		sb_rm(b, 19);
		assert(sb_len(b) == 41);
		sb_ins(b, 0, x);
		assert(sb_len(b) == 42);
	}

	assert(sb_len(b) == 42);
	for (i = 0; i < 10; ++i)
		assert(CMP(&a.at[i], &b.at[i]) == 0);


	sb_free(a);
	sb_initcap(a, 42);
	assert(sb_len(a) == 0);
	assert(sb_cap(a) >= 42);

	for (i = 0; i < 42; ++i)
		sb_push(a, b.at[i]);

	assert(sb_len(a) == 42);

	sb_popn(a, 9);
	assert(sb_len(a) == 33);
	sb_pop(a);
	assert(sb_len(a) == 32);

	for (i = 0; i < 32; ++i)
		assert(CMP(&a.at[i], &b.at[i]) == 0);

	sb_free(a);
	sb_free(b);
}
#undef T
