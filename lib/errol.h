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
 * errol function declarations
 */

int16_t errol1_dtoa(double val, char *buf, bool *opt);
int16_t errol2_dtoa(double val, char *buf);
int16_t errol3_dtoa(double val, char *buf);

#ifdef __cplusplus
}
#endif

#endif
