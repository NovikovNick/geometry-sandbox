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
#include <vector>

namespace gs
{
namespace
{
const std::size_t kMaxPolyDegree = 9;
}

/** @brief Evaluates bezier value at specified t using De Casteljau algorithm */
Vec3 calcBezierWithDeCasteljauAlgorithm(const std::vector<Vec3>& controlPoints, const float t)
{
	assert(controlPoints.size() >= 2);
	assert(controlPoints.size() <= kMaxPolyDegree);

	std::array<Vec3, kMaxPolyDegree> points;
	std::copy(controlPoints.begin(), controlPoints.end(), points.begin());

	// for cubic it should be like:
	// p0   p1   p2  p3
	//  \   /\   /\  /
	//   p01  p12  p23
	//    \   / \  /
	//    p012  p123
	//       \  /
	//       res
	int n = controlPoints.size();
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

float coefficientBinomial(const int n, const int k)
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

float bernsteinPolynomial(const int n, const int i, const float t)
{
	return coefficientBinomial(n, i) * std::pow(t, i) * std::pow(1.0F - t, n - i);
}

/** @brief Evaluates bezier value at specified time using De Casteljau algorithm */
Vec3 calcBezierWithBernsteinPolynomial(const std::vector<Vec3>& controlPoints, const float t)
{
	const int n = controlPoints.size() - 1;
	Vec3 res	= Vec3::Zero();

	for (int i = 0; i <= n; ++i)
	{
		res += bernsteinPolynomial(n, i, t) * controlPoints[i];
	}
	return res;
}

/** @brief Calculates Monomial coefficients */
void updateCoefficientMonomial(const std::vector<Vec3>& controlPoints, std::vector<Vec3>& monomials)
{
	// NOLINTBEGIN(*-magic-numbers)
	assert(controlPoints.size() == 4);
	assert(monomials.size() == 4);

	const Vec3& p0 = controlPoints[0];
	const Vec3& p1 = controlPoints[1];
	const Vec3& p2 = controlPoints[2];
	const Vec3& p3 = controlPoints[3];
	monomials[0]   = p0;
	monomials[1]   = -3 * p0 + 3 * p1;
	monomials[2]   = 3 * p0 - 6 * p1 + 3 * p2;
	monomials[3]   = -p0 + 3 * p1 - 3 * p2 + p3;
	// NOLINTEND(*-magic-numbers)
}
}  // namespace gs

#endif	// CURVE_SAMPLE_MATH_H
