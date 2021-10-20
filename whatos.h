/**
 * A header utility that lets one check if the current OS is Windows or some Unix-like.
 */

#ifndef __WHATOS_H__
#define __WHATOS_H__

#define OS_WINDOWS
#define OS_UNIXLIKE

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#undef OS_UNIXLIKE
#else
#undef OS_WINDOWS
#endif

#endif  // __WHATOS_H__
