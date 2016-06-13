#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errol.h>
#include <gmp.h>
#include <inttypes.h>


/*
 * local function declarations
 */

static bool opt_long(char ***arg, const char *pre, char **value);
static bool opt_num(char ***arg, const char *pre, int *num);

static int intsort(const void *left, const void *right);
static int err_t_sort(const void *left, const void *right);

static double chk_conv(double val, const char *str, int32_t exp, bool *cor, bool *opt, bool *best);

static void table_add(struct errol_err_t[static 1024], int i, double val);
static void table_enum(unsigned int ver, bool bld);
static void table_to_tree(struct errol_err_t *table, int n);

/*
 * interop function declarations
 */

double rndval();
void reseed(uint_fast64_t value);
uint_fast64_t get_seed();

int grisu3_proc(double val, char *buf, bool *suc);
uint32_t grisu_bench(double val, bool *suc);

int dragon4_proc(double val, char *buf);
uint32_t dragon4_bench(double val, bool *suc);

bool errolN_check(unsigned int n, double val);
bool errolNu_check(unsigned int n, double val);
int errolN_proc(unsigned int n, double val, char *buf, bool *opt);
uint32_t errolN_bench(unsigned int n, double val, bool *suc);

/*
 * proof function declarations
 */

int64_t *proof_enum(mpz_t delta, mpz_t m0, mpz_t alpha, mpz_t tau, unsigned int p);


/**
 * Main entry point.
 *   @argc: The number of arguments.
 *   @argv: The argument array.
 *   &returns: The error code.
 */

int main(int argc, char **argv)
{
	char **arg;
	bool quiet = false, enum3 = false, enum4 = false, check3 = false, check4 = false;
	int n, perf = 0, fuzz[5] = { 0, 0, 0, 0, 0 };

	rndval();

	for(arg = argv + 1; *arg != NULL; ) {
		if(opt_num(&arg, "fuzz0", &n))
			fuzz[0] = n;
		else if(opt_num(&arg, "fuzz1", &n))
			fuzz[1] = n;
		else if(opt_num(&arg, "fuzz2", &n))
			fuzz[3] = n;
		else if(opt_num(&arg, "fuzz3", &n))
			fuzz[3] = n;
		else if(opt_num(&arg, "fuzz4", &n))
			fuzz[4] = n;
		else if(opt_num(&arg, "perf", &n))
			perf = n;
		else if(opt_long(&arg, "enum3", NULL))
			enum3 = true;
		else if(opt_long(&arg, "enum4", NULL))
			enum4 = true;
		else if(opt_long(&arg, "check3", NULL))
			check3 = true;
		else if(opt_long(&arg, "check4", NULL))
			check4 = true;
		else
			fprintf(stderr, "Invalid option '%s'.\n", *arg), abort();
	}

	for(n = 0; n < 5; n++) {
		unsigned int i, nfail = 0, subopt = 0, notbest = 0;

		if(fuzz[n] == 0)
			continue;

		for(i = 0; i < fuzz[n]; i++) {
			int exp;
			char str[32];
			double val, chk;
			bool cor, opt, best;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KFuzzing Errol%u... %uk/%uk %2.2f%%", n, i / 1000, fuzz[n] / 1000, 100.0 * (double)i / (double)fuzz[n]);
				fflush(stdout);
			}

			val = rndval();
			exp = errolN_proc(n, val, str, &opt);
			chk = chk_conv(val, str, exp, &cor, &opt, &best);

			nfail += (cor ? 0 : 1);
			subopt += (opt ? 0 : 1);
			notbest += (best ? 0 : 1);

			if(!best && !quiet) {
				int dragon4exp;
				char dragon4str[128];

				dragon4exp = dragon4_proc(val, dragon4str);
				fprintf(stderr, "Conversion failed. Expected 0.%se%d. Actual 0.%se%d. Read as %.17e.\n", dragon4str, dragon4exp, str, exp, chk);
			}
		}

		printf("\x1b[G\x1b[KFuzzing Errol%u done on %u numbers, %u failures (%.3f%%), %u suboptimal (%.3f%%), %u notbest (%.3f%%)\n", n, fuzz[n], nfail, 100.0 * (double)nfail / (double)fuzz[n], subopt, 100.0 * (double)subopt / (double)fuzz[n], notbest, 100.0 * (double)notbest / (double)fuzz[n]);
	}

	if(perf > 0) {
#define N	100
#define Nlow	(N / 10)
#define Nhigh	(N - Nlow)
#define Nsize	(Nhigh - Nlow)

		uint_fast64_t seed = get_seed();
		uint32_t dragon4 = 0, grisu3 = 0, errol[5] = { 0, 0, 0, 0, 0 }, adj3 = 0;
		unsigned int i, j;
		static uint32_t dragon4all[20000][N], grisu3all[20000][N], errolNall[5][20000][N], adj3all[20000][N];

		if(perf > 20000)
			fprintf(stderr, "Cannot support more than 20k performance numberss.\n"), abort();

		for(j = 0; j < N; j++) {
			reseed(seed);
			for(i = 0; i < perf; i++) {
				double val;
				bool suc;

				val = rndval();

				dragon4all[i][j] = dragon4_bench(val, &suc);
				errolNall[0][i][j] = errolN_bench(0, val, &suc);
				errolNall[1][i][j] = errolN_bench(1, val, &suc);
				errolNall[2][i][j] = errolN_bench(2, val, &suc);
				errolNall[3][i][j] = errolN_bench(3, val, &suc);
				errolNall[4][i][j] = errolN_bench(4, val, &suc);
				grisu3all[i][j] = grisu_bench(val, &suc);
				adj3all[i][j] = suc ? grisu3all[i][j] : dragon4all[i][j];
			}
		}

		reseed(seed);

		for(i = 0; i < perf; i++) {
			double val = rndval();
			uint32_t dragon4tm = 0, grisu3tm = 0, errolNtm[5] = { 0, 0, 0, 0, 0 }, adj3tm = 0;

			qsort(dragon4all[i], N, sizeof(uint32_t), intsort);
			qsort(errolNall[0][i], N, sizeof(uint32_t), intsort);
			qsort(errolNall[1][i], N, sizeof(uint32_t), intsort);
			qsort(errolNall[2][i], N, sizeof(uint32_t), intsort);
			qsort(errolNall[3][i], N, sizeof(uint32_t), intsort);
			qsort(errolNall[4][i], N, sizeof(uint32_t), intsort);
			qsort(grisu3all[i], N, sizeof(uint32_t), intsort);
			qsort(adj3all[i], N, sizeof(uint32_t), intsort);

			for(j = Nlow; j < Nhigh; j++) {
				dragon4tm += dragon4all[i][j];
				errolNtm[0] += errolNall[0][i][j];
				errolNtm[1] += errolNall[1][i][j];
				errolNtm[2] += errolNall[2][i][j];
				errolNtm[3] += errolNall[3][i][j];
				errolNtm[4] += errolNall[4][i][j];
				grisu3tm += grisu3all[i][j];
				adj3tm += adj3all[i][j];
			}

			dragon4 += dragon4tm /= Nsize;
			errol[0] += errolNtm[0] /= Nsize;
			errol[1] += errolNtm[1] /= Nsize;
			errol[2] += errolNtm[2] /= Nsize;
			errol[3] += errolNtm[3] /= Nsize;
			errol[4] += errolNtm[4] /= Nsize;
			grisu3 += grisu3tm /= Nsize;
			adj3 += adj3tm /= Nsize;

			fprintf(stderr, "%.18e\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n", val, errolNtm[0], errolNtm[1], errolNtm[2], errolNtm[3], errolNtm[4], grisu3tm, dragon4tm, adj3tm);
		}

		printf("\x1b[G\x1b[KBenchmarking done,\n");
		printf("==== Absolute Results ====\n");
		printf("Errol0            %u cycles\n", errol[0] / perf);
		printf("Errol1            %u cycles\n", errol[1] / perf);
		printf("Errol2            %u cycles\n", errol[2] / perf);
		printf("Errol3            %u cycles\n", errol[3] / perf);
		printf("Errol4            %u cycles\n", errol[4] / perf);
		printf("Grisu3            %u cycles\n", grisu3 / perf);
		printf("Dragon4           %u cycles\n", dragon4 / perf);
		printf("Grisu3 w/fallback %u cycles\n", adj3 / perf);
		printf("==== Relative Speedup of Errol3 ====\n");
		printf("Grisu3            %.2fx\n", (double)grisu3 / (double)errol[3]);
		printf("Dragon4           %.2fx\n", (double)dragon4 / (double)errol[3]);
		printf("Grisu3 w/fallback %.2fx\n", (double)adj3 / (double)errol[3]);
		printf("==== Relative Speedup of Errol4 ====\n");
		printf("Grisu3            %.2fx\n", (double)grisu3 / (double)errol[4]);
		printf("Dragon4           %.2fx\n", (double)dragon4 / (double)errol[4]);
		printf("Grisu3 w/fallback %.2fx\n", (double)adj3 / (double)errol[4]);
	}

	if(enum3)
		table_enum(3, true);

	if(enum4)
		table_enum(4, true);

	if(check3)
		table_enum(3, false);

	if(check4)
		table_enum(4, false);

	return 0;
}


/**
 * Retrieve a long argument.
 *   @arg: The argument pointer.
 *   @pre: The option to match.
 *   @parameter: Optional. The parameter pointer.
 *   &returns: True if matched with argument incremented.
 */

static bool opt_long(char ***arg, const char *pre, char **param)
{
	char *str = **arg + 2;

	while(true) {
		if(*pre == '\0')
			break;

		if(*pre++ != *str++)
			return false;
	}

	if(param != NULL) {
		if(*str == '\0')
			*param = *(++(*arg));
		else if(*str == '=') {
			*param = str + 1;
		}
		else
			fprintf(stderr, "Option '--%s' expected argument.\n", **arg + 2), abort();
	}
	else {
		if(*str == '=')
			fprintf(stderr, "Unexpected argument to option '--%s'.\n", **arg + 2), abort();
		else if(*str != '\0')
			return false;
	}

	(*arg)++;

	return true;
}

/**
 * Retrieve an integer argument.
 *   @arg: The argument pointer.
 *   @pre: The option to match.
 *   @parameter: Optional. The parameter pointer.
 *   &returns: True if matched with argument incremented.
 */

static bool opt_num(char ***arg, const char *pre, int *num)
{
	unsigned long val;
	char *param, *endptr;

	if(!opt_long(arg, pre, &param))
		return false;

	errno = 0;
	val = strtol(param, &endptr, 0);
	if((val == 0) && (errno == 0))
		fprintf(stderr, "Parameter cannot be zero.\n"), abort();

	if((*endptr == 'k') || (*endptr == 'K'))
		val *= 1000, endptr++;
	else if((*endptr == 'm') || (*endptr == 'M'))
		val *= 1000000, endptr++;

	if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
		fprintf(stderr, "Invalid %s parameter '%s'.\n", pre, param), abort();
	else if(val > INT_MAX)
		fprintf(stderr, "Number too large '%s'.\n", param), abort();

	*num = val;

	return true;
}


/**
 * Sort integers.
 *   @left: The left pointer.
 *   @right: The right pointer.
 *   &returns: The order.
 */

static int intsort(const void *left, const void *right)
{
	if(*(const uint32_t *)left > *(const uint32_t *)right)
		return -1;
	else if(*(const uint32_t *)left < *(const uint32_t *)right)
		return 1;
	else
		return 0;
}

/**
 * Sort errol_err_t in terms of bit patterns of the floating points.
 *   @left: The left pointer.
 *   @right: The right pointer.
 *   &returns: The order.
 */

static int err_t_sort(const void *left, const void *right)
{
	errol_bits_t l = { ((struct errol_err_t *)left)->val };
	errol_bits_t r = { ((struct errol_err_t *)right)->val };

	if (l.i < r.i)
		return -1;
	else if (l.i == r.i)
		return 0;
	else
		return 1;
}


/**
 * Check a conversion.
 *   @val: The value.
 *   @str: The string.
 *   @exp: The exponent.
 *   @cor: The correct flag.
 *   @opt: The optimal flag.
 *   @best: The best flag.
 *   &returns: The actual value.
 */

static double chk_conv(double val, const char *str, int32_t exp, bool *cor, bool *opt, bool *best)
{
	double chk;
	int32_t dragon4exp;
	char dragon4[32], full[snprintf(NULL, 0, "0.%se%d", str, exp) + 1];

	dragon4exp = dragon4_proc(val, dragon4);

	sprintf(full, "0.%se%d", str, exp);
	sscanf(full, "%lf", &chk);

	*cor = (val == chk);
	*opt = (*cor && (strlen(str) == strlen(dragon4)));
	*best = (*opt && (exp == dragon4exp) && !strcmp(str, dragon4));

	return chk;
}


/**
 * Add to the table.
 *   @table: The table.
 *   @i: Insertion point.
 *   @val: The value.
 */

static void table_add(struct errol_err_t table[static 1024], int i, double val)
{
	table[i] = (struct errol_err_t){ val };
	table[i].exp = dragon4_proc(table[i].val, table[i].str);
}

/**
 * Process the enumeration algorithm.
 *   @ver: The version.
 *   @bld: Whether to build the table or check it.
 */

static void table_enum(unsigned int ver, bool bld)
{
	int i, e, n, p, exp, cnt = 0;
	int64_t *arr;
	mpz_t delta, m0, alpha, tau, t;
	struct errol_err_t table[1024] = {{ 0 }};
	static unsigned int D = 17, P = 52;

	assert((ver == 3) || (ver == 4));

	mpz_inits(delta, m0, alpha, tau, t, NULL);

	//for(e = -1022; e < 4; e++) {
	for(e = -1074; e <= 4; e++) {
		mpz_t t0, t1;

		/* bits of precision */
		p = (e >= -1022) ? P : (e + 1074);

		/* log10(5^{-e+p+1}2^{p+1}) - D + 2 */
		n = floor((-e+p+1)*log10(5.0) + (p+1)*log10(2.0)) - D + 2;

		/* common constants */
		mpz_inits(t0, t1, NULL);
		mpz_ui_pow_ui(t0, 5, -e+p+1-n);
		mpz_ui_pow_ui(t1, 2, P-1);

		/* Δ = 2^{1-P}5^{-e+p+1-n} */
		mpz_mul_ui(delta, t0, 79);

		/* α = 2*5^{-e+p+1-n}*/
		mpz_mul_ui(alpha, t0, 2);
		mpz_mul(alpha, alpha, t1);

		/* τ = 2^n */
		mpz_ui_pow_ui(tau, 2, n);
		mpz_mul(tau, tau, t1);

		/* m0 = 2^{p+1} 5^{-e+p+1-n} + 5^{e+p+1-n} */
		mpz_ui_pow_ui(t, 2, p+1);
		mpz_mul(m0, t0, t);
		mpz_add(m0, m0, t0);
		mpz_mul(m0, m0, t1);

		/*
		   gmp_printf("e  %d\n", e);
		   gmp_printf("p  %d\n", p);
		   gmp_printf("n  %u\n", n);
		   gmp_printf("α  %Zd\n", alpha);
		   gmp_printf("Δ  %Zd\n", delta);
		   gmp_printf("τ  %Zd\n", tau);
		   gmp_printf("m0 %Zd\n", m0);
		   */

		/* find possible failures */
		arr = proof_enum(delta, m0, alpha, tau, p);

		for(i = 0; arr[i] >= 0; i++) {
			double v;

			v = ldexp(1.0, e) + ldexp(arr[i], e-52);
			if(!(bld ? errolNu_check : errolN_check)(ver, v))
				table_add(table, cnt++, v);

			v = ldexp(1.0, e) + ldexp(arr[i]+1, e-52);
			if(!(bld ? errolNu_check : errolN_check)(ver, v))
				table_add(table, cnt++, v);
		}

		free(arr);

		mpz_clears(t0, t1, NULL);
	}

	for(int e = 128; e <= 1023; e++) {
		unsigned int p = P;

		/* log10(2^{e+1}) - D + 2 */
		n = floor((e+1.0)*log10(2.0)) - D + 2;

		/* Δ = 2^(e-p-n) 79ϵ = 79 2^{e-2p-n) */
		exp = e - 2*p - n;
		mpz_ui_pow_ui(delta, 2, (exp > 0) ? exp : 0);
		mpz_mul_ui(delta, delta, 179);

		/* α = 2^(e-p-n) */
		mpz_ui_pow_ui(alpha, 2, e - p - n);

		/* τ = 5^n */
		mpz_ui_pow_ui(tau, 5, n);

		/* m0 = 2^(e-n) + 2^(e-p-1-n) */
		mpz_ui_pow_ui(m0, 2, e-n);
		mpz_ui_pow_ui(t, 2, e-p-n-1);
		mpz_add(m0, m0, t);

		/*
		   gmp_printf("e  %d\n", e);
		   gmp_printf("p  %d\n", p);
		   gmp_printf("n  %u\n", n);
		   gmp_printf("α  %Zd\n", alpha);
		   gmp_printf("Δ  %Zd\n", delta);
		   gmp_printf("τ  %Zd\n", tau);
		   */

		/* find possible failures */
		arr = proof_enum(delta, m0, alpha, tau, p);

		for(i = 0; arr[i] >= 0; i++) {
			double v;

			v = ldexp(1.0, e) + ldexp(arr[i], e-52);
			if(!(bld ? errolNu_check : errolN_check)(ver, v))
				table_add(table, cnt++, v);

			v = ldexp(1.0, e) + ldexp(arr[i]+1, e-52);
			if(!(bld ? errolNu_check : errolN_check)(ver, v))
				table_add(table, cnt++, v);
		}

		free(arr);
	}

	mpz_clears(delta, m0, alpha, tau, t, NULL);

	if(!(bld ? errolNu_check : errolN_check)(ver, DBL_MIN))
		table_add(table, cnt++, DBL_MIN);

	if(!(bld ? errolNu_check : errolN_check)(ver, DBL_MAX))
		table_add(table, cnt++, DBL_MAX);

	if(bld) {
		FILE *file;

		qsort(table, cnt, sizeof(struct errol_err_t), err_t_sort);
		table_to_tree(table, cnt);

		file = fopen((ver == 3) ? "enum3.h" : "enum4.h", "w");
		fprintf(file, "static uint64_t errol_enum%d[%d] = {\n", ver, cnt);

		for(i = 0; i < cnt; i++) {
			errol_bits_t bits = { table[i].val };
			fprintf(file, "\t%#.16" PRIx64 ",\n", bits.i);
		}

		fprintf(file, "};\n");
		fprintf(file, "static struct errol_slab_t errol_enum%d_data[%d] = {\n", ver, cnt);

		for(i = 0; i < cnt; i++) {
			fprintf(file, "\t{ \"%s\", %d },\n", table[i].str, table[i].exp);
		}

		fprintf(file, "};\n");
		fclose(file);
	}
	else {
		for(i = 0; i < 1024; i++) {
			if(table[i].val != 0.0)
				printf("Failed value %.17e\n", table[i].val);

			if(table[i].val != 0.0)
				printf("Failed value %.17e\n", table[i].val);
		}
	}

	printf("Enumerating Errol%u%s, %u failures\n", ver, bld ? "u" : "", cnt);
}

/**
 * Calculate the index of the root node when reordering a sorted array to
 * the level-order of a binary search tree.
 *   @n: Array size.
 *   &returns: The index.
 */

static inline int table_root(int n)
{
	if (n <= 1)
		return 0;
	int i = 2;
	while (i <= n)
		i *= 2;
	int a = i / 2 - 1;
	int b = n - i / 4;
	return a < b ? a : b;
}

/**
 * Insert all elements in a sorted array to another array in level-order.
 *   @table: The destination.
 *   @i: Insertion point.
 *   @from: The source.
 *   @n: Array size.
 */

static void table_to_tree_iter(struct errol_err_t *table, int i,
                               struct errol_err_t *from, int n)
{
	if (n > 0)
	{
		int h = table_root(n);
		table[i] = from[h];
		table_to_tree_iter(table, 2 * i + 1, from, h);
		table_to_tree_iter(table, 2 * i + 2, from + h + 1, n - h - 1);
	}
}

/**
 * Reorder a sorted array to the level-order of a binary search tree.
 *   @table: The input array.
 *   @n: Array size.
 */

static void table_to_tree(struct errol_err_t *table, int n)
{
	struct errol_err_t from[1024];
	memcpy(from, table, n * sizeof(struct errol_err_t));
	table_to_tree_iter(table, 0, from, n);
}
