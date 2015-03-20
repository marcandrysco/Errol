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

/*
 * local function declarations
 */

static bool opt_long(char ***arg, const char *pre, char **value);

static void rndinit();
static double rndval();


/**
 * Main entry point.
 *   @argc: The number of arguments.
 *   @argv: The argument array.
 *   &returns: The error code.
 */

int main(int argc, char **argv)
{
	unsigned long val;
	unsigned int i, fuzz = 0, bench = 0;
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
			else if(opt_long(&arg, "fuzz", &value)) {
				errno = 0;
				val = strtol(value, &endptr, 0);

				if((*endptr == 'k') || (*endptr == 'K'))
					val *= 1000, endptr++;
				else if((*endptr == 'm') || (*endptr == 'M'))
					val *= 1000000, endptr++;

				if(((val == 0) && (errno != 0)) || (*endptr != '\0'))
					fprintf(stderr, "Invalid fuzz parameter '%s'.\n", value), abort();
				else if(val > UINT_MAX)
					fprintf(stderr, "Number too large '%s'.\n", value);

				fuzz = val;
			}
			else if(opt_long(&arg, "bench", &value)) {
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

				bench = val;
			}
			else
				fprintf(stderr, "Invalid option '%s'.\n", *arg), abort();
		}
		else
			fprintf(stderr, "Invalid option '%s'.\n", *arg), abort();
	}

	if(fuzz > 0) {
		unsigned int nfail = 0, subopt = 0;

		for(i = 0; i < fuzz; i++) {
			if((i % 1000) == 0) {
				printf("\x1b[G\x1b[KFuzzing... %uk/%uk %2.2f%%", i/1000, fuzz/1000, 100.0 * (double)i / (double)fuzz);
				fflush(stdout);
			}

			{
				int exp;
				char buf[108], fmt[108];
				double val, chk;
				bool opt;

				val = rndval();
				exp = errol1_dtoa(val, buf, &opt);
				sprintf(fmt, "0.%se%d\n", buf, exp);
				sscanf(fmt, "%lf", &chk);

				if(!opt)
					subopt++;

				if(chk != val) {
					nfail++;

					if(1)
						fprintf(stderr, "buf: %s, exp: %d\n", buf, exp);

					fprintf(stderr, "fmt: %s\n", fmt);
					fprintf(stderr, "Conversion failed. Expected %.17e. Actual %.17e.\n", val, chk);
				}
			}
		}

		printf("\x1b[G\x1b[KFuzzing done, %u failures (%.3f%%), %u suboptimal (%.3f%%)\n", nfail, 100.0 * (double)nfail / (double)fuzz, subopt, 100.0 * (double)subopt / (double)fuzz);
	}

	if(bench > 0) {
	}

	return 0;
}


/**
 * Retrieve a long argument.
 *   @arg: The argument pointer.
 *   @opt: The option to match.
 *   @parameter: Optional. The paratmer pointer.
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
			fprintf(stderr, "Option '--%s' expected argument.\n", pre), abort();
	}
	else {
		if(*str == '=')
			fprintf(stderr, "Unexpected argument to option '--%s'.\n", pre), abort();
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
	} while(isnan(val.d) || (val.d == 0.0) || !isfinite(val.d) || (val.d < 1e-190));

	return val.d;
}
