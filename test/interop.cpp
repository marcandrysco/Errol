#include "../grisu/src/fast-dtoa.h"
#include "../lib/errol.h"
#include "dragon4.h"
#include <stdio.h>
#include <string.h>

/*
 * namespaces
 */

using namespace double_conversion;


/**
 * Read the time source clock.
 *   &returns: The source clock as a 64-bit unsignd integer.
 */

static inline uint64_t rdtsc()
{
	uint32_t a, d;

	__asm__ __volatile__("cpuid;"
			"rdtsc;"
			: "=a" (a), "=d" (d)
			:
			: "%rcx", "%rbx", "memory");

	return ((uint64_t)a) | (((uint64_t)d) << 32);
}


/**
 * Benchmark decimal to string using Grisu.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" uint32_t grisu_bench(double val, bool *suc)
{
	bool chk;
	static const int kBufferSize = 100;
	char buf[kBufferSize];
	Vector<char> buffer(buf, kBufferSize);
	int length, point;
	uint64_t tm;

	tm = rdtsc();
	chk = FastDtoa(val, FAST_DTOA_SHORTEST, 0, buffer, &length, &point);
	tm = rdtsc() - tm;

	if(suc)
		*suc = chk;

	return tm;
}


/**
 * Benchmark decimal to string using Dragon4.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" uint32_t dragon4_bench(double val, bool *suc)
{
	uint64_t tm;
	char *str;
	int pt, sign;

	tm = rdtsc();
	str = dtoa(val, 0, 24, &pt, &sign, NULL);
	tm = rdtsc() - tm;

	freedtoa(str);

	if(suc)
		*suc = true;

	return tm;
}

/**
 * Process the drangon4 algorithm.
 *   @val: The double value.
 *   @buf: The output buffer.
 *   &returns: The exponent.
 */

extern "C" int32_t dragon4_proc(double val, char *buf)
{
	char *str;
	int pt, sign;

	str = dtoa(val, 0, 24, &pt, &sign, NULL);
	strcpy(buf, str);
	freedtoa(str);

	return pt;
}

/**
 * Check the length of the dragon4 algorithm.
 *   @val: The double value.
 *   &returns: The length, guaranteed to be shortest.
 */

extern "C" uint32_t dragon4_len(double val)
{
	char buf[32];

	dragon4_proc(val, buf);

	return strlen(buf);
}


/**
 * Benchmark Errol1 decimal to string conversion.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" uint32_t errol1_bench(double val, bool *suc)
{
	uint64_t tm;
	bool opt;
	char buf[100];

	tm = rdtsc();
	errol1_dtoa(val, buf, &opt);
	tm = rdtsc() - tm;

	if(suc)
		*suc = opt;

	return tm;
}

/**
 * Benchmark Errol2 decimal to string conversion.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" uint32_t errol2_bench(double val, bool *suc)
{
	uint64_t tm;
	bool opt;
	char buf[100];

	tm = rdtsc();
	errol2_dtoa(val, buf, &opt);
	tm = rdtsc() - tm;

	if(suc)
		*suc = opt;

	return tm;
}

/**
 * Benchmark Errol3 decimal to string conversion.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" uint32_t errol3_bench(double val, bool *suc)
{
	uint64_t tm;
	char buf[100];

	tm = rdtsc();
	errol3_dtoa(val, buf);
	tm = rdtsc() - tm;

	if(suc)
		*suc = true;

	return tm;
}

/**
 * Check Errol1 decimal to string convesion.
 *   @val: The value.
 *   @suc: The success flag. Set if the algorithm indicates optimal output.
 *   @opt: The optimality flag. Set if the output is optimal.
 *   &returns: True if conversion is correct, false otherwise.
 */

extern "C" bool errol1_check(double val, bool *suc, bool *opt)
{
	double chk;
	int32_t exp;
	char buf[100], fmt[100];

	exp = errol1_dtoa(val, buf, opt);
	sprintf(fmt, "0.%se%d", buf, exp);
	sscanf(fmt, "%lf", &chk);

	return opt;
}

/**
 * Check Errol1 decimal to string convesion.
 *   @val: The value.
 *   &returns: True if conversion is correct, false otherwise.
 */

extern "C" bool errol3_check(double val)
{
	double chk;
	int32_t exp;
	char buf[100], fmt[100];

	exp = errol3_dtoa(val, buf);
	sprintf(fmt, "0.%se%d", buf, exp);
	sscanf(fmt, "%lf", &chk);

	return val == chk;
}
