#pragma once

#define GLN_COMPILER_CLANG 0
#define GLN_COMPILER_GCC 0
#define GLN_COMPILER_MSVC 0

#if defined(_MSC_VER)
#	undef GLN_COMPILER_MSVC
#	define GLN_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef GLN_COMPILER_GCC
#	define GLN_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__clang__)
#	undef GLN_COMPILER_CLANG
#	define GLN_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#else
#	error "Undefined compiler"
#endif

#define GLN_PLATFORM_WINDOWS 0
#define GLN_PLATFORM_LINUX 0

#if defined(_WIN32) || defined(_WIN64)
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	undef GLN_PLATFORM_WINDOWS
#	define GLN_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#	undef GLN_PLATFORM_LINUX
#	define GLN_PLATFORM_LINUX 1
#else
#	error "Unsupported platform"
#endif

#include <stdint.h>
#if UINTPTR_MAX == 0xffffffffffffffff
#	define GLN_ARCH_64
#elif UINTPTR_MAX == 0xffffffff
#	define GLN_ARCH_32
#else
#	error "Platform which is neither 32 or 64 bits is not supported"
#endif

#define GLN_DEBUG 0
#define GLN_RELWTIHDEBINFO 0
#define GLN_RELEASE 0

#if defined _DEBUG
#	undef GLN_DEBUG
#	define GLN_DEBUG 1
#elif defined _RELWITHDEBINFO
#	undef GLN_RELWITHDEBINFO
#	define GLN_RELWITHDEBINFO 1
#else
#	undef GLN_RELEASE
#	define GLN_RELEASE 1
#endif

#define GLN_UNUSED(Expr) ((void)Expr)

#if GLN_COMPILER_GCC || GLN_COMPILER_CLANG
#	define GLN_ALIGN(Expr, Align) Expr __attribute__((aligned(Align)))
#	define GLN_NO_VTABLE
#	define GLN_LIKELY(Expr) __builtin_expect(!!(Expr), 1)
#	define GLN_UNLIKELY(Expr) __builtin_expect(!!(Expr), 0)
#	define GLN_FORCE_INLINE inline __attribute__((__always_inline__))
#	define GLN_NO_INLINE __attribute__((noinline))
#	define GLN_FUNCTION __PRETTY_FUNCTION__
#	define GLN_BREAKPOINT __asm__ volatile("int $0x03")
#elif GLN_COMPILER_MSVC
#	define GLN_ALIGN(Expr, Align) __declspec(align(Align)) Expr
#	define GLN_NO_VTABLE __declspec(novtable)
#	define GLN_LIKEYLY(Expr) (Expr)
#	define GLN_UNLIKELY(Expr) (Expr)
#	define GLN_FORCE_INLINE __forceinline
#	define GLN_NO_INLINE __declspec(noinline)
#	define GLN_FUNCTION __FUNCTION__
#	define GLN_BREAKPOINT __debugbreak()
#endif

#if GLN_DEBUG
#	define GLN_ASSERT(Expr)                                                                                                               \
		do                                                                                                                                 \
		{                                                                                                                                  \
			if (!GLN_LIKEYLY(Expr))                                                                                                        \
			{                                                                                                                              \
				GLN_BREAKPOINT;                                                                                                            \
			}                                                                                                                              \
		} while (false)
#else
#	define GLN_ASSERT(Expr)
#endif

#define GLN_ARRAY_SIZE(x) (sizeof(x) / (sizeof(x[0])))
