#include <double-conversion/fast-dtoa.h>
#include <errol.h>
#include "dragon4.h"
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <random>
#include <immintrin.h>

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

	__asm__ __volatile__(
			"rdtscp;"
			: "=a" (a), "=d" (d)
			:
			: "%rcx", "%rbx", "memory");

	return ((uint64_t)a) | (((uint64_t)d) << 32);
}


/**
 * Retrieve a 64-bit seed from CPU.
 *   &returns: An integer with an unpredictable value.
 */

extern "C" uint_fast64_t get_seed()
{
#if defined(__RDRND__)
	unsigned long long v;
	_rdrand64_step(&v);
	return v;
#else
	return rdtsc() ^ (uint_fast64_t(std::random_device{}()) << 32);
#endif
}


/**
 * Initialize the global random engine.
 *   &returns: The engine.
 */

inline std::mt19937_64& global_rng()
{
	static std::mt19937_64 e{ get_seed() };
	return e;
}


/**
 * Create a random double value. The random value is always positive, non-zero,
 * not infinity, and non-NaN.
 *   &returns: The random double.
 */

extern "C" double rndval()
{
	union
	{
		double d;
		uint64_t i;
	} val;

	do
		val.i = global_rng()() & ~(0x8000000000000000);
	while ((val.d == 0.0) || !std::isfinite(val.d));
	return val.d;
}


/**
 * Seed the global random engine.
 *   @value: The seed to use.
 */

extern "C" void reseed(uint_fast64_t value)
{
	global_rng().seed(value);
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
 * Benchmark decimal to string using Grisu.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" int grisu3_proc(double val, char *buf, bool *suc)
{
	bool chk;
	Vector<char> buffer(buf, 100);
	int length, point;

	chk = FastDtoa(val, FAST_DTOA_SHORTEST, 0, buffer, &length, &point);
	if(suc)
		*suc = chk;

	return point;
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

extern "C" int dragon4_proc(double val, char *buf)
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
 * Check Errol.
 *   @n: The version.
 *   @val: The value.
 *   &returns: True if correct, false otherwise.
 */

extern "C" bool errolN_check(unsigned int n, double val)
{
	bool opt;
	char errol[32], dragon[32];

	switch(n) {
	case 0: errol1_dtoa(val, errol, &opt); break;
	case 1: errol1_dtoa(val, errol, &opt); break;
	case 2: errol2_dtoa(val, errol, &opt); break;
	case 3: errol3_dtoa(val, errol); break;
	case 4: errol4_dtoa(val, errol); break;
	}

	dragon4_proc(val, dragon);

	return strcmp(errol, dragon) == 0;
}

/**
 * Check uncorrected Errol.
 *   @n: The version.
 *   @val: The value.
 *   &returns: True if correct, false otherwise.
 */

extern "C" bool errolNu_check(unsigned int n, double val)
{
	char errol[32], dragon[32];

	switch(n) {
	case 3: errol3u_dtoa(val, errol); break;
	case 4: errol4u_dtoa(val, errol); break;
	}

	dragon4_proc(val, dragon);

	return strcmp(errol, dragon) == 0;
}

/**
 * Process Errol.
 *   @n: The version.
 *   @val: The value.
 *   @buf: The output buffer.
 *   @opt: The optimality flag.
 *   &returns: The exponent.
 */

extern "C" int errolN_proc(unsigned int n, double val, char *buf, bool *opt)
{
	*opt = true;

	switch(n) {
	case 0: return errol1_dtoa(val, buf, opt);
	case 1: return errol1_dtoa(val, buf, opt);
	case 2: return errol2_dtoa(val, buf, opt);
	case 3: return errol3_dtoa(val, buf);
	case 4: return errol4_dtoa(val, buf);
	}

	assert(false);

	return 0;
}

/**
 * Benchmark Errol.
 *   @val: The value.
 *   @suc: The success flag.
 *   &returns: The number of cycles.
 */

extern "C" uint32_t errolN_bench(unsigned int n, double val, bool *suc)
{
	uint64_t tm;
	char buf[100];
	bool opt = true;

	switch(n) {
	case 0:
		tm = rdtsc();
		errol0_dtoa(val, buf);
		tm = rdtsc() - tm;
		break;

	case 1:
		tm = rdtsc();
		errol1_dtoa(val, buf, &opt);
		tm = rdtsc() - tm;
		break;

	case 2:
		tm = rdtsc();
		errol2_dtoa(val, buf, &opt);
		tm = rdtsc() - tm;
		break;

	case 3:
		tm = rdtsc();
		errol3_dtoa(val, buf);
		tm = rdtsc() - tm;
		break;

	case 4:
		tm = rdtsc();
		errol4_dtoa(val, buf);
		tm = rdtsc() - tm;
		break;

	default:
		assert(true);
		tm = 0;
	}

	if(suc)
		*suc = opt;

	return tm;
}
