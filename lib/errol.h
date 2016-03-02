#ifndef ERROL_H
#define ERROL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * common headers
 */

#include <stdint.h>

/*
 * errol declarations
 */

#define ERR_LEN   512
#define ERR_DEPTH 4

int16_t errol0_dtoa(double val, char *buf);
int16_t errol1_dtoa(double val, char *buf, bool *opt);
int16_t errol2_dtoa(double val, char *buf, bool *opt);
int errol3_dtoa(double val, char *buf);
int errol3u_dtoa(double val, char *buf);
int errol4_dtoa(double val, char *buf);
int errol4u_dtoa(double val, char *buf);

int errol_int(double val, char *buf);
int errol_fixed(double val, char *buf);

uint16_t errol_hash(double val);

struct errol_err_t {
	double val;
	char str[18];
	int exp;
};

#ifdef __cplusplus
}
#endif

#endif
