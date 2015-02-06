#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include "errol.h"


/**
 * High-precision data structure.
 *   @val, off: The value and offset.
 */

struct hp_t {
	double val, off;
};


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

static double fp_next(double val);
static double fp_prev(double val);


/**
 * Errol double to ASCII conversion.
 *   @val: The value.
 *   @buf: The output buffer.
 *   &returns: The exponent.
 */

int16_t errol_dtoa(double val, char *buf)
{
	return 0;
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
	high.off = mid.off + (fp_next(val) - val) * ten / 2.00000001;
	low.val = mid.val;
	low.off = mid.off + (fp_prev(val) - val) * ten / 2.00000001;

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
	double val = hp->val;

	hp->val += hp->off;
	hp->off += val - hp->val;
}

/**
 * Multiply the high-precision number by ten.
 *   @hp: The high-precision number
 */

static void hp_mul10(struct hp_t *hp)
{
	double off, val = hp->val;

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
 * Retrieve the next representable floating-point number.
 *   @val: The value.
 *   &returns: The next value.
 */

static double fp_next(double val)
{
	return nextafter(val, INFINITY);
}

/**
 * Retrieve the previous representable floating-point number.
 *   @val: The value.
 *   &returns: The next value.
 */

static double fp_prev(double val)
{
	return nextafter(val, -INFINITY);
}
