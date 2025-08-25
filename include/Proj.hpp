#pragma once


#if defined(__cplusplus) && __cplusplus >= 201703L
    #define INLINE inline

#else 
    #if defined(_MSC_VER)
        #define INLINE __inline
    #elif defined(__GNUC__) || defined(__clang__)
        #define INLINE __inline__
    #else
        #define INLINE 
    #endif
#endif

/* 
 * @Usage: if (LIKELY(x > 0)) or if (UNLIKELY(x > 0))
 */
#if defined(__cplusplus) && __cplusplus >= 202002L
    #define LIKELY(x)   (x) [[likely]]
    #define UNLIKELY(x) (x) [[unlikely]]
#elif defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

#if defined(__cplusplus) && __cplusplus >= 201103L
    #define CONSTEXPR constexpr
#else
    #define CONSTEXPR
#endif

