#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errol.h"


/**
 * Function to check if string is all zeroes.
 *   &returns: True if all zeros, false otherwise.
 */

static char allzero(const char *str)
{
	while(*str == '0')
		str++;

	return *str == '\0';
}

/*
 * floating point format definitions
 */

typedef double fpnum_t;
#define FP_MAX DOUBLE_MAX
#define FP_MIN DOUBLE_MIN
#define fpnext(val) nextafter(val, INFINITY)
#define fpprev(val) nextafter(val, -INFINITY)

#define ERROL0_EPSILON	0.0000001
#define ERROL1_EPSILON  8.77e-15


/**
 * High-precision data structure.
 *   @val, off: The value and offset.
 */

struct hp_t {
	fpnum_t val, off;
};


/*
 * lookup table data
 */

#include "lookup.h"
#include "err.h"

/*
 * high-precision constants
 */

/*
 * local function declarations
 */

static void hp_normalize(struct hp_t *hp);
static void hp_mul10(struct hp_t *hp);
static void hp_div10(struct hp_t *hp);
struct hp_t hp_prod(struct hp_t in, double val);


/**
 * Errol0 double to ASCII conversion, guaranteed correct but possibly not
 * optimal. Useful for embedded systems.
 *   @val: The value.
 *   @buf: The output buffer.
 *   &returns: The exponent.
 */

int16_t errol0_dtoa(double val, char *buf)
{
	double ten;
	int16_t exp;
	struct hp_t mid, inhi, inlo;

	ten = 1.0;
	exp = 1;

	/* normalize the midpoint */

	mid.val = val;
	mid.off = 0.0;

	while(((mid.val > 10.0) || ((mid.val == 10.0) && (mid.off >= 0.0))) && (exp < 308))
		exp++, hp_div10(&mid), ten /= 10.0;

	while(((mid.val < 1.0) || ((mid.val == 1.0) && (mid.off < 0.0))) && (exp > -307))
		exp--, hp_mul10(&mid), ten *= 10.0;

	inhi.val = mid.val;
	inhi.off = mid.off + (fpnext(val) - val) * ten / (2.0 + ERROL0_EPSILON);
	inlo.val = mid.val;
	inlo.off = mid.off + (fpprev(val) - val) * ten / (2.0 + ERROL0_EPSILON);

	hp_normalize(&inhi);
	hp_normalize(&inlo);

	/* normalized boundaries */

	while(inhi.val > 10.0 || (inhi.val == 10.0 && (inhi.off >= 0.0)))
		exp++, hp_div10(&inhi), hp_div10(&inlo);

	while(inhi.val < 1.0 || (inhi.val == 1.0 && (inhi.off < 0.0)))
		exp--, hp_mul10(&inhi), hp_mul10(&inlo);

	/* digit generation */

	while(inhi.val != 0.0 || inhi.off != 0.0) {
		uint8_t ldig, hdig;

		hdig = (uint8_t)(inhi.val);
		inhi.val -= hdig;
		if((inhi.val == 0.0) && (inhi.off < 0))
			hdig -= 1, inhi.val += 1.0;

		ldig = (uint8_t)(inlo.val);
		inlo.val -= ldig;
		if((inlo.val == 0.0) && (inlo.off < 0))
			ldig -= 1, inlo.val += 1.0;

		*buf++ = hdig + '0';

		if(ldig != hdig)
			break;

		hp_mul10(&inhi);
		hp_mul10(&inlo);
	}

	*buf = '\0';

	return exp;
}

/**
 * Errol double to ASCII conversion.
 *   @val: The value.
 *   @buf: The output buffer.
 *   @opt: The optimality flag.
 *   &returns: The exponent.
 */

int16_t errol1_dtoa(double val, char *buf, bool *opt)
{
	double ten, lten;
	int e;
	int16_t exp;
	struct hp_t mid, inhi, inlo, outhi, outlo;

	ten = 1.0;
	exp = 1;

	/* normalize the midpoint */

	frexp(val, &e);
	exp = 307 + (double)e*0.30103;
	if(exp < 20)
		exp = 20;
	else if(exp >= LOOKUP_TABLE_LEN)
		exp = LOOKUP_TABLE_LEN - 1;

	mid = lookup_table[exp];
	mid = hp_prod(mid, val);
	lten = lookup_table[exp].val;
	ten = 1.0;

	exp -= 307;

	if(((mid.val > 10.0) || ((mid.val == 10.0 && mid.off >= 0.0))) || ((mid.val < 1.0) || ((mid.val == 1.0) && (mid.off < 0.0)))) {
	}

	while((mid.val > 10.0) || ((mid.val == 10.0 && mid.off >= 0.0)))
		exp++, hp_div10(&mid), ten /= 10.0;

	while((mid.val < 1.0) || ((mid.val == 1.0) && (mid.off < 0.0)))
		exp--, hp_mul10(&mid), ten *= 10.0;

	inhi.val = mid.val;
	inhi.off = mid.off + (fpnext(val) - val) * lten * ten / (2.0 + ERROL1_EPSILON);
	inlo.val = mid.val;
	inlo.off = mid.off + (fpprev(val) - val) * lten * ten / (2.0 + ERROL1_EPSILON);
	outhi.val = mid.val;
	outhi.off = mid.off + (fpnext(val) - val) * lten * ten / (2.0 - ERROL1_EPSILON);
	outlo.val = mid.val;
	outlo.off = mid.off + (fpprev(val) - val) * lten * ten / (2.0 - ERROL1_EPSILON);

	hp_normalize(&inhi);
	hp_normalize(&inlo);
	hp_normalize(&outhi);
	hp_normalize(&outlo);

	/* normalized boundaries */

	while(inhi.val > 10.0 || (inhi.val == 10.0 && inhi.off >= 0.0))
		exp++, hp_div10(&inhi), hp_div10(&inlo), hp_div10(&outhi), hp_div10(&outlo);

	while(inhi.val < 1.0 || (inhi.val == 1.0 && inhi.off < 0.0))
		exp--, hp_mul10(&inhi), hp_mul10(&inlo), hp_mul10(&outhi), hp_mul10(&outlo);

	/* digit generation */

	*opt = true;

	while(inhi.val != 0.0 || inhi.off != 0.0) {
		uint8_t ldig, hdig;

		hdig = (uint8_t)(inhi.val);
		inhi.val -= hdig;
		if((inhi.val == 0.0) && (inhi.off < 0))
			hdig -= 1, inhi.val += 1.0;

		ldig = (uint8_t)(inlo.val);
		inlo.val -= ldig;
		if((inlo.val == 0.0) && (inlo.off < 0))
			ldig -= 1, inlo.val += 1.0;

		*buf++ = hdig + '0';

		if(ldig != hdig)
			break;

		hdig = (uint8_t)(outhi.val);
		outhi.val -= hdig;
		if((outhi.val == 0.0) && (outhi.off < 0))
			hdig -= 1, outhi.val += 1.0;

		ldig = (uint8_t)(outlo.val);
		outlo.val -= ldig;
		if((outlo.val == 0.0) && (outlo.off < 0))
			ldig -= 1, outlo.val += 1.0;

		if(ldig != hdig)
			*opt = false;

		hp_mul10(&inhi);
		hp_mul10(&inlo);
		hp_mul10(&outhi);
		hp_mul10(&outlo);
	}

	*buf = '\0';

	return exp;
}

/**
 * Errol2 double to ASCII conversion.
 *   @val: The value.
 *   @buf: The output buffer.
 *   @opt: The optimality flag.
 *   &returns: The exponent.
 */

int16_t errol2_dtoa(double val, char *buf, bool *opt)
{
	//if((val < 1.80143985094820e+16) || (val >= 3.40282366920938e+38))
	if((val < 9.007199254740992e15) || (val >= 3.40282366920938e38))
		return errol1_dtoa(val, buf, opt);

	int8_t i, j;
	int32_t exp;
	union { double d; uint64_t i; } bits;
	char lstr[41], hstr[41];
	uint64_t l64, h64;
	__uint128_t low, high, pow19 = (__uint128_t)1e19;

	low = (__uint128_t)val - (__uint128_t)((nextafter(val, INFINITY) -val) / 2.0);
	high = (__uint128_t)val + (__uint128_t)((val - nextafter(val, -INFINITY)) / 2.0);

	bits.d = val;
	if(bits.i & 0x1)
		low++, high--;

	i = 39;
	lstr[40] = hstr[40] = '\0';
	while(high != 0) {
		l64 = low % pow19;
		low /= pow19;
		h64 = high % pow19;
		high /= pow19;

		for(j = 0; ((high != 0) && (j < 19)) || ((high == 0) && (h64 != 0)); j++, i--) {
			lstr[i] = '0' + l64 % (uint64_t)10;
			hstr[i] = '0' + h64 % (uint64_t)10;

			l64 /= 10;
			h64 /= 10;
		}
	}

	exp = 39 - i++;

	do
		*buf++ = hstr[i++];
	while(hstr[i] != '\0' && hstr[i] == lstr[i]);

	if(allzero(lstr+i) || allzero(hstr+i)) {
		while(buf[-1] == '0')
			buf--;
	}
	else
		*buf++ = hstr[i];

	*buf = '\0';
	*opt = true;

	return exp;
}

/**
 * Errol3 double to ASCII conversion.
 *   @val: The value.
 *   @buf: The output buffer.
 *   &returns: The exponent.
 */

int err_search(const void *key, const void *el) {
	double val = *(const double *)key;
	const struct err_t *err = el;

	if(val < err->val)
		return -1;
	else if(val > err->val)
		return 1;
	else
		return 0;
}

int16_t errol3_dtoa(double val, char *buf)
{
	int e;
	double ten, lten;
	int16_t exp;
	struct hp_t mid;
	struct hp_t high = { val, 0.0 };
	struct hp_t low = { val, 0.0 };

	bool opt;
	//if((val >= 1.80143985094820e+16) && (val < 3.40282366920938e+38))
	if((val >= 9.007199254740992e15) && (val < 3.40282366920938e38))
		return errol2_dtoa(val, buf, &opt);

	int l, h, m;
	l = 0; h = ERR_TABLE_LEN - 1;

	while(l <= h) {
		m = (l + h) / 2;
		if(val < err_table[m].val)
			h = m - 1;
		else if(val > err_table[m].val)
			l = m + 1;
		else {
			strcpy(buf, err_table[m].frac);

			return err_table[m].exp + 1;
		}
	}

	ten = 1.0;
	exp = 1;

	/* normalize the midpoint */

	frexp(val, &e);
	exp = 307 + (double)e*0.30103;
	if(exp < 20)
		exp = 20;
	else if(exp >= LOOKUP_TABLE_LEN)
		exp = LOOKUP_TABLE_LEN - 1;

	mid = lookup_table[exp];
	mid = hp_prod(mid, val);
	lten = lookup_table[exp].val;
	ten = 1.0;

	exp -= 307;

	while(mid.val > 10.0 || (mid.val == 10.0 && mid.off >= 0.0))
		exp++, hp_div10(&mid), ten /= 10.0;

	while(mid.val < 1.0 || (mid.val == 1.0 && mid.off < 0.0))
		exp--, hp_mul10(&mid), ten *= 10.0;

	/* compute boundaries */

	high.val = mid.val;
	high.off = mid.off + (fpnext(val) - val) * lten * ten / 2.0;
	//high.off = mid.off + (fpnext(val) - val) * lten * ten / (2.0 - ERROL1_EPSILON);
	low.val = mid.val;
	low.off = mid.off + (fpprev(val) - val) * lten * ten / 2.0;
	//low.off = mid.off + (fpprev(val) - val) * lten * ten / (2.0 - ERROL1_EPSILON);

	hp_normalize(&high);
	hp_normalize(&low);

	/* normalized boundaries */

	while(high.val > 10.0 || (high.val == 10.0 && high.off >= 0.0))
		exp++, hp_div10(&high), hp_div10(&low);

	while(high.val < 1.0 || (high.val == 1.0 && high.off < 0.0))
		exp--, hp_mul10(&high), hp_mul10(&low);

	/* digit generation */

	while(high.val != 0.0 || high.off != 0.0) {
		uint8_t ldig, hdig;

		hdig = (uint8_t)(high.val);
		high.val -= hdig;
		if((high.val == 0.0) && (high.off < 0))
			hdig -= 1, high.val += 1.0;

		ldig = (uint8_t)(low.val);
		low.val -= ldig;
		if((low.val == 0.0) && (low.off < 0))
			ldig -= 1, low.val += 1.0;

		*buf++ = hdig + '0';

		if(ldig != hdig)
			break;

		hp_mul10(&high);
		hp_mul10(&low);
	}

	*buf = '\0';

	return exp;
}


/**
 * Normalize the number by factoring in the error.
 *   @hp: The float pair.
 */

static void hp_normalize(struct hp_t *hp)
{
	fpnum_t val = hp->val;

	hp->val += hp->off;
	hp->off += val - hp->val;
}

/**
 * Multiply the high-precision number by ten.
 *   @hp: The high-precision number
 */

static void hp_mul10(struct hp_t *hp)
{
	fpnum_t off, val = hp->val;

	hp->val *= 10.0;
	hp->off *= 10.0;
	
	off = hp->val;
	off -= val * 8.0;
	off -= val * 2.0;

	hp->off -= off;

	hp_normalize(hp);
}

/**
 * Divide the high-precision number by ten.
 *   @hp: The high-precision number
 */

static void hp_div10(struct hp_t *hp)
{
	double val = hp->val;

	hp->val /= 10.0;
	hp->off /= 10.0;

	val -= hp->val * 8.0;
	val -= hp->val * 2.0;

	hp->off += val / 10.0;

	hp_normalize(hp);
}

double gethi(double in)
{
	union { double d; uint64_t i; } v = { .d = in };

	//v.i += 0x0000000004000000;
	v.i &= 0xFFFFFFFFF8000000;

	return v.d;
}

void split(double val, double *hi, double *lo)
{
	/*
	double t = (134217728.0 + 1.0) * val;

	*hi = t - (t - val);
	if(*hi != gethi(val)) 
		fprintf(stderr, "DIFF %a -> %a vs %a\n", val, *hi, gethi(val)), abort();
	*lo = val - *hi;
		*/

	*hi = gethi(val);
	*lo = val - *hi;
}

struct hp_t hp_prod(struct hp_t in, double val)
{
	double p, hi, lo, e;

	double hi2, lo2;
	split(in.val, &hi, &lo);
	split(val, &hi2, &lo2);

	p = in.val * val;
	e = ((hi * hi2 - p) + lo * hi2 + hi * lo2) + lo * lo2;

	//fprintf(stderr, "(%.17e + %.17e) + %.17e\n", in.val, in.off, val);
	//fprintf(stderr, ": %.17e %.17e %.17e\n", in.off* val, e, in.off* val+e);
	return (struct hp_t){ p, in.off * val + e };
}
