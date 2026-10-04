#ifdef _MSC_VER
#define PIXAURA_CODEC_HIDDEN
#define PIXAURA_CODEC_INLINE __inline
#define PIXAURA_CODEC_TLS __declspec(thread)
#ifdef _WIN64
#define PIXAURA_CODEC_SIZE_T 8
#else
#define PIXAURA_CODEC_SIZE_T 4
#endif
#else
#define PIXAURA_CODEC_HIDDEN __attribute__((visibility("hidden")))
#define PIXAURA_CODEC_INLINE inline
#define PIXAURA_CODEC_TLS __thread
#define PIXAURA_CODEC_SIZE_T __SIZEOF_SIZE_T__
#endif
/* libjpeg-turbo build number */
#define BUILD  "20260630"

/* How to hide global symbols. */
#define HIDDEN  PIXAURA_CODEC_HIDDEN

/* Compiler's inline keyword */
#undef inline

/* How to obtain function inlining. */
#define INLINE  PIXAURA_CODEC_INLINE

/* How to obtain thread-local storage */
#define THREAD_LOCAL  PIXAURA_CODEC_TLS

/* Define to the full name of this package. */
#define PACKAGE_NAME  "libjpeg-turbo"

/* Version number of package */
#define VERSION  "3.2.0"

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T  PIXAURA_CODEC_SIZE_T

/* Define if your compiler has __builtin_ctzl() and sizeof(unsigned long) == sizeof(size_t). */


/* Define to 1 if you have the <intrin.h> header file. */


#if defined(_MSC_VER) && defined(HAVE_INTRIN_H)
#if (SIZEOF_SIZE_T == 8)
#define HAVE_BITSCANFORWARD64
#elif (SIZEOF_SIZE_T == 4)
#define HAVE_BITSCANFORWARD
#endif
#endif

#if defined(__has_attribute)
#if __has_attribute(fallthrough)
#define FALLTHROUGH  __attribute__((fallthrough));
#else
#define FALLTHROUGH
#endif
#else
#define FALLTHROUGH
#endif

/*
 * Define BITS_IN_JSAMPLE as either
 *   8   for 8-bit sample values (the usual setting)
 *   12  for 12-bit sample values
 * Only 8 and 12 are legal data precisions for lossy JPEG according to the
 * JPEG standard, and the IJG code does not support anything else!
 */

#ifndef BITS_IN_JSAMPLE
#define BITS_IN_JSAMPLE  8      /* use 8 or 12 */
#endif

#undef C_ARITH_CODING_SUPPORTED
#undef D_ARITH_CODING_SUPPORTED
#undef WITH_SIMD

#if BITS_IN_JSAMPLE == 8

/* Support arithmetic encoding */


/* Support arithmetic decoding */


/* Use accelerated SIMD routines. */


#define SIMD_ARCHITECTURE  0

#endif
