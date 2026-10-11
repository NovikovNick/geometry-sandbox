/**
 * @file curve_math.h
 * @brief helper functions for curve sample
 * @author MetalHeart
 */
#ifndef CURVE_SAMPLE_MATH_H
#define CURVE_SAMPLE_MATH_H

#include "core/math.h"
#include "core/types.h"

#include <array>
#include <cassert>
#include <cmath>
#include <span>
#include <vector>

namespace gs
{
const std::size_t kMaxPolyDegree = 9;

/** @brief Evaluates bezier value at specified t using De Casteljau algorithm */
inline Vec3 evalBezierByDeCasteljau(std::span<Vec3> poles, const float t)
{
	assert(poles.size() >= 2);
	assert(poles.size() <= kMaxPolyDegree);

	std::array<Vec3, kMaxPolyDegree> points;
	std::copy(poles.begin(), poles.end(), points.begin());

	// for cubic it should be like:
	// p0   p1   p2  p3
	//  \   /\   /\  /
	//   p01  p12  p23
	//    \   / \  /
	//    p012  p123
	//       \  /
	//       res
	int n = poles.size();
	while (n > 0)
	{
		--n;
		for (int i = 0; i < n; ++i)
		{
			points[i] = std::lerp(points[i], points[i + 1], t);
		}
	}
	return points[0];
}

/** @brief Evaluates cubic bezier derivative at specified time */
inline Vec3 evalDerivate(const std::span<Vec3>& poles, const float t)
{
	assert(poles.size() == 4);
	const Vec3 p01 = std::lerp(poles[0], poles[1], t);
	const Vec3 p12 = std::lerp(poles[1], poles[2], t);
	const Vec3 p23 = std::lerp(poles[2], poles[3], t);
	return 3.0f * (std::lerp(p12, p23, t) - std::lerp(p01, p12, t));
}

constexpr int factorial(const int n)
{
	if (n <= 1)
	{
		return 1;
	}

	int res = n;
	for (int i = 1; i < n; ++i)
	{
		res *= i;
	}
	return res;
}

inline float coefficientBinomial(const int n, const int k)
{
	if (n < kMaxPolyDegree && n <= k)
	{
		static const std::array<std::array<int32_t, 10>, 10> coefficientsBinomiaux = {{{1},
																					   {1, 1},
																					   {1, 2, 1},
																					   {1, 3, 3, 1},
																					   {1, 4, 6, 4, 1},
																					   {1, 5, 10, 10, 5, 1},
																					   {1, 6, 15, 20, 15, 6, 1},
																					   {1, 7, 21, 35, 35, 21, 7, 1},
																					   {1, 8, 28, 56, 70, 56, 28, 8, 1},
																					   {1, 9, 36, 84, 126, 126, 84, 36, 9, 1}}};

		return coefficientsBinomiaux[n][k];
	}
	return factorial(n) / static_cast<float>(factorial(k) * factorial(n - k));
}

inline float bernsteinPolynomial(const int n, const int i, const float t)
{
	return coefficientBinomial(n, i) * std::pow(t, i) * std::pow(1.0F - t, n - i);
}

/**
 * @brief Evaluates bezier value at specified time using Bernstein polynomial
 *
 * Bernstein polynomial is just rearranged DeCasteljau
 */
inline Vec3 evalBezierByBernstein(std::span<Vec3> poles, const float t)
{
	const int n = poles.size() - 1;
	if (n == 3)
	{
		const float t2 = t * t;
		const float t3 = t2 * t;
		return poles[0] * (-t3 + 3 * t2 - 3 * t + 1) +	//
			   poles[1] * (3 * t3 - 6 * t2 + 3 * t) +	//
			   poles[2] * (-3 * t3 + 3 * t2) +			//
			   poles[3] * t3;
	}

	Vec3 res = Vec3::Zero();
	for (int i = 0; i <= n; ++i)
	{
		res += bernsteinPolynomial(n, i, t) * poles[i];
	}
	return res;
}

/** @brief Calculates polinomial coefficients for cubic bezier curve */
inline std::tuple<Vec3, Vec3, Vec3, Vec3> polinomialCoefficients(std::span<Vec3> poles)
{
	// NOLINTBEGIN(*-magic-numbers)
	assert(poles.size() == 4);

	const Vec3& p0 = poles[0];
	const Vec3& p1 = poles[1];
	const Vec3& p2 = poles[2];
	const Vec3& p3 = poles[3];

	return std::make_tuple(p0,						  //
						   -3 * p0 + 3 * p1,		  //
						   3 * p0 - 6 * p1 + 3 * p2,  //
						   -p0 + 3 * p1 - 3 * p2 + p3);

	// NOLINTEND(*-magic-numbers)
}

inline Vec3 evalBezierByPolinomialCoefficients(std::span<Vec3> poles, const float t)
{
	const auto [a1, a2, a3, a4] = polinomialCoefficients(poles);

	return a1 +			 //
		   a2 * t +		 //
		   a3 * t * t +	 //
		   a4 * t * t * t;
}
}  // namespace gs

#endif	// CURVE_SAMPLE_MATH_H
