/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_LIBC_HPP
#define MPT_BASE_DETECT_LIBC_HPP



#include "mpt/base/detect_compiler.hpp"
#include "mpt/base/detect_os.hpp"

#include <cstddef>



// order of checks is important!
#if MPT_COMPILER_GENERIC
#define MPT_LIBC_GENERIC 1
#elif (defined(__MINGW32__) || defined(__MINGW64__))
#define MPT_LIBC_MINGW 1
#elif (defined(__GLIBC__) || defined(__GNU_LIBRARY__))
#define MPT_LIBC_GLIBC 1
#elif MPT_COMPILER_MSVC
#define MPT_LIBC_MS 1
#elif MPT_COMPILER_CLANG && MPT_OS_WINDOWS
#define MPT_LIBC_MS 1
#elif defined(__BIONIC__)
#define MPT_LIBC_BIONIC 1
#elif defined(__APPLE__)
#define MPT_LIBC_APPLE 1
#else
#define MPT_LIBC_GENERIC 1
#endif

#ifndef MPT_LIBC_GENERIC
#define MPT_LIBC_GENERIC 0
#endif
#ifndef MPT_LIBC_GLIBC
#define MPT_LIBC_GLIBC 0
#endif
#ifndef MPT_LIBC_MINGW
#define MPT_LIBC_MINGW 0
#endif
#ifndef MPT_LIBC_MS
#define MPT_LIBC_MS 0
#endif
#ifndef MPT_LIBC_BIONIC
#define MPT_LIBC_BIONIC 0
#endif
#ifndef MPT_LIBC_APPLE
#define MPT_LIBC_APPLE 0
#endif



#endif // MPT_BASE_DETECT_LIBC_HPP
