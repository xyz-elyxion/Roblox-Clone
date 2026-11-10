#ifndef RBLXMATH_SETUP_INCLUDED

#include <cassert>
#include <cstddef>

#define RBLXMATH_VERSION_MAJOR 1
#define RBLXMATH_VERSION_MINOR 0
#define RBLXMATH_VERSION_PATCH 1
#define RBLXMATH_VERSION_REVISION 0 // Deprecated
#define RBLXMATH_VERSION 1000 // Deprecated
#define RBLXMATH_VERSION_MESSAGE "RBLXMATH: version 1.0.1"

#define RBLXMATH_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29U) | (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))

#define RBLXMATH_VERSION_COMPLETE RBLXMATH_MAKE_API_VERSION(0, RBLXMATH_VERSION_MAJOR, RBLXMATH_VERSION_MINOR, RBLXMATH_VERSION_PATCH)

#define RBLXMATH_SETUP_INCLUDED RBLXMATH_VERSION

#define RBLXMATH_GET_VERSION_VARIANT(version) ((uint32_t)(version) >> 29U)
#define RBLXMATH_GET_VERSION_MAJOR(version) (((uint32_t)(version) >> 22U) & 0x7FU)
#define RBLXMATH_GET_VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define RBLXMATH_GET_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

///////////////////////////////////////////////////////////////////////////////////
// Active states

#define RBLXMATH_DISABLE		0
#define RBLXMATH_ENABLE		1

///////////////////////////////////////////////////////////////////////////////////
// Messages

#if defined(RBLXMATH_FORCE_MESSAGES)
#	define RBLXMATH_MESSAGES RBLXMATH_ENABLE
#else
#	define RBLXMATH_MESSAGES RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Detect the platform

#include "../simd/platform.h"

///////////////////////////////////////////////////////////////////////////////////
// Build model

#if defined(_M_ARM64) || defined(__LP64__) || defined(_M_X64) || defined(__ppc64__) || defined(__x86_64__)
#	define RBLXMATH_MODEL	RBLXMATH_MODEL_64
#elif defined(__i386__) || defined(__ppc__) || defined(__ILP32__) || defined(_M_ARM)
#	define RBLXMATH_MODEL	RBLXMATH_MODEL_32
#else
#	define RBLXMATH_MODEL	RBLXMATH_MODEL_32
#endif//

#if !defined(RBLXMATH_MODEL) && RBLXMATH_COMPILER != 0
#	error "RBLXMATH_MODEL undefined, your compiler may not be supported by RBLXMATH. Add #define RBLXMATH_MODEL 0 to ignore this message."
#endif//RBLXMATH_MODEL

///////////////////////////////////////////////////////////////////////////////////
// C++ Version

// User defines: RBLXMATH_FORCE_CXX98, RBLXMATH_FORCE_CXX03, RBLXMATH_FORCE_CXX11, RBLXMATH_FORCE_CXX14, RBLXMATH_FORCE_CXX17, RBLXMATH_FORCE_CXX2A

#define RBLXMATH_LANG_CXX98_FLAG			(1 << 1)
#define RBLXMATH_LANG_CXX03_FLAG			(1 << 2)
#define RBLXMATH_LANG_CXX0X_FLAG			(1 << 3)
#define RBLXMATH_LANG_CXX11_FLAG			(1 << 4)
#define RBLXMATH_LANG_CXX14_FLAG			(1 << 5)
#define RBLXMATH_LANG_CXX17_FLAG			(1 << 6)
#define RBLXMATH_LANG_CXX20_FLAG			(1 << 7)
#define RBLXMATH_LANG_CXXMS_FLAG			(1 << 8)
#define RBLXMATH_LANG_CXXGNU_FLAG		(1 << 9)

#define RBLXMATH_LANG_CXX98			RBLXMATH_LANG_CXX98_FLAG
#define RBLXMATH_LANG_CXX03			(RBLXMATH_LANG_CXX98 | RBLXMATH_LANG_CXX03_FLAG)
#define RBLXMATH_LANG_CXX0X			(RBLXMATH_LANG_CXX03 | RBLXMATH_LANG_CXX0X_FLAG)
#define RBLXMATH_LANG_CXX11			(RBLXMATH_LANG_CXX0X | RBLXMATH_LANG_CXX11_FLAG)
#define RBLXMATH_LANG_CXX14			(RBLXMATH_LANG_CXX11 | RBLXMATH_LANG_CXX14_FLAG)
#define RBLXMATH_LANG_CXX17			(RBLXMATH_LANG_CXX14 | RBLXMATH_LANG_CXX17_FLAG)
#define RBLXMATH_LANG_CXX20			(RBLXMATH_LANG_CXX17 | RBLXMATH_LANG_CXX20_FLAG)
#define RBLXMATH_LANG_CXXMS			RBLXMATH_LANG_CXXMS_FLAG
#define RBLXMATH_LANG_CXXGNU			RBLXMATH_LANG_CXXGNU_FLAG

#if (defined(_MSC_EXTENSIONS))
#	define RBLXMATH_LANG_EXT RBLXMATH_LANG_CXXMS_FLAG
#elif ((RBLXMATH_COMPILER & (RBLXMATH_COMPILER_CLANG | RBLXMATH_COMPILER_GCC)) && (RBLXMATH_ARCH & RBLXMATH_ARCH_SIMD_BIT))
#	define RBLXMATH_LANG_EXT RBLXMATH_LANG_CXXMS_FLAG
#else
#	define RBLXMATH_LANG_EXT 0
#endif

#if (defined(RBLXMATH_FORCE_CXX_UNKNOWN))
#	define RBLXMATH_LANG 0
#elif defined(RBLXMATH_FORCE_CXX20)
#	define RBLXMATH_LANG (RBLXMATH_LANG_CXX20 | RBLXMATH_LANG_EXT)
#	define RBLXMATH_LANG_STL11_FORCED
#elif defined(RBLXMATH_FORCE_CXX17)
#	define RBLXMATH_LANG (RBLXMATH_LANG_CXX17 | RBLXMATH_LANG_EXT)
#	define RBLXMATH_LANG_STL11_FORCED
#elif defined(RBLXMATH_FORCE_CXX14)
#	define RBLXMATH_LANG (RBLXMATH_LANG_CXX14 | RBLXMATH_LANG_EXT)
#	define RBLXMATH_LANG_STL11_FORCED
#elif defined(RBLXMATH_FORCE_CXX11)
#	define RBLXMATH_LANG (RBLXMATH_LANG_CXX11 | RBLXMATH_LANG_EXT)
#	define RBLXMATH_LANG_STL11_FORCED
#elif defined(RBLXMATH_FORCE_CXX03)
#	define RBLXMATH_LANG (RBLXMATH_LANG_CXX03 | RBLXMATH_LANG_EXT)
#elif defined(RBLXMATH_FORCE_CXX98)
#	define RBLXMATH_LANG (RBLXMATH_LANG_CXX98 | RBLXMATH_LANG_EXT)
#else
#	if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC && defined(_MSVC_LANG)
#		if RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC15_7
#			define RBLXMATH_LANG_PLATFORM _MSVC_LANG
#		elif RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC15
#			if _MSVC_LANG > 201402L
#				define RBLXMATH_LANG_PLATFORM 201402L
#			else
#				define RBLXMATH_LANG_PLATFORM _MSVC_LANG
#			endif
#		else
#			define RBLXMATH_LANG_PLATFORM 0
#		endif
#	else
#		define RBLXMATH_LANG_PLATFORM 0
#	endif

#	if __cplusplus > 201703L || RBLXMATH_LANG_PLATFORM > 201703L
#		define RBLXMATH_LANG (RBLXMATH_LANG_CXX20 | RBLXMATH_LANG_EXT)
#	elif __cplusplus == 201703L || RBLXMATH_LANG_PLATFORM == 201703L
#		define RBLXMATH_LANG (RBLXMATH_LANG_CXX17 | RBLXMATH_LANG_EXT)
#	elif __cplusplus == 201402L || __cplusplus == 201406L || __cplusplus == 201500L || RBLXMATH_LANG_PLATFORM == 201402L
#		define RBLXMATH_LANG (RBLXMATH_LANG_CXX14 | RBLXMATH_LANG_EXT)
#	elif __cplusplus == 201103L || RBLXMATH_LANG_PLATFORM == 201103L
#		define RBLXMATH_LANG (RBLXMATH_LANG_CXX11 | RBLXMATH_LANG_EXT)
#	elif defined(__INTEL_CXX11_MODE__) || defined(_MSC_VER) || defined(__GXX_EXPERIMENTAL_CXX0X__)
#		define RBLXMATH_LANG (RBLXMATH_LANG_CXX0X | RBLXMATH_LANG_EXT)
#	elif __cplusplus == 199711L
#		define RBLXMATH_LANG (RBLXMATH_LANG_CXX98 | RBLXMATH_LANG_EXT)
#	else
#		define RBLXMATH_LANG (0 | RBLXMATH_LANG_EXT)
#	endif
#endif

///////////////////////////////////////////////////////////////////////////////////
// Has of C++ features

// http://clang.llvm.org/cxx_status.html
// http://gcc.gnu.org/projects/cxx0x.html
// http://msdn.microsoft.com/en-us/library/vstudio/hh567368(v=vs.120).aspx

// Android has multiple STLs but C++11 STL detection doesn't always work #284 #564
#if RBLXMATH_PLATFORM == RBLXMATH_PLATFORM_ANDROID && !defined(RBLXMATH_LANG_STL11_FORCED)
#	define RBLXMATH_HAS_CXX11_STL 0
#elif (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA_RTC) == RBLXMATH_COMPILER_CUDA_RTC
#	define RBLXMATH_HAS_CXX11_STL 0
#elif (RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)
#	define RBLXMATH_HAS_CXX11_STL 0
#elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	if (defined(_LIBCPP_VERSION) || (RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) || defined(RBLXMATH_LANG_STL11_FORCED))
#		define RBLXMATH_HAS_CXX11_STL 1
#	else
#		define RBLXMATH_HAS_CXX11_STL 0
#	endif
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_CXX11_STL 1
#else
#	define RBLXMATH_HAS_CXX11_STL ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_GCC48)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC12)) || \
		((RBLXMATH_PLATFORM != RBLXMATH_PLATFORM_WINDOWS) && (RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_INTEL15))))
#endif

// N1720
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_STATIC_ASSERT __has_feature(cxx_static_assert)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_STATIC_ASSERT 1
#else
#	define RBLXMATH_HAS_STATIC_ASSERT ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N1988
#if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_EXTENDED_INTEGER_TYPE 1
#else
#	define RBLXMATH_HAS_EXTENDED_INTEGER_TYPE (\
		((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC)) || \
		((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)))
#endif

// N2672 Initializer lists http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2672.htm
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_INITIALIZER_LISTS __has_feature(cxx_generalized_initializers)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_INITIALIZER_LISTS 1
#else
#	define RBLXMATH_HAS_INITIALIZER_LISTS ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC15)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_INTEL14)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N2544 Unrestricted unions http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2544.pdf
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_UNRESTRICTED_UNIONS __has_feature(cxx_unrestricted_unions)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_UNRESTRICTED_UNIONS 1
#else
#	define RBLXMATH_HAS_UNRESTRICTED_UNIONS (RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		(RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)))
#endif

// N2346
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_DEFAULTED_FUNCTIONS __has_feature(cxx_defaulted_functions)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_DEFAULTED_FUNCTIONS 1
#else
#	define RBLXMATH_HAS_DEFAULTED_FUNCTIONS ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC12)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL)) || \
		(RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)))
#endif

// N2118
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_RVALUE_REFERENCES __has_feature(cxx_rvalue_references)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_RVALUE_REFERENCES 1
#else
#	define RBLXMATH_HAS_RVALUE_REFERENCES ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N2437 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2437.pdf
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_EXPLICIT_CONVERSION_OPERATORS __has_feature(cxx_explicit_conversions)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_EXPLICIT_CONVERSION_OPERATORS 1
#else
#	define RBLXMATH_HAS_EXPLICIT_CONVERSION_OPERATORS ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_INTEL14)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC12)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N2258 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2258.pdf
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_TEMPLATE_ALIASES __has_feature(cxx_alias_templates)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_TEMPLATE_ALIASES 1
#else
#	define RBLXMATH_HAS_TEMPLATE_ALIASES ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC12)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N2930 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2009/n2930.html
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_RANGE_FOR __has_feature(cxx_range_for)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_RANGE_FOR 1
#else
#	define RBLXMATH_HAS_RANGE_FOR ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N2341 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2341.pdf
#if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#	define RBLXMATH_HAS_ALIGNOF __has_feature(cxx_alignas)
#elif RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_ALIGNOF 1
#else
#	define RBLXMATH_HAS_ALIGNOF ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_INTEL15)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC14)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

// N2235 Generalized Constant Expressions http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2235.pdf
// N3652 Extended Constant Expressions http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3652.html
#if (RBLXMATH_ARCH & RBLXMATH_ARCH_SIMD_BIT) // Compiler SIMD intrinsics don't support constexpr...
#	define RBLXMATH_HAS_CONSTEXPR 0
#elif (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#	define RBLXMATH_HAS_CONSTEXPR __has_feature(cxx_relaxed_constexpr)
#elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX14_FLAG)
#	define RBLXMATH_HAS_CONSTEXPR 1
#else
#	define RBLXMATH_HAS_CONSTEXPR ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && RBLXMATH_HAS_INITIALIZER_LISTS && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_INTEL17)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC15))))
#endif

#if RBLXMATH_HAS_CONSTEXPR
#	define RBLXMATH_CONSTEXPR constexpr
#else
#	define RBLXMATH_CONSTEXPR
#endif

//
#if RBLXMATH_HAS_CONSTEXPR
# if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#	if __has_feature(cxx_if_constexpr)
#		define RBLXMATH_HAS_IF_CONSTEXPR 1
#	else
# 		define RBLXMATH_HAS_IF_CONSTEXPR 0
#	endif
# elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX17_FLAG)
# 	define RBLXMATH_HAS_IF_CONSTEXPR 1
# else
# 	define RBLXMATH_HAS_IF_CONSTEXPR 0
# endif
#else
#	define RBLXMATH_HAS_IF_CONSTEXPR 0
#endif

#if RBLXMATH_HAS_IF_CONSTEXPR
# 	define RBLXMATH_IF_CONSTEXPR if constexpr
#else
#	define RBLXMATH_IF_CONSTEXPR if
#endif

// [nodiscard]
#if RBLXMATH_LANG & RBLXMATH_LANG_CXX17_FLAG
#	define RBLXMATH_NODISCARD [[nodiscard]]
#else
#	define RBLXMATH_NODISCARD
#endif

//
#if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_ASSIGNABLE 1
#else
#	define RBLXMATH_HAS_ASSIGNABLE ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC15)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_GCC49))))
#endif

//
#define RBLXMATH_HAS_TRIVIAL_QUERIES 0

//
#if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_MAKE_SIGNED 1
#else
#	define RBLXMATH_HAS_MAKE_SIGNED ((RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC12)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP))))
#endif

//
#if defined(RBLXMATH_FORCE_INTRINSICS)
#	define RBLXMATH_HAS_BITSCAN_WINDOWS ((RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS) && (\
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL)) || \
		((RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) && (RBLXMATH_COMPILER >= RBLXMATH_COMPILER_VC14) && (RBLXMATH_ARCH & RBLXMATH_ARCH_X86_BIT))))
#else
#	define RBLXMATH_HAS_BITSCAN_WINDOWS 0
#endif

#if RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG
#	define RBLXMATH_HAS_NOEXCEPT 1
#else
#	define RBLXMATH_HAS_NOEXCEPT 0
#endif

#if RBLXMATH_HAS_NOEXCEPT
#	define RBLXMATH_NOEXCEPT noexcept
#else
#	define RBLXMATH_NOEXCEPT
#endif

///////////////////////////////////////////////////////////////////////////////////
// OpenMP
#ifdef _OPENMP
#	if RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC
#		if RBLXMATH_COMPILER >= RBLXMATH_COMPILER_GCC61
#			define RBLXMATH_HAS_OPENMP 45
#		elif RBLXMATH_COMPILER >= RBLXMATH_COMPILER_GCC49
#			define RBLXMATH_HAS_OPENMP 40
#		elif RBLXMATH_COMPILER >= RBLXMATH_COMPILER_GCC47
#			define RBLXMATH_HAS_OPENMP 31
#		else
#			define RBLXMATH_HAS_OPENMP 0
#		endif
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#		if RBLXMATH_COMPILER >= RBLXMATH_COMPILER_CLANG38
#			define RBLXMATH_HAS_OPENMP 31
#		else
#			define RBLXMATH_HAS_OPENMP 0
#		endif
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#		define RBLXMATH_HAS_OPENMP 20
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL
#		if RBLXMATH_COMPILER >= RBLXMATH_COMPILER_INTEL16
#			define RBLXMATH_HAS_OPENMP 40
#		else
#			define RBLXMATH_HAS_OPENMP 0
#		endif
#	else
#		define RBLXMATH_HAS_OPENMP 0
#	endif
#else
#	define RBLXMATH_HAS_OPENMP 0
#endif

///////////////////////////////////////////////////////////////////////////////////
// nullptr

#if RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG
#	define RBLXMATH_CONFIG_NULLPTR RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_NULLPTR RBLXMATH_DISABLE
#endif

#if RBLXMATH_CONFIG_NULLPTR == RBLXMATH_ENABLE
#	define RBLXMATH_NULLPTR nullptr
#else
#	define RBLXMATH_NULLPTR 0
#endif

///////////////////////////////////////////////////////////////////////////////////
// Static assert

#if RBLXMATH_HAS_STATIC_ASSERT
#	define RBLXMATH_STATIC_ASSERT(x, message) static_assert(x, message)
#elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#	define RBLXMATH_STATIC_ASSERT(x, message) typedef char __CASSERT__##__LINE__[(x) ? 1 : -1]
#else
#	define RBLXMATH_STATIC_ASSERT(x, message) assert(x)
#endif//RBLXMATH_LANG

///////////////////////////////////////////////////////////////////////////////////
// Qualifiers

// User defines: RBLXMATH_CUDA_FORCE_DEVICE_FUNC, RBLXMATH_CUDA_FORCE_HOST_FUNC

#if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)
#	if defined(RBLXMATH_CUDA_FORCE_DEVICE_FUNC) && defined(RBLXMATH_CUDA_FORCE_HOST_FUNC)
#		error "RBLXMATH error: RBLXMATH_CUDA_FORCE_DEVICE_FUNC and RBLXMATH_CUDA_FORCE_HOST_FUNC should not be defined at the same time, RBLXMATH by default generates both device and host code for CUDA compiler."
#	endif//defined(RBLXMATH_CUDA_FORCE_DEVICE_FUNC) && defined(RBLXMATH_CUDA_FORCE_HOST_FUNC)

#	if defined(RBLXMATH_CUDA_FORCE_DEVICE_FUNC)
#		define RBLXMATH_CUDA_FUNC_DEF __device__
#		define RBLXMATH_CUDA_FUNC_DECL __device__
#	elif defined(RBLXMATH_CUDA_FORCE_HOST_FUNC)
#		define RBLXMATH_CUDA_FUNC_DEF __host__
#		define RBLXMATH_CUDA_FUNC_DECL __host__
#	else
#		define RBLXMATH_CUDA_FUNC_DEF __device__ __host__
#		define RBLXMATH_CUDA_FUNC_DECL __device__ __host__
#	endif//defined(RBLXMATH_CUDA_FORCE_XXXX_FUNC)
#else
#	define RBLXMATH_CUDA_FUNC_DEF
#	define RBLXMATH_CUDA_FUNC_DECL
#endif

#if defined(RBLXMATH_FORCE_INLINE)
#	if RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#		define RBLXMATH_INLINE __forceinline
#		define RBLXMATH_NEVER_INLINE __declspec(noinline)
#	elif RBLXMATH_COMPILER & (RBLXMATH_COMPILER_GCC | RBLXMATH_COMPILER_CLANG)
#		define RBLXMATH_INLINE inline __attribute__((__always_inline__))
#		define RBLXMATH_NEVER_INLINE __attribute__((__noinline__))
#	elif (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)
#		define RBLXMATH_INLINE __forceinline__
#		define RBLXMATH_NEVER_INLINE __noinline__
#	else
#		define RBLXMATH_INLINE inline
#		define RBLXMATH_NEVER_INLINE
#	endif//RBLXMATH_COMPILER
#else
#	define RBLXMATH_INLINE inline
#	define RBLXMATH_NEVER_INLINE
#endif//defined(RBLXMATH_FORCE_INLINE)

#define RBLXMATH_CTOR_DECL RBLXMATH_CUDA_FUNC_DECL RBLXMATH_CONSTEXPR
#define RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CUDA_FUNC_DECL
#define RBLXMATH_FUNC_DECL RBLXMATH_NODISCARD RBLXMATH_CUDA_FUNC_DECL
#define RBLXMATH_FUNC_QUALIFIER RBLXMATH_CUDA_FUNC_DEF RBLXMATH_INLINE

// Do not use CUDA function qualifiers on CUDA compiler when functions are made default
#if RBLXMATH_HAS_DEFAULTED_FUNCTIONS
#	define RBLXMATH_DEFAULTED_FUNC_DECL
#	define RBLXMATH_DEFAULTED_FUNC_QUALIFIER RBLXMATH_INLINE
#else
#	define RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_FUNC_DISCARD_DECL
#	define RBLXMATH_DEFAULTED_FUNC_QUALIFIER RBLXMATH_FUNC_QUALIFIER
#endif//RBLXMATH_HAS_DEFAULTED_FUNCTIONS
#if !defined(RBLXMATH_FORCE_CTOR_INIT)
#	define RBLXMATH_DEFAULTED_DEFAULT_CTOR_DECL RBLXMATH_CUDA_FUNC_DECL
#	define RBLXMATH_DEFAULTED_DEFAULT_CTOR_QUALIFIER RBLXMATH_DEFAULTED_FUNC_QUALIFIER
#else
#	define RBLXMATH_DEFAULTED_DEFAULT_CTOR_DECL RBLXMATH_FUNC_DISCARD_DECL
#	define RBLXMATH_DEFAULTED_DEFAULT_CTOR_QUALIFIER RBLXMATH_FUNC_QUALIFIER
#endif//RBLXMATH_FORCE_CTOR_INIT

///////////////////////////////////////////////////////////////////////////////////
// Swizzle operators

// User defines: RBLXMATH_FORCE_SWIZZLE

#define RBLXMATH_SWIZZLE_DISABLED		0
#define RBLXMATH_SWIZZLE_OPERATOR		1
#define RBLXMATH_SWIZZLE_FUNCTION		2

#if defined(RBLXMATH_SWIZZLE)
#	pragma message("RBLXMATH: RBLXMATH_SWIZZLE is deprecated, use RBLXMATH_FORCE_SWIZZLE instead.")
#	define RBLXMATH_FORCE_SWIZZLE
#endif

#if defined(RBLXMATH_FORCE_SWIZZLE) && (RBLXMATH_LANG & RBLXMATH_LANG_CXXMS_FLAG) && !defined(RBLXMATH_FORCE_XYZW_ONLY)
#	define RBLXMATH_CONFIG_SWIZZLE RBLXMATH_SWIZZLE_OPERATOR
#elif defined(RBLXMATH_FORCE_SWIZZLE)
#	define RBLXMATH_CONFIG_SWIZZLE RBLXMATH_SWIZZLE_FUNCTION
#else
#	define RBLXMATH_CONFIG_SWIZZLE RBLXMATH_SWIZZLE_DISABLED
#endif

///////////////////////////////////////////////////////////////////////////////////
// Allows using not basic types as genType

// #define RBLXMATH_FORCE_UNRESTRICTED_GENTYPE

#ifdef RBLXMATH_FORCE_UNRESTRICTED_GENTYPE
#	define RBLXMATH_CONFIG_UNRESTRICTED_GENTYPE RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_UNRESTRICTED_GENTYPE RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Allows using any scaler as float

// #define RBLXMATH_FORCE_UNRESTRICTED_FLOAT

#ifdef RBLXMATH_FORCE_UNRESTRICTED_FLOAT
#	define RBLXMATH_CONFIG_UNRESTRICTED_FLOAT RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_UNRESTRICTED_FLOAT RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Clip control, define RBLXMATH_FORCE_DEPTH_ZERO_TO_ONE before including RBLXMATH
// to use a clip space between 0 to 1.
// Coordinate system, define RBLXMATH_FORCE_LEFT_HANDED before including RBLXMATH
// to use left handed coordinate system by default.

#define RBLXMATH_CLIP_CONTROL_ZO_BIT		(1 << 0) // ZERO_TO_ONE
#define RBLXMATH_CLIP_CONTROL_NO_BIT		(1 << 1) // NEGATIVE_ONE_TO_ONE
#define RBLXMATH_CLIP_CONTROL_LH_BIT		(1 << 2) // LEFT_HANDED, For DirectX, Metal, Vulkan
#define RBLXMATH_CLIP_CONTROL_RH_BIT		(1 << 3) // RIGHT_HANDED, For OpenGL, default in RBLXMATH

#define RBLXMATH_CLIP_CONTROL_LH_ZO (RBLXMATH_CLIP_CONTROL_LH_BIT | RBLXMATH_CLIP_CONTROL_ZO_BIT)
#define RBLXMATH_CLIP_CONTROL_LH_NO (RBLXMATH_CLIP_CONTROL_LH_BIT | RBLXMATH_CLIP_CONTROL_NO_BIT)
#define RBLXMATH_CLIP_CONTROL_RH_ZO (RBLXMATH_CLIP_CONTROL_RH_BIT | RBLXMATH_CLIP_CONTROL_ZO_BIT)
#define RBLXMATH_CLIP_CONTROL_RH_NO (RBLXMATH_CLIP_CONTROL_RH_BIT | RBLXMATH_CLIP_CONTROL_NO_BIT)

#ifdef RBLXMATH_FORCE_DEPTH_ZERO_TO_ONE
#	ifdef RBLXMATH_FORCE_LEFT_HANDED
#		define RBLXMATH_CONFIG_CLIP_CONTROL RBLXMATH_CLIP_CONTROL_LH_ZO
#	else
#		define RBLXMATH_CONFIG_CLIP_CONTROL RBLXMATH_CLIP_CONTROL_RH_ZO
#	endif
#else
#	ifdef RBLXMATH_FORCE_LEFT_HANDED
#		define RBLXMATH_CONFIG_CLIP_CONTROL RBLXMATH_CLIP_CONTROL_LH_NO
#	else
#		define RBLXMATH_CONFIG_CLIP_CONTROL RBLXMATH_CLIP_CONTROL_RH_NO
#	endif
#endif

///////////////////////////////////////////////////////////////////////////////////
// Qualifiers

#if (RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC) || ((RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL) && (RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS))
#	define RBLXMATH_DEPRECATED __declspec(deprecated)
#	define RBLXMATH_ALIGNED_TYPEDEF(type, name, alignment) typedef __declspec(align(alignment)) type name
#elif RBLXMATH_COMPILER & (RBLXMATH_COMPILER_GCC | RBLXMATH_COMPILER_CLANG | RBLXMATH_COMPILER_INTEL)
#	define RBLXMATH_DEPRECATED __attribute__((__deprecated__))
#	define RBLXMATH_ALIGNED_TYPEDEF(type, name, alignment) typedef type name __attribute__((aligned(alignment)))
#elif (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA) || (RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP)
#	define RBLXMATH_DEPRECATED
#	define RBLXMATH_ALIGNED_TYPEDEF(type, name, alignment) typedef type name __align__(x)
#else
#	define RBLXMATH_DEPRECATED
#	define RBLXMATH_ALIGNED_TYPEDEF(type, name, alignment) typedef type name
#endif

///////////////////////////////////////////////////////////////////////////////////

#ifdef RBLXMATH_FORCE_EXPLICIT_CTOR
#	define RBLXMATH_EXPLICIT explicit
#else
#	define RBLXMATH_EXPLICIT
#endif

///////////////////////////////////////////////////////////////////////////////////
// Length type: all length functions returns a length_t type.
// When RBLXMATH_FORCE_SIZE_T_LENGTH is defined, length_t is a typedef of size_t otherwise
// length_t is a typedef of int like GLSL defines it.

#define RBLXMATH_LENGTH_INT		1
#define RBLXMATH_LENGTH_SIZE_T	2

#ifdef RBLXMATH_FORCE_SIZE_T_LENGTH
#	define RBLXMATH_CONFIG_LENGTH_TYPE		RBLXMATH_LENGTH_SIZE_T
#	define RBLXMATH_ASSERT_LENGTH(l, max) (assert ((l) < (max)))
#else
#	define RBLXMATH_CONFIG_LENGTH_TYPE		RBLXMATH_LENGTH_INT
#	define RBLXMATH_ASSERT_LENGTH(l, max) (assert ((l) >= 0 && (l) < (max)))
#endif

namespace rblxmath
{
	using std::size_t;
#	if RBLXMATH_CONFIG_LENGTH_TYPE == RBLXMATH_LENGTH_SIZE_T
		typedef size_t length_t;
#	else
		typedef int length_t;
#	endif
}//namespace rblxmath

///////////////////////////////////////////////////////////////////////////////////
// constexpr

#if RBLXMATH_HAS_CONSTEXPR
#	define RBLXMATH_CONFIG_CONSTEXP RBLXMATH_ENABLE

	namespace rblxmath
	{
		template<typename T, std::size_t N>
		constexpr std::size_t countof(T const (&)[N])
		{
			return N;
		}
	}//namespace rblxmath
#	define RBLXMATH_COUNTOF(arr) rblxmath::countof(arr)
#elif defined(_MSC_VER)
#	define RBLXMATH_CONFIG_CONSTEXP RBLXMATH_DISABLE

#	define RBLXMATH_COUNTOF(arr) _countof(arr)
#else
#	define RBLXMATH_CONFIG_CONSTEXP RBLXMATH_DISABLE

#	define RBLXMATH_COUNTOF(arr) sizeof(arr) / sizeof(arr[0])
#endif

///////////////////////////////////////////////////////////////////////////////////
// uint

namespace rblxmath{
namespace detail
{
	template<typename T>
	struct is_int
	{
		enum test {value = 0};
	};

	template<>
	struct is_int<unsigned int>
	{
		enum test {value = ~0};
	};

	template<>
	struct is_int<signed int>
	{
		enum test {value = ~0};
	};
}//namespace detail

	typedef unsigned int	uint;
}//namespace rblxmath

///////////////////////////////////////////////////////////////////////////////////
// 64-bit int

#if RBLXMATH_HAS_EXTENDED_INTEGER_TYPE
#	include <cstdint>
#endif

namespace rblxmath{
namespace detail
{
#	if RBLXMATH_HAS_EXTENDED_INTEGER_TYPE
		typedef std::uint64_t						uint64;
		typedef std::int64_t						int64;
#	elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) // C99 detected, 64 bit types available
		typedef uint64_t							uint64;
		typedef int64_t								int64;
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
		typedef unsigned __int64					uint64;
		typedef signed __int64						int64;
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC
#		pragma GCC diagnostic ignored "-Wlong-long"
		__extension__ typedef unsigned long long	uint64;
		__extension__ typedef signed long long		int64;
#	elif (RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG)
#		pragma clang diagnostic ignored "-Wc++11-long-long"
		typedef unsigned long long					uint64;
		typedef signed long long					int64;
#	else//unknown compiler
		typedef unsigned long long					uint64;
		typedef signed long long					int64;
#	endif
}//namespace detail
}//namespace rblxmath

///////////////////////////////////////////////////////////////////////////////////
// make_unsigned

#if RBLXMATH_HAS_MAKE_SIGNED
#	include <type_traits>

namespace rblxmath{
namespace detail
{
	using std::make_unsigned;
}//namespace detail
}//namespace rblxmath

#else

namespace rblxmath{
namespace detail
{
	template<typename genType>
	struct make_unsigned
	{};

	template<>
	struct make_unsigned<char>
	{
		typedef unsigned char type;
	};

	template<>
	struct make_unsigned<signed char>
	{
		typedef unsigned char type;
	};

	template<>
	struct make_unsigned<short>
	{
		typedef unsigned short type;
	};

	template<>
	struct make_unsigned<int>
	{
		typedef unsigned int type;
	};

	template<>
	struct make_unsigned<long>
	{
		typedef unsigned long type;
	};

	template<>
	struct make_unsigned<int64>
	{
		typedef uint64 type;
	};

	template<>
	struct make_unsigned<unsigned char>
	{
		typedef unsigned char type;
	};

	template<>
	struct make_unsigned<unsigned short>
	{
		typedef unsigned short type;
	};

	template<>
	struct make_unsigned<unsigned int>
	{
		typedef unsigned int type;
	};

	template<>
	struct make_unsigned<unsigned long>
	{
		typedef unsigned long type;
	};

	template<>
	struct make_unsigned<uint64>
	{
		typedef uint64 type;
	};
}//namespace detail
}//namespace rblxmath
#endif

///////////////////////////////////////////////////////////////////////////////////
// Only use x, y, z, w as vector type components

#ifdef RBLXMATH_FORCE_XYZW_ONLY
#	define RBLXMATH_CONFIG_XYZW_ONLY RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_XYZW_ONLY RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of defaulted initialized types

#define RBLXMATH_CTOR_INIT_DISABLE		0
#define RBLXMATH_CTOR_INITIALIZER_LIST	1
#define RBLXMATH_CTOR_INITIALISATION		2

#if defined(RBLXMATH_FORCE_CTOR_INIT) && RBLXMATH_HAS_INITIALIZER_LISTS
#	define RBLXMATH_CONFIG_CTOR_INIT RBLXMATH_CTOR_INITIALIZER_LIST
#elif defined(RBLXMATH_FORCE_CTOR_INIT) && !RBLXMATH_HAS_INITIALIZER_LISTS
#	define RBLXMATH_CONFIG_CTOR_INIT RBLXMATH_CTOR_INITIALISATION
#else
#	define RBLXMATH_CONFIG_CTOR_INIT RBLXMATH_CTOR_INIT_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Use SIMD instruction sets

#if RBLXMATH_HAS_ALIGNOF && (RBLXMATH_LANG & RBLXMATH_LANG_CXXMS_FLAG) && (RBLXMATH_ARCH & RBLXMATH_ARCH_SIMD_BIT)
#	define RBLXMATH_CONFIG_SIMD RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_SIMD RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of defaulted function

#if RBLXMATH_HAS_DEFAULTED_FUNCTIONS
#	define RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS RBLXMATH_ENABLE
#	define RBLXMATH_DEFAULT = default
#else
#	define RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS RBLXMATH_DISABLE
#	define RBLXMATH_DEFAULT
#endif

#if RBLXMATH_CONFIG_CTOR_INIT == RBLXMATH_CTOR_INIT_DISABLE && RBLXMATH_CONFIG_DEFAULTED_FUNCTIONS == RBLXMATH_ENABLE
#	define RBLXMATH_CONFIG_DEFAULTED_DEFAULT_CTOR RBLXMATH_ENABLE
#	define RBLXMATH_DEFAULT_CTOR RBLXMATH_DEFAULT
#else
#	define RBLXMATH_CONFIG_DEFAULTED_DEFAULT_CTOR RBLXMATH_DISABLE
#	define RBLXMATH_DEFAULT_CTOR
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of aligned gentypes

#ifdef RBLXMATH_FORCE_ALIGNED // Legacy define
#	define RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

#ifdef RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES
#	define RBLXMATH_FORCE_ALIGNED_GENTYPES
#endif

#if RBLXMATH_HAS_ALIGNOF && (RBLXMATH_LANG & RBLXMATH_LANG_CXXMS_FLAG) && (defined(RBLXMATH_FORCE_ALIGNED_GENTYPES) || (RBLXMATH_CONFIG_SIMD == RBLXMATH_ENABLE))
#	define RBLXMATH_CONFIG_ALIGNED_GENTYPES RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_ALIGNED_GENTYPES RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Configure the use of anonymous structure as implementation detail

#if ((RBLXMATH_CONFIG_SIMD == RBLXMATH_ENABLE) || (RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR) || (RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_ENABLE))
#	define RBLXMATH_CONFIG_ANONYMOUS_STRUCT RBLXMATH_ENABLE
#else
#	define RBLXMATH_CONFIG_ANONYMOUS_STRUCT RBLXMATH_DISABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Silent warnings

#ifdef RBLXMATH_FORCE_WARNINGS
#	define RBLXMATH_SILENT_WARNINGS RBLXMATH_DISABLE
#else
#	define RBLXMATH_SILENT_WARNINGS RBLXMATH_ENABLE
#endif

///////////////////////////////////////////////////////////////////////////////////
// Precision

#define RBLXMATH_HIGHP		1
#define RBLXMATH_MEDIUMP		2
#define RBLXMATH_LOWP		3

#if defined(RBLXMATH_FORCE_PRECISION_HIGHP_BOOL) || defined(RBLXMATH_PRECISION_HIGHP_BOOL)
#	define RBLXMATH_CONFIG_PRECISION_BOOL		RBLXMATH_HIGHP
#elif defined(RBLXMATH_FORCE_PRECISION_MEDIUMP_BOOL) || defined(RBLXMATH_PRECISION_MEDIUMP_BOOL)
#	define RBLXMATH_CONFIG_PRECISION_BOOL		RBLXMATH_MEDIUMP
#elif defined(RBLXMATH_FORCE_PRECISION_LOWP_BOOL) || defined(RBLXMATH_PRECISION_LOWP_BOOL)
#	define RBLXMATH_CONFIG_PRECISION_BOOL		RBLXMATH_LOWP
#else
#	define RBLXMATH_CONFIG_PRECISION_BOOL		RBLXMATH_HIGHP
#endif

#if defined(RBLXMATH_FORCE_PRECISION_HIGHP_INT) || defined(RBLXMATH_PRECISION_HIGHP_INT)
#	define RBLXMATH_CONFIG_PRECISION_INT			RBLXMATH_HIGHP
#elif defined(RBLXMATH_FORCE_PRECISION_MEDIUMP_INT) || defined(RBLXMATH_PRECISION_MEDIUMP_INT)
#	define RBLXMATH_CONFIG_PRECISION_INT			RBLXMATH_MEDIUMP
#elif defined(RBLXMATH_FORCE_PRECISION_LOWP_INT) || defined(RBLXMATH_PRECISION_LOWP_INT)
#	define RBLXMATH_CONFIG_PRECISION_INT			RBLXMATH_LOWP
#else
#	define RBLXMATH_CONFIG_PRECISION_INT			RBLXMATH_HIGHP
#endif

#if defined(RBLXMATH_FORCE_PRECISION_HIGHP_UINT) || defined(RBLXMATH_PRECISION_HIGHP_UINT)
#	define RBLXMATH_CONFIG_PRECISION_UINT		RBLXMATH_HIGHP
#elif defined(RBLXMATH_FORCE_PRECISION_MEDIUMP_UINT) || defined(RBLXMATH_PRECISION_MEDIUMP_UINT)
#	define RBLXMATH_CONFIG_PRECISION_UINT		RBLXMATH_MEDIUMP
#elif defined(RBLXMATH_FORCE_PRECISION_LOWP_UINT) || defined(RBLXMATH_PRECISION_LOWP_UINT)
#	define RBLXMATH_CONFIG_PRECISION_UINT		RBLXMATH_LOWP
#else
#	define RBLXMATH_CONFIG_PRECISION_UINT		RBLXMATH_HIGHP
#endif

#if defined(RBLXMATH_FORCE_PRECISION_HIGHP_FLOAT) || defined(RBLXMATH_PRECISION_HIGHP_FLOAT)
#	define RBLXMATH_CONFIG_PRECISION_FLOAT		RBLXMATH_HIGHP
#elif defined(RBLXMATH_FORCE_PRECISION_MEDIUMP_FLOAT) || defined(RBLXMATH_PRECISION_MEDIUMP_FLOAT)
#	define RBLXMATH_CONFIG_PRECISION_FLOAT		RBLXMATH_MEDIUMP
#elif defined(RBLXMATH_FORCE_PRECISION_LOWP_FLOAT) || defined(RBLXMATH_PRECISION_LOWP_FLOAT)
#	define RBLXMATH_CONFIG_PRECISION_FLOAT		RBLXMATH_LOWP
#else
#	define RBLXMATH_CONFIG_PRECISION_FLOAT		RBLXMATH_HIGHP
#endif

#if defined(RBLXMATH_FORCE_PRECISION_HIGHP_DOUBLE) || defined(RBLXMATH_PRECISION_HIGHP_DOUBLE)
#	define RBLXMATH_CONFIG_PRECISION_DOUBLE		RBLXMATH_HIGHP
#elif defined(RBLXMATH_FORCE_PRECISION_MEDIUMP_DOUBLE) || defined(RBLXMATH_PRECISION_MEDIUMP_DOUBLE)
#	define RBLXMATH_CONFIG_PRECISION_DOUBLE		RBLXMATH_MEDIUMP
#elif defined(RBLXMATH_FORCE_PRECISION_LOWP_DOUBLE) || defined(RBLXMATH_PRECISION_LOWP_DOUBLE)
#	define RBLXMATH_CONFIG_PRECISION_DOUBLE		RBLXMATH_LOWP
#else
#	define RBLXMATH_CONFIG_PRECISION_DOUBLE		RBLXMATH_HIGHP
#endif

///////////////////////////////////////////////////////////////////////////////////
// Check inclusions of different versions of RBLXMATH

#elif ((RBLXMATH_SETUP_INCLUDED != RBLXMATH_VERSION) && !defined(RBLXMATH_FORCE_IGNORE_VERSION))
#	error "RBLXMATH error: A different version of RBLXMATH is already included. Define RBLXMATH_FORCE_IGNORE_VERSION before including RBLXMATH headers to ignore this error."
#elif RBLXMATH_SETUP_INCLUDED == RBLXMATH_VERSION

///////////////////////////////////////////////////////////////////////////////////
// Messages

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_MESSAGE_DISPLAYED)
#	define RBLXMATH_MESSAGE_DISPLAYED
#		define RBLXMATH_STR_HELPER(x) #x
#		define RBLXMATH_STR(x) RBLXMATH_STR_HELPER(x)

	// Report RBLXMATH version
#		pragma message (RBLXMATH_STR(RBLXMATH_VERSION_MESSAGE))

	// Report C++ language
#	if (RBLXMATH_LANG & RBLXMATH_LANG_CXX20_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 20 with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX20_FLAG)
#		pragma message("RBLXMATH: C++ 2A")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX17_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 17 with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX17_FLAG)
#		pragma message("RBLXMATH: C++ 17")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX14_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 14 with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX14_FLAG)
#		pragma message("RBLXMATH: C++ 14")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 11 with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX11_FLAG)
#		pragma message("RBLXMATH: C++ 11")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 0x with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX0X_FLAG)
#		pragma message("RBLXMATH: C++ 0x")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX03_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 03 with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX03_FLAG)
#		pragma message("RBLXMATH: C++ 03")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX98_FLAG) && (RBLXMATH_LANG & RBLXMATH_LANG_EXT)
#		pragma message("RBLXMATH: C++ 98 with extensions")
#	elif (RBLXMATH_LANG & RBLXMATH_LANG_CXX98_FLAG)
#		pragma message("RBLXMATH: C++ 98")
#	else
#		pragma message("RBLXMATH: C++ language undetected")
#	endif//RBLXMATH_LANG

	// Report compiler detection
#	if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CUDA
#		pragma message("RBLXMATH: CUDA compiler detected")
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_HIP
#		pragma message("RBLXMATH: HIP compiler detected")
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#		pragma message("RBLXMATH: Visual C++ compiler detected")
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#		pragma message("RBLXMATH: Clang compiler detected")
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_INTEL
#		pragma message("RBLXMATH: Intel Compiler detected")
#	elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC
#		pragma message("RBLXMATH: GCC compiler detected")
#	else
#		pragma message("RBLXMATH: Compiler not detected")
#	endif

	// Report build target
#	if (RBLXMATH_ARCH & RBLXMATH_ARCH_AVX2_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with AVX2 instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_AVX2_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with AVX2 instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_AVX_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with AVX instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_AVX_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with AVX instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE42_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with SSE4.2 instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE42_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with SSE4.2 instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE41_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with SSE4.1 instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE41_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with SSE4.1 instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSSE3_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with SSSE3 instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSSE3_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with SSSE3 instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE3_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with SSE3 instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE3_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with SSE3 instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits with SSE2 instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_SSE2_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits with SSE2 instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_X86_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: x86 64 bits build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_X86_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: x86 32 bits build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_NEON_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: ARM 64 bits with Neon instruction set build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_NEON_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: ARM 32 bits with Neon instruction set build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_ARM_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: ARM 64 bits build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_ARM_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: ARM 32 bits build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_MIPS_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: MIPS 64 bits build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_MIPS_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: MIPS 32 bits build target")

#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_PPC_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_64)
#		pragma message("RBLXMATH: PowerPC 64 bits build target")
#	elif (RBLXMATH_ARCH & RBLXMATH_ARCH_PPC_BIT) && (RBLXMATH_MODEL == RBLXMATH_MODEL_32)
#		pragma message("RBLXMATH: PowerPC 32 bits build target")
#	else
#		pragma message("RBLXMATH: Unknown build target")
#	endif//RBLXMATH_ARCH

	// Report platform name
#	if(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_QNXNTO)
#		pragma message("RBLXMATH: QNX platform detected")
//#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_IOS)
//#		pragma message("RBLXMATH: iOS platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_APPLE)
#		pragma message("RBLXMATH: Apple platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINCE)
#		pragma message("RBLXMATH: WinCE platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_WINDOWS)
#		pragma message("RBLXMATH: Windows platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_CHROME_NACL)
#		pragma message("RBLXMATH: Native Client detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_ANDROID)
#		pragma message("RBLXMATH: Android platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_LINUX)
#		pragma message("RBLXMATH: Linux platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_UNIX)
#		pragma message("RBLXMATH: UNIX platform detected")
#	elif(RBLXMATH_PLATFORM & RBLXMATH_PLATFORM_UNKNOWN)
#		pragma message("RBLXMATH: platform unknown")
#	else
#		pragma message("RBLXMATH: platform not detected")
#	endif

	// Report whether only xyzw component are used
#	if defined RBLXMATH_FORCE_XYZW_ONLY
#		pragma message("RBLXMATH: RBLXMATH_FORCE_XYZW_ONLY is defined. Only x, y, z and w component are available in vector type. This define disables swizzle operators and SIMD instruction sets.")
#	endif

	// Report swizzle operator support
#	if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SWIZZLE is defined, swizzling operators enabled.")
#	elif RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SWIZZLE is defined, swizzling functions enabled. Enable compiler C++ language extensions to enable swizzle operators.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SWIZZLE is undefined. swizzling functions or operators are disabled.")
#	endif

	// Report .length() type
#	if RBLXMATH_CONFIG_LENGTH_TYPE == RBLXMATH_LENGTH_SIZE_T
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SIZE_T_LENGTH is defined. .length() returns a rblxmath::length_t, a typedef of std::size_t.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SIZE_T_LENGTH is undefined. .length() returns a rblxmath::length_t, a typedef of int following GLSL.")
#	endif

#	if RBLXMATH_CONFIG_UNRESTRICTED_GENTYPE == RBLXMATH_ENABLE
#		pragma message("RBLXMATH: RBLXMATH_FORCE_UNRESTRICTED_GENTYPE is defined. Removes GLSL restrictions on valid function genTypes.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_FORCE_UNRESTRICTED_GENTYPE is undefined. Follows strictly GLSL on valid function genTypes.")
#	endif

#	if RBLXMATH_SILENT_WARNINGS == RBLXMATH_ENABLE
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SILENT_WARNINGS is defined. Ignores C++ warnings from using C++ language extensions.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SILENT_WARNINGS is undefined. Shows C++ warnings from using C++ language extensions.")
#	endif

#	ifdef RBLXMATH_FORCE_SINGLE_ONLY
#		pragma message("RBLXMATH: RBLXMATH_FORCE_SINGLE_ONLY is defined. Using only single precision floating-point types.")
#	endif

#	if defined(RBLXMATH_FORCE_ALIGNED_GENTYPES) && (RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_ENABLE)
#		undef RBLXMATH_FORCE_ALIGNED_GENTYPES
#		pragma message("RBLXMATH: RBLXMATH_FORCE_ALIGNED_GENTYPES is defined, allowing aligned types. This prevents the use of C++ constexpr.")
#	elif defined(RBLXMATH_FORCE_ALIGNED_GENTYPES) && (RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_DISABLE)
#		undef RBLXMATH_FORCE_ALIGNED_GENTYPES
#		pragma message("RBLXMATH: RBLXMATH_FORCE_ALIGNED_GENTYPES is defined but is disabled. It requires C++11 and language extensions.")
#	endif

#	if defined(RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES)
#		if RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_DISABLE
#			undef RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES
#			pragma message("RBLXMATH: RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES is defined but is disabled. It requires C++11 and language extensions.")
#		elif RBLXMATH_CONFIG_ALIGNED_GENTYPES == RBLXMATH_ENABLE
#			pragma message("RBLXMATH: RBLXMATH_FORCE_DEFAULT_ALIGNED_GENTYPES is defined. All gentypes (e.g. vec3) will be aligned and padded by default.")
#		endif
#	endif

#	if RBLXMATH_CONFIG_CLIP_CONTROL & RBLXMATH_CLIP_CONTROL_ZO_BIT
#		pragma message("RBLXMATH: RBLXMATH_FORCE_DEPTH_ZERO_TO_ONE is defined. Using zero to one depth clip space.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_FORCE_DEPTH_ZERO_TO_ONE is undefined. Using negative one to one depth clip space.")
#	endif

#	if RBLXMATH_CONFIG_CLIP_CONTROL & RBLXMATH_CLIP_CONTROL_LH_BIT
#		pragma message("RBLXMATH: RBLXMATH_FORCE_LEFT_HANDED is defined. Using left handed coordinate system.")
#	else
#		pragma message("RBLXMATH: RBLXMATH_FORCE_LEFT_HANDED is undefined. Using right handed coordinate system.")
#	endif
#endif//RBLXMATH_MESSAGES

#endif//RBLXMATH_SETUP_INCLUDED
