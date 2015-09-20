#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "../lib/errol.h"
#include <gmp.h>


#define DEBUG 0
#define ERRPATH "err.list"


struct shift_t {
	uint64_t idx;
	mpz_t val;
};

struct list_t {
	struct shift_t *arr;
	unsigned int len;
};


struct list_t list_init(void)
{
	return (struct list_t){ malloc(0), 0 };
}

void list_destroy(struct list_t *list)
{
	unsigned int i;

	for(i = 0; i < list->len; i++)
		mpz_clear(list->arr[i].val);

	free(list->arr);
}

/**
 * Add a shift to the list.
 *   @list: The list.
 *   @idx: The index.
 *   @val: The value.
 */

void list_add(struct list_t *list, uint64_t idx, mpz_t val)
{
	unsigned int n = list->len++;

	list->arr = realloc(list->arr, list->len * sizeof(struct shift_t));
	list->arr[n].idx = idx;
	mpz_init_set(list->arr[n].val, val);
}

/**
 * Retrieve the last shift from the list.
 *   @list: The list.
 *   &returns: The last shift.
 */

struct shift_t *list_last(struct list_t *list)
{
	return &list->arr[list->len - 1];
}

bool list_contains(struct list_t *list, uint64_t idx)
{
	unsigned int i;

	for(i = 0; i < list->len; i++) {
		if(list->arr[i].idx == idx)
			return true;
	}

	return false;
}

void list_search(uint64_t idx, mpz_t v, struct list_t *out, struct list_t *up, struct list_t *down, mpz_t delta)
{
	mpz_t t;
	unsigned int i;

	if(idx > 9007199254740992)
		return;
	if(list_contains(out, idx))
		return;

	mpz_init(t);

	list_add(out, idx, v);

	for(i = up->len - 1; i != UINT_MAX; i--) {
		mpz_add(t, v, up->arr[i].val);
		if(mpz_cmpabs(t, delta) <= 0)
			list_search(idx + up->arr[i].idx, t, out, up, down, delta);
		else
			break;
	}

	for(i = down->len - 1; i != UINT_MAX; i--) {
		mpz_add(t, v, down->arr[i].val);
		if(mpz_cmpabs(t, delta) <= 0)
			list_search(idx + down->arr[i].idx, t, out, up, down, delta);
		else
			break;
	}

	mpz_clear(t);
}


/*
 * interop function declarations
 */

uint32_t grisu_bench(double val, bool *suc);

uint32_t dragon4_bench(double val, bool *suc);
int32_t dragon4_proc(double val, char *buf);
uint32_t dragon4_len(double val);

uint32_t errol1_bench(double val, bool *suc);
uint32_t errol2_bench(double val, bool *suc);
uint32_t errol3_bench(double val, bool *suc);
bool errol3_check(double val);

/*
 * local function declarations
 */

static bool opt_long(char ***arg, const char *pre, char **value);

static void rndinit();
static double rndval();

static int intsort(const void *left, const void *right);


/**
 * Main entry point.
 *   @argc: The number of arguments.
 *   @argv: The argument array.
 *   &returns: The error code.
 */

int main(int argc, char **argv)
{
	bool quiet = false, errenum = false;
	unsigned long val;
	unsigned int i, fuzz0 = 0, fuzz1 = 0, fuzz3 = 0;
	unsigned int bench1 = 0, bench2 = 0, bench3 = 0, perf = 0;
	char **arg, *value, *endptr;

	rndinit();

	for(arg = argv + 1; *arg != NULL; ) {
		if((*arg)[0] == '-') {
			if((*arg)[1] != '-') {
				char *opt = *arg + 2;

				while(*opt != '\0') {
					opt++;
				}

				arg++;
			}
			else if(opt_long(&arg, "enum", NULL))
				errenum = true;
			else if(opt_long(&arg, "fuzz0", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid fuzz0 parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				fuzz0 = val;
			}
			else if(opt_long(&arg, "fuzz1", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid fuzz1 parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				fuzz1 = val;
			}
			else if(opt_long(&arg, "fuzz3", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid fuzz1 parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				fuzz3 = val;
			}
			else if(opt_long(&arg, "bench1", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid bench parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				bench1 = val;
			}
			else if(opt_long(&arg, "bench2", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid bench parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				bench2 = val;
			}
			else if(opt_long(&arg, "bench3", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid bench parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				bench3 = val;
			}
			else if(opt_long(&arg, "perf", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid bench parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				perf = val;
			}
			else
				fprintf(stderr, "Invalid option '%s'.\n", *arg), abort();
		}
		else
			fprintf(stderr, "Invalid option '%s'.\n", *arg), abort();
	}

	if(fuzz0 > 0) {
		unsigned int nfail = 0, subopt = 0;

		for(i = 0; i < fuzz0; i++) {
			int exp;
			char buf[32], fmt[32];
			double val, chk;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KFuzzing Errol0... %uk/%uk %2.2f%%", i / 1000, fuzz0 / 1000, 100.0 * (double)i / (double)fuzz0);
				fflush(stdout);
			}

			val = rndval();
			exp = errol0_dtoa(val, buf);
			sprintf(fmt, "0.%se%d", buf, exp);
			sscanf(fmt, "%lf", &chk);

			if(chk != val) {
				nfail++;

				if(!quiet)
					fprintf(stderr, "Conversion failed. Expected %.17e. Actual %.17e.\n", val, chk);
			}

			if(strlen(buf) != dragon4_len(val))
				subopt++;
		}

		printf("\x1b[G\x1b[KFuzzing Errol0 done on %u numbers, %u failures (%.3f%%), %u suboptimal (%.3f%%)\n", fuzz0, nfail, 100.0 * (double)nfail / (double)fuzz0, subopt, 100.0 * (double)subopt / (double)fuzz0);
	}

	if(fuzz1 > 0) {
		unsigned int nfail = 0, subopt = 0;

		for(i = 0; i < fuzz1; i++) {
			int exp;
			char buf[32], fmt[32];
			double val, chk;
			bool opt;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KFuzzing Errol1... %uk/%uk %2.2f%%", i / 1000, fuzz1 / 1000, 100.0 * (double)i / (double)fuzz1);
				fflush(stdout);
			}

			val = rndval();
			exp = errol1_dtoa(val, buf, &opt);
			sprintf(fmt, "0.%se%d", buf, exp);
			sscanf(fmt, "%lf", &chk);

			if(chk != val) {
				nfail++;

				if(!quiet)
					fprintf(stderr, "\x1b[G\x1b[KConversion failed. Expected %.17e. Actual %.17e.\n", val, chk);
			}

			if(strlen(buf) != dragon4_len(val)) {
				if(opt) {
					nfail++;

					if(!quiet)
						fprintf(stderr, "\x1b[G\x1b[KOptimaily check failed. Value %.17e.\n", chk);
				}

				subopt++;
			}
		}

		printf("\x1b[G\x1b[KFuzzing Errol1 done on %u numbers, %u failures (%.3f%%), %u suboptimal (%.3f%%)\n", fuzz1, nfail, 100.0 * (double)nfail / (double)fuzz1, subopt, 100.0 * (double)subopt / (double)fuzz1);
	}

	if(fuzz3 > 0) {
		unsigned int nfail = 0;

		for(i = 0; i < fuzz3; i++) {
			int exp;
			char buf[32], fmt[32];
			double val, chk;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KFuzzing Errol3... %uk/%uk %2.2f%%", i / 1000, fuzz3 / 1000, 100.0 * (double)i / (double)fuzz3);
				fflush(stdout);
			}

			val = rndval();
			exp = errol3_dtoa(val, buf);
			sprintf(fmt, "0.%se%d", buf, exp);
			sscanf(fmt, "%lf", &chk);

			if(chk != val) {
				nfail++;

				if(!quiet)
					fprintf(stderr, "\x1b[G\x1b[KConversion failed. Expected %.17e. Actual %.17e.\n", val, chk);
			}

			if(strlen(buf) != dragon4_len(val)) {
				char dbuf[32];
				int dexp;

				nfail++;

				dexp = dragon4_proc(val, dbuf);

				if(!quiet)
					fprintf(stderr, "\x1b[G\x1b[KOptimaily check failed. Expected 0.%se%d, Actual %se%d.\n", dbuf, dexp, buf, exp);
			}
		}

		printf("\x1b[G\x1b[KFuzzing Errol1 done on %u numbers, %u failures (%.3f%%)\n", fuzz3, nfail, 100.0 * (double)nfail / (double)fuzz3);
	}

	if(bench1 > 0) {
		uint32_t grisu = 0, errol1 = 0;

		for(i = 0; i < bench1; i++) {
			double val;
			uint32_t tm;
			bool suc;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KBenchmarking... %uk/%uk %2.2f%%", i / 1000, bench1 / 1000, 100.0 * (double)i / (double)bench1);
				fflush(stdout);
			}

			val = rndval();

			tm = grisu_bench(val, &suc);
			grisu += tm;

			tm = errol1_bench(val, &suc);
			errol1 += tm;
		}

		printf("\x1b[G\x1b[KBenchmarking done, Errol1 %u (%.2f), Grisu3 %u\n", errol1, (double)grisu / (double)errol1, grisu);
	}

	if(bench2 > 0) {
		uint32_t grisu = 0, errol2 = 0;

		for(i = 0; i < bench2; i++) {
			double val;
			uint32_t tm;
			bool suc;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KBenchmarking... %uk/%uk %2.2f%%", i / 1000, bench2 / 1000, 100.0 * (double)i / (double)bench2);
				fflush(stdout);
			}

			val = rndval();

			tm = grisu_bench(val, &suc);
			grisu += tm;

			tm = errol2_bench(val, &suc);
			errol2 += tm;
		}

		printf("\x1b[G\x1b[KBenchmarking done, Errol1 %u (%.2f), Grisu3 %u\n", errol2, (double)grisu / (double)errol2, grisu);
	}

	if(bench3 > 0) {
		uint32_t grisu3 = 0, errol3 = 0;

		for(i = 0; i < bench3; i++) {
			double val;
			uint32_t tm;
			bool suc;

			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KBenchmarking... %uk/%uk %2.2f%%", i / 1000, bench3 / 1000, 100.0 * (double)i / (double)bench3);
				fflush(stdout);
			}

			val = rndval();

			tm = grisu_bench(val, &suc);
			grisu3 += tm;

			tm = errol3_bench(val, &suc);
			errol3 += tm;
		}

		printf("\x1b[G\x1b[KBenchmarking done, Errol3 %u (%.2f), Grisu3 %u\n", errol3, (double)grisu3 / (double)errol3, grisu3);
	}

	if(perf > 0) {
		uint32_t dragon4 = 0, grisu3 = 0, errol3 = 0, adj3 = 0;

#define N	100
#define Nlow	(N / 10)
#define Nhigh	(N - Nlow)
#define Nsize	(Nhigh - Nlow)
		static uint32_t dragon4all[20000][N], grisu3all[20000][N], errol3all[20000][N], adj3all[20000][N];
		unsigned int i, j;

		for(j = 0; j < N; j++) {
			srand(10);
			for(i = 0; i < perf; i++) {
				double val;
				bool suc;

				//if((i % 100) == 0) {
					//printf("\x1b[G\x1b[KPerformance testing... %u/%u %2.2f%%", i, perf, 100.0 * (double)i / (double)perf);
					//fflush(stdout);
				//}

				val = rndval();

				dragon4all[i][j] = dragon4_bench(val, &suc);
				errol3all[i][j] = errol3_bench(val, &suc);
				grisu3all[i][j] = grisu_bench(val, &suc);
				adj3all[i][j] = suc ? grisu3all[i][j] : dragon4all[i][j];
			}
		}

		srand(10);

		for(i = 0; i < perf; i++) {
			double val = rndval();
			uint32_t dragon4tm = 0, grisu3tm = 0, errol3tm = 0, adj3tm = 0;

			qsort(dragon4all[i], N, sizeof(uint32_t), intsort);
			qsort(errol3all[i], N, sizeof(uint32_t), intsort);
			qsort(grisu3all[i], N, sizeof(uint32_t), intsort);
			qsort(adj3all[i], N, sizeof(uint32_t), intsort);

			for(j = Nlow; j < Nhigh; j++) {
				dragon4tm += dragon4all[i][j];
				errol3tm += errol3all[i][j];
				grisu3tm += grisu3all[i][j];
				adj3tm += adj3all[i][j];
			}

			dragon4 += dragon4tm /= Nsize;
			errol3 += errol3tm /= Nsize;
			grisu3 += grisu3tm /= Nsize;
			adj3 += adj3tm /= Nsize;


			fprintf(stderr, "%.18e\t%u\t%u\t%u\t%u\n", val, errol3tm, grisu3tm, dragon4tm, adj3tm);
		}

		printf("\x1b[G\x1b[KBenchmarking done, Errol1 %u (%.2f, %.2f), Grisu3 %u, Grisu3adj %u\n", errol3, (double)grisu3 / (double)errol3, (double)adj3 / (double)errol3, grisu3, adj3);
	}

	if(errenum) {
		int e;
		unsigned int i, n;
		unsigned int D = 17;
		uint64_t idx;
		mpz_t delta, alpha, tau, v, t;
		struct list_t up, down;

		if(access(ERRPATH, F_OK) >= 0)
			remove(ERRPATH);

		mpz_inits(delta, alpha, tau, v, t, NULL);

		for(e = -1074; e < 1023; e++) {
			unsigned int p = 52;

			if(e != -12) continue;

			if(e < 53) {
				// n = ⌊(e+1)log10(2)⌋ 
				n = floor((e+1.0)*log10(2.0)) - D + 2;
				printf("n = %u\n", n);
			}
			else if(e >= 128) {
				printf("#### e = %d ####\n", e);

				// n = ⌊(e+1)log10(2)⌋ 
				n = floor((e+1.0)*log10(2.0)) - D + 2;
				printf("n = %u\n", n);

				// Δ = 2^(e-p-n) 79ϵ = 79 2^{e-2p-n)
				mpz_ui_pow_ui(delta, 2, e - 2*p - n);
				mpz_mul_ui(delta, delta, 79);
				if(DEBUG) gmp_printf("delta: %Zd\n", delta);

				// α = 2^(e-p-n)
				mpz_ui_pow_ui(alpha, 2, e - p - n);
				if(DEBUG) gmp_printf("alpha: %Zd\n", alpha);

				// tau = 5^n
				mpz_ui_pow_ui(tau, 5, n);
				if(DEBUG) gmp_printf("tau: %Zd\n", tau);

				up = list_init();
				down = list_init();

				// u0 = M+(α, tau)
				mpz_mod(v, alpha, tau);
				list_add(&up, 1, v);
				if(DEBUG) gmp_printf("u1: %Zd\n", v);

				// d0 = M-(α, tau)
				mpz_sub(v, v, tau);
				list_add(&down, 1, v);
				if(DEBUG) gmp_printf("d1: %Zd\n", v);

				while(true) {
					if(list_last(&up)->idx <= list_last(&down)->idx) {
						for(i = 0; i < down.len; i++) {
							mpz_add(v, list_last(&up)->val, down.arr[i].val);
							if(mpz_cmp(v, list_last(&down)->val) > 0)
								break;
						}

						if(i == down.len)
							fprintf(stderr, "invalid shift\n"), abort();

						uint64_t next = list_last(&up)->idx + down.arr[i].idx;
						if(next >= 9007199254740992)
							break;

						if(mpz_sgn(v) >= 0)
							list_add(&up, next, v);
						//gmp_printf("u%u: %Zd\n", next, v);

						if(mpz_sgn(v) <= 0)
							list_add(&down, next, v);
						//gmp_printf("d%u: %Zd\n", next, v);
					}
					else {
						for(i = 0; i < up.len; i++) {
							mpz_add(v, list_last(&down)->val, up.arr[i].val);
							if(mpz_cmp(v, list_last(&up)->val) < 0)
								break;
						}

						if(i == up.len)
							fprintf(stderr, "invalid shift\n"), abort();

						uint64_t next = list_last(&down)->idx + up.arr[i].idx;
						if(next >= 9007199254740992)
							break;

						if(mpz_sgn(v) >= 0)
							list_add(&up, next, v);
						//gmp_printf("u%u: %Zd\n", next, v);

						if(mpz_sgn(v) <= 0)
							list_add(&down, next, v);
						//gmp_printf("d%u: %Zd\n", next, v);
					}
				}

				// m0 = 2^(e-n) + 2^(e-p-1-n)

				idx = 0;
				mpz_ui_pow_ui(v, 2, e - n + 1);
				mpz_add(v, v, alpha);
				mpz_divexact_ui(v, v, 2);
				mpz_mod(v, v, tau);

				while(true) {
					if(DEBUG) gmp_printf("m%llu: %Zd\n", idx, v);

					if(mpz_cmpabs(v, delta) <= 0)
						break;

					if(mpz_sgn(v) >= 0) {
						for(i = 0; i < down.len; i++) {
							mpz_add(t, v, down.arr[i].val);
							if(mpz_cmpabs(t, v) < 0)
								break;
						}

						if(i == down.len)
							gmp_printf("invalid state\n"), abort();

						idx += down.arr[i].idx;
						mpz_set(v, t);
					}
					else {
						for(i = 0; i < up.len; i++) {
							mpz_add(t, v, up.arr[i].val);
							if(mpz_cmpabs(t, v) < 0)
								break;
						}

						if(i == up.len)
							gmp_printf("invalid state\n"), abort();

						idx += up.arr[i].idx;
						mpz_set(v, t);
					}
				}

				if(mpz_cmpabs(v, delta) <= 0) {
					struct list_t fnd;
					FILE *file;

					fnd = list_init();
					file = fopen(ERRPATH, "a");

					list_search(idx, v, &fnd, &up, &down, delta);

					for(i = 0; i < fnd.len; i++) {
						double flt;
						char buf[10*1024];

						if(DEBUG) gmp_printf("%llu: %Zd\n", fnd.arr[i].idx, fnd.arr[i].val);

						mpz_ui_pow_ui(v, 2, e);
						mpz_ui_pow_ui(t, 2, e-p);
						mpz_mul_ui(t, t, fnd.arr[i].idx);
						mpz_add(v, v, t);
						gmp_sprintf(buf, "%Zd\n", v);
						sscanf(buf, "%lf", &flt);
						if(!errol3_check(flt))
							fprintf(file, "%.17g\n", flt),
								printf("%u:%.17g!!!!\n", i, flt);

						mpz_ui_pow_ui(t, 2, e-p);
						mpz_add(v, v, t);
						gmp_sprintf(buf, "%Zd\n", v);
						sscanf(buf, "%lf", &flt);
						if(!errol3_check(flt))
							fprintf(file, "%.17g\n", flt),
								printf("%u:%.17g!!!!\n", i, flt);
					}

					fclose(file);
					list_destroy(&fnd);
				}
			}

			list_destroy(&up);
			list_destroy(&down);
		}

		mpz_clears(delta, alpha, tau, v, t, NULL);
	}

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
 * Initialize the random number generator.
 */

static void rndinit()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	srand(1000000 * (int64_t)tv.tv_sec + (int64_t)tv.tv_usec);
}

/**
 * Create a random double value. The random value is always positive, non-zero, not
 * infinity, and non-NaN.
 *   &returns: The random double.
 */

static double rndval()
{
	union {
		double d;
		uint16_t arr[4];
	} val;

	do {
		val.arr[0] = rand();
		val.arr[1] = rand();
		val.arr[2] = rand();
		val.arr[3] = rand() & ~(0x8000);
	} while(isnan(val.d) || (val.d == 0.0) || !isfinite(val.d));

	return val.d;
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
