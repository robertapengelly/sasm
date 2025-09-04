/******************************************************************************
 * @file            stdint.h
 *****************************************************************************/
#ifndef     _STDINT_H_INCLUDED
#ifndef     _STDINT_H
#ifndef     _STDINT_H_

#define     _STDINT_H_INCLUDED
#define     _STDINT_H
#define     _STDINT_H_

#include    <limits.h>

#if     defined (NO_LONG_LONG) || ULONG_MAX > 4294967295UL
typedef     signed long                 int64_t;
typedef     unsigned long               uint64_t;
#else
typedef     signed long long            int64_t;
typedef     unsigned long long          uint64_t;
#endif

#endif      /* _STDINT_H_ */
#endif      /* _STDINT_H */
#endif      /* _STDINT_H_INCLUDED */

