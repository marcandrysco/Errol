#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "errol.h"


/*
 * floating point format definitions
 */

typedef double fpnum_t;
#define FP_MAX DOUBLE_MAX
#define FP_MIN DOUBLE_MIN
#define fpnext(val) nextafter(val, INFINITY)
#define fpprev(val) nextafter(val, -INFINITY)


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

/*
 * high-precision constants
 */

//static struct hp_t hp_tenth = { 0.1, -5.551115123125783e-18 };

/*
 * local function declarations
 */

static void hp_normalize(struct hp_t *hp);
static void hp_mul10(struct hp_t *hp);
static void hp_div10(struct hp_t *hp);
static struct hp_t hp_prod(struct hp_t in, double val);
struct hp_t hp_prod2(struct hp_t in, double val);


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
	int16_t exp;
	struct hp_t mid, inhi, inlo, outhi, outlo;

	ten = 1.0;
	exp = 1;
	//mid = (struct hp_t){ val, 0.0 };

	/* normalize the midpoint */

	if(0) {
		int e;
		frexp(val, &e);
		exp = 307 + (double)e*0.30103; //0.30103 = log_10(2)
		if(exp < 20)
			exp = 20;
		else if(exp >= LOOKUP_TABLE_LEN)
			exp = LOOKUP_TABLE_LEN - 1;

		mid = lookup_table[exp];
		if(1)mid = hp_prod2(mid, val);
		else mid = hp_prod(mid, val);
		lten = lookup_table[exp].val;
		ten = 1.0;

		exp -= 307;
	}
	else {
		mid.val = val;
		mid.off = 0.0;
		lten = 1.0;
	}

	while(mid.val > 10.0 || (mid.val == 10.0 && mid.off >= 0.0))
		exp++, hp_div10(&mid), ten /= 10.0;

	while(mid.val < 1.0 || (mid.val == 1.0 && mid.off < 0.0))
		exp--, hp_mul10(&mid), ten *= 10.0;

	inhi.val = mid.val;
	inhi.off = mid.off + (fpnext(val) - val) * lten * ten / (2.0 + 0.00000001);
	inlo.val = mid.val;
	inlo.off = mid.off + (fpprev(val) - val) * lten * ten / (2.0 + 0.00000001);
	outhi.val = mid.val;
	outhi.off = mid.off + (fpnext(val) - val) * lten * ten / (2.0 - 0.00000001);
	outlo.val = mid.val;
	outlo.off = mid.off + (fpprev(val) - val) * lten * ten / (2.0 - 0.00000001);

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

		//printf("%c %c ", ldig + '0', hdig + '0');

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

		//printf("%c %c \n", ldig + '0', hdig + '0');
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
 *   &returns: The exponent.
 */

int16_t errol2_dtoa(double val, char *buf)
{
	return 0;
}

/**
 * Errol3 double to ASCII conversion.
 *   @val: The value.
 *   @buf: The output buffer.
 *   &returns: The exponent.
 */

int16_t errol3_dtoa(double val, char *buf)
{
	double ten;
	int16_t exp;
	struct hp_t mid;
	struct hp_t high = { val, 0.0 };
	struct hp_t low = { val, 0.0 };

	ten = 1.0;
	exp = 1;
	mid = (struct hp_t){ val, 0.0 };

	/* normalize the midpoint */

	while(mid.val > 10.0 || (mid.val == 10.0 && mid.off >= 0.0))
		exp++, hp_div10(&mid), ten /= 10.0;

	while(mid.val < 1.0 || (mid.val == 1.0 && mid.off < 0.0))
		exp--, hp_mul10(&mid), ten *= 10.0;

	/* compute boundaries */

	high.val = mid.val;
	high.off = mid.off + (fpnext(val) - val) * ten / 2.00000001;
	low.val = mid.val;
	low.off = mid.off + (fpprev(val) - val) * ten / 2.00000001;

	hp_normalize(&high);
	hp_normalize(&low);

	/* normalized boundaries */

	while(high.val > 10.0 || (high.val == 10.0 && high.off >= 0.0))
		exp++, hp_div10(&high), hp_div10(&low);

	while(high.val < 1.0 || (high.val == 1.0 && high.off < 0.0))
		exp--, hp_mul10(&high), hp_mul10(&low);

	/* digit generation */

	int a = 0;
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

		if(a++ > 50);
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

/**
 * Multiply the high-precision number by an arbitrary value.
 *   @in: The high-precision input.
 *   @val: The native input.
 *   &returns: The high-precision output.
 */

static struct hp_t hp_prod(struct hp_t in, double val)
{
	int exp;
	double comp, err, frac;
	struct hp_t out;

	out.val = in.val * val;

	err = out.val;
	frac = frexp(val, &exp);
	comp = ldexp(in.val, exp);
	while(frac != 0.0) {
		if(frac >= 1.0) {
			frac -= 1.0;
			err -= comp;
		}

		comp /= 2.0;
		frac *= 2.0;
	}

	out.off = val * in.off - err;
	hp_normalize(&out);

	return out;
}

void split(double val, double *hi, double *lo)
{
	double t = (134217728.0 + 1) * val;

	*hi = t - (t - val);
	*lo = val - *hi;
}

struct hp_t hp_prod2(struct hp_t in, double val)
{
	double p, hi, lo, e;

	split(in.val, &hi, &lo);

	p = in.val * val;
	e = (hi * val - p) + lo * val;

	return (struct hp_t){ p, in.off * val + e };
}
