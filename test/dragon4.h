#ifndef DRAGON4_H
#define DRAGON4_H

/*
 * dragon4 function declarations
 */

extern "C" char *dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve);
extern "C" void freedtoa(char *s);

#endif
