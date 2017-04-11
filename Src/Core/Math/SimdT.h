// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"

#define RIO_SIMD_FORCE_INLINE RIO_FORCE_INLINE
#define RIO_SIMD_INLINE inline

#define RIO_SIMD_AVX 0
#define RIO_SIMD_LANGEXT 0
#define RIO_SIMD_NEON 0
#define RIO_SIMD_SSE 0

#if defined(__AVX__) || defined(__AVX2__)
#include <immintrin.h>
#undef  RIO_SIMD_AVX
#define RIO_SIMD_AVX 1
#endif

#if defined(__SSE2__) || (RIO_COMPILER_MSVC && (RIO_ARCH_64BIT || _M_IX86_FP >= 2) )
#include <emmintrin.h> // __m128i
#if defined(__SSE4_1__)
#include <smmintrin.h>
#endif // defined(__SSE4_1__)
#include <xmmintrin.h> // __m128
#undef  RIO_SIMD_SSE
#define RIO_SIMD_SSE 1
#elif defined(__ARM_NEON__) && !RIO_COMPILER_CLANG
#include <arm_neon.h>
#undef  RIO_SIMD_NEON
#define RIO_SIMD_NEON 1
#elif RIO_COMPILER_CLANG \
	&& !RIO_PLATFORM_EMSCRIPTEN \
	&& !RIO_PLATFORM_IOS \
	&&  RIO_CLANG_HAS_EXTENSION(attribute_ext_vector_type)
#include <math.h>
#undef RIO_SIMD_LANGEXT
#define RIO_SIMD_LANGEXT 1
#endif

namespace RioCore
{
#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define RIO_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			template<typename Ty> \
			RIO_SIMD_FORCE_INLINE Ty simd_swiz_##_x##_y##_z##_w(Ty _a);
#include "Simd128Swizzle.inl"

#undef RIO_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define RIO_SIMD128_IMPLEMENT_TEST(_xyzw) \
			template<typename Ty> \
			RIO_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(Ty _test); \
			\
			template<typename Ty> \
			RIO_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(Ty _test)

RIO_SIMD128_IMPLEMENT_TEST(x);
RIO_SIMD128_IMPLEMENT_TEST(y);
RIO_SIMD128_IMPLEMENT_TEST(xy);
RIO_SIMD128_IMPLEMENT_TEST(z);
RIO_SIMD128_IMPLEMENT_TEST(xz);
RIO_SIMD128_IMPLEMENT_TEST(yz);
RIO_SIMD128_IMPLEMENT_TEST(xyz);
RIO_SIMD128_IMPLEMENT_TEST(w);
RIO_SIMD128_IMPLEMENT_TEST(xw);
RIO_SIMD128_IMPLEMENT_TEST(yw);
RIO_SIMD128_IMPLEMENT_TEST(xyw);
RIO_SIMD128_IMPLEMENT_TEST(zw);
RIO_SIMD128_IMPLEMENT_TEST(xzw);
RIO_SIMD128_IMPLEMENT_TEST(yzw);
RIO_SIMD128_IMPLEMENT_TEST(xyzw);
#undef RIO_SIMD128_IMPLEMENT_TEST

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_xyAB(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_ABxy(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_CDzw(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_zwCD(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_xAyB(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_yBxA(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_zCwD(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_shuf_CzDw(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE float simd_x(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE float simd_y(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE float simd_z(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE float simd_w(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_ld(const void* _ptr);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE void simd_st(void* _ptr, Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE void simd_stx(void* _ptr, Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE void simd_stream(void* _ptr, Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_ld(float _x, float _y, float _z, float _w);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_ld(float _x, float _y, float _z, float _w, float _a, float _b, float _c, float _d);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_splat(const void* _ptr);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_splat(float _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_isplat(uint32_t _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_zero();

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_itof(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_ftoi(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_round(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_add(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_sub(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_mul(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_div(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_rcp_est(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_sqrt(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_rsqrt_est(Ty _a);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_dot3(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_dot(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_cmpeq(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_cmplt(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_cmple(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_cmpgt(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_cmpge(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_min(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_max(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_and(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_andc(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_or(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_xor(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_sll(Ty _a, int _count);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_srl(Ty _a, int _count);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_sra(Ty _a, int _count);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_icmpeq(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_icmplt(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_icmpgt(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_imin(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_imax(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_iadd(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_FORCE_INLINE Ty simd_isub(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_shuf_xAzC(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_shuf_yBwD(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_rcp(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_orx(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_orc(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_neg(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_madd(Ty _a, Ty _b, Ty _c);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_nmsub(Ty _a, Ty _b, Ty _c);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_div_nr(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_selb(Ty _mask, Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_sels(Ty _test, Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_not(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_abs(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_clamp(Ty _a, Ty _min, Ty _max);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_lerp(Ty _a, Ty _b, Ty _s);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_rsqrt(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_rsqrt_nr(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_rsqrt_carmack(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_sqrt_nr(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_log2(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_exp2(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_pow(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_cross3(Ty _a, Ty _b);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_normalize3(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_ceil(Ty _a);

	template<typename Ty>
	RIO_SIMD_INLINE Ty simd_floor(Ty _a);

#if RIO_SIMD_AVX
	typedef __m256 simd256_avx_t;
#endif // RIO_SIMD_SSE

#if RIO_SIMD_LANGEXT
	union simd128_langext_t
	{
		float __attribute__((vector_size(16))) vf;
		int32_t  __attribute__((vector_size(16))) vi;
		uint32_t __attribute__((vector_size(16))) vu;
		float fxyzw[4];
		int32_t  ixyzw[4];
		uint32_t uxyzw[4];

	};
#endif // RIO_SIMD_LANGEXT

#if RIO_SIMD_NEON
	typedef float32x4_t simd128_neon_t;
#endif // RIO_SIMD_NEON

#if RIO_SIMD_SSE
	typedef __m128 simd128_sse_t;
#endif // RIO_SIMD_SSE

} // namespace RioCore

#if RIO_SIMD_AVX
#include "Simd256Avx.inl"
#endif // RIO_SIMD_AVX

#if RIO_SIMD_LANGEXT
#include "Simd128LangExt.inl"
#endif // RIO_SIMD_LANGEXT

#if RIO_SIMD_NEON
#include "Simd128Neon.inl"
#endif // RIO_SIMD_NEON

#if RIO_SIMD_SSE
#include "Simd128Sse.inl"
#endif // RIO_SIMD_SSE

namespace RioCore
{
	union simd128_ref_t
	{
		float fxyzw[4];
		int32_t ixyzw[4];
		uint32_t uxyzw[4];
	};

#ifndef RIO_SIMD_WARN_REFERENCE_IMPL
#define RIO_SIMD_WARN_REFERENCE_IMPL 0
#endif // RIO_SIMD_WARN_REFERENCE_IMPL

#if !( RIO_SIMD_LANGEXT \
	|| RIO_SIMD_NEON \
	|| RIO_SIMD_SSE \
	 )
#if RIO_SIMD_WARN_REFERENCE_IMPL
#pragma message("Using SIMD128 reference implementation")
#endif // RIO_SIMD_WARN_REFERENCE_IMPL

	typedef simd128_ref_t simd128_t;
#endif //

	struct simd256_ref_t
	{
		simd128_t simd128_0;
		simd128_t simd128_1;
	};

#if !RIO_SIMD_AVX
#if RIO_SIMD_WARN_REFERENCE_IMPL
#pragma message("Using SIMD256 reference implementation")
#endif // RIO_SIMD_WARN_REFERENCE_IMPL

	typedef simd256_ref_t simd256_t;
#endif // !RIO_SIMD_AVX

} // namespace RioCore

#include "Simd128Ref.inl"
#include "Simd256Ref.inl"

namespace RioCore
{
	RIO_SIMD_FORCE_INLINE simd128_t simd_zero()
	{
		return simd_zero<simd128_t>();
	}

	RIO_SIMD_FORCE_INLINE simd128_t simd_ld(const void* _ptr)
	{
		return simd_ld<simd128_t>(_ptr);
	}

	RIO_SIMD_FORCE_INLINE simd128_t simd_ld(float _x, float _y, float _z, float _w)
	{
		return simd_ld<simd128_t>(_x, _y, _z, _w);
	}

	RIO_SIMD_FORCE_INLINE simd128_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		return simd_ild<simd128_t>(_x, _y, _z, _w);
	}

	RIO_SIMD_FORCE_INLINE simd128_t simd_splat(const void* _ptr)
	{
		return simd_splat<simd128_t>(_ptr);
	}

	RIO_SIMD_FORCE_INLINE simd128_t simd_splat(float _a)
	{
		return simd_splat<simd128_t>(_a);
	}

	RIO_SIMD_FORCE_INLINE simd128_t simd_isplat(uint32_t _a)
	{
		return simd_isplat<simd128_t>(_a);
	}
} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka