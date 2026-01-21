#ifndef GEOMETRY_SANDBOX_EASING_H
#define GEOMETRY_SANDBOX_EASING_H

/*
 * Derived from Robert Penner's easing equations: http://robertpenner.com/easing/
 *
 * Copyright (c) 2001 Robert Penner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cmath>
#include <numbers>

namespace detail
{
using real_t = float;

namespace linear
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return c * t / d + b;
}
};	// namespace linear

namespace sine
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return -c * std::cos(t / d * (std::numbers::pi / 2)) + c + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	return c * std::sin(t / d * (std::numbers::pi / 2)) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	return -c / 2 * (std::cos(std::numbers::pi * t / d) - 1) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace sine

namespace quint
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return c * std::pow(t / d, 5) + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	return c * (std::pow(t / d - 1, 5) + 1) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	t = t / d * 2;

	if (t < 1)
	{
		return c / 2 * std::pow(t, 5) + b;
	}
	return c / 2 * (std::pow(t - 2, 5) + 2) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace quint

namespace quart
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return c * std::pow(t / d, 4) + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	return -c * (std::pow(t / d - 1, 4) - 1) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	t = t / d * 2;

	if (t < 1)
	{
		return c / 2 * std::pow(t, 4) + b;
	}
	return -c / 2 * (std::pow(t - 2, 4) - 2) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace quart

namespace quad
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return c * std::pow(t / d, 2) + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	t /= d;
	return -c * t * (t - 2) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	t = t / d * 2;

	if (t < 1)
	{
		return c / 2 * std::pow(t, 2) + b;
	}
	return -c / 2 * ((t - 1) * (t - 3) - 1) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace quad

namespace expo
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	if (t == 0)
	{
		return b;
	}
	return c * std::pow(2, 10 * (t / d - 1)) + b - c * 0.001;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	if (t == d)
	{
		return b + c;
	}
	return c * 1.001 * (-std::pow(2, -10 * t / d) + 1) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	if (t == 0)
	{
		return b;
	}

	if (t == d)
	{
		return b + c;
	}

	t = t / d * 2;

	if (t < 1)
	{
		return c / 2 * std::pow(2, 10 * (t - 1)) + b - c * 0.0005;
	}
	return c / 2 * 1.0005 * (-std::pow(2, -10 * (t - 1)) + 2) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace expo

namespace elastic
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	if (t == 0)
	{
		return b;
	}

	t /= d;
	if (t == 1)
	{
		return b + c;
	}

	t -= 1;
	float p = d * 0.3f;
	float a = c * std::pow(2, 10 * t);
	float s = p / 4;

	return -(a * std::sin((t * d - s) * (2 * std::numbers::pi) / p)) + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	if (t == 0)
	{
		return b;
	}

	t /= d;
	if (t == 1)
	{
		return b + c;
	}

	float p = d * 0.3f;
	float s = p / 4;

	return (c * std::pow(2, -10 * t) * std::sin((t * d - s) * (2 * std::numbers::pi) / p) + c + b);
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	if (t == 0)
	{
		return b;
	}

	if ((t /= d / 2) == 2)
	{
		return b + c;
	}

	float p = d * (0.3f * 1.5f);
	float a = c;
	float s = p / 4;

	if (t < 1)
	{
		t -= 1;
		a *= std::pow(2, 10 * t);
		return -0.5f * (a * std::sin((t * d - s) * (2 * std::numbers::pi) / p)) + b;
	}

	t -= 1;
	a *= std::pow(2, -10 * t);
	return a * std::sin((t * d - s) * (2 * std::numbers::pi) / p) * 0.5f + c + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace elastic

namespace cubic
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	t /= d;
	return c * t * t * t + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	t = t / d - 1;
	return c * (t * t * t + 1) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	t /= d / 2;
	if (t < 1)
	{
		return c / 2 * t * t * t + b;
	}

	t -= 2;
	return c / 2 * (t * t * t + 2) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace cubic

namespace circ
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	t /= d;
	return -c * (std::sqrt(1 - t * t) - 1) + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	t = t / d - 1;
	return c * std::sqrt(1 - t * t) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	t /= d / 2;
	if (t < 1)
	{
		return -c / 2 * (std::sqrt(1 - t * t) - 1) + b;
	}

	t -= 2;
	return c / 2 * (std::sqrt(1 - t * t) + 1) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace circ

namespace bounce
{
inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	t /= d;

	if (t < (1 / 2.75f))
	{
		return c * (7.5625f * t * t) + b;
	}

	if (t < (2 / 2.75f))
	{
		t -= 1.5f / 2.75f;
		return c * (7.5625f * t * t + 0.75f) + b;
	}

	if (t < (2.5 / 2.75))
	{
		t -= 2.25f / 2.75f;
		return c * (7.5625f * t * t + 0.9375f) + b;
	}

	t -= 2.625f / 2.75f;
	return c * (7.5625f * t * t + 0.984375f) + b;
}

inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return c - out(d - t, 0, c, d) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return in(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return out(t * 2 - d, b + h, h, d);
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace bounce

namespace back
{
inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	float s = 1.70158f;
	t /= d;

	return c * t * t * ((s + 1) * t - s) + b;
}

inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	float s = 1.70158f;
	t		= t / d - 1;

	return c * (t * t * ((s + 1) * t + s) + 1) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	float s = 1.70158f * 1.525f;
	t /= d / 2;

	if (t < 1)
	{
		return c / 2 * (t * t * ((s + 1) * t - s)) + b;
	}

	t -= 2;
	return c / 2 * (t * t * ((s + 1) * t + s) + 2) + b;
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace back

namespace spring
{
inline real_t out(real_t t, real_t b, real_t c, real_t d)
{
	t /= d;
	real_t s = 1.0 - t;
	t		 = (std::sin(t * std::numbers::pi * (0.2 + 2.5 * t * t * t)) * std::pow(s, 2.2) + t) * (1.0 + (1.2 * s));
	return c * t + b;
}

inline real_t in(real_t t, real_t b, real_t c, real_t d)
{
	return c - out(d - t, 0, c, d) + b;
}

inline real_t in_out(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return in(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return out(t * 2 - d, b + h, h, d);
}

inline real_t out_in(real_t t, real_t b, real_t c, real_t d)
{
	if (t < d / 2)
	{
		return out(t * 2, b, c / 2, d);
	}
	real_t h = c / 2;
	return in(t * 2 - d, b + h, h, d);
}
};	// namespace spring

}  // namespace detail

namespace gs
{
using real_t = float;

// clang-format off
namespace linear
{
inline real_t in(real_t t) { return t; }
};	// namespace linear

namespace sine
{
inline real_t in(real_t t) { return detail::sine::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::sine::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::sine::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::sine::out_in(t, 0, 1, 1); }
}  // namespace sine

namespace quint
{
inline real_t in(real_t t) { return detail::quint::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::quint::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::quint::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::quint::out_in(t, 0, 1, 1); }
}  // namespace quint

namespace quart
{
inline real_t in(real_t t) { return detail::quart::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::quart::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::quart::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::quart::out_in(t, 0, 1, 1); }
}  // namespace quart

namespace quad
{
inline real_t in(real_t t) { return detail::quad::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::quad::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::quad::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::quad::out_in(t, 0, 1, 1); }
}  // namespace quad

namespace expo
{
inline real_t in(real_t t) { return detail::expo::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::expo::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::expo::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::expo::out_in(t, 0, 1, 1); }
}  // namespace expo

namespace elastic
{
inline real_t in(real_t t) { return detail::elastic::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::elastic::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::elastic::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::elastic::out_in(t, 0, 1, 1); }
}  // namespace elastic

namespace cubic
{
inline real_t in(real_t t) { return detail::cubic::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::cubic::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::cubic::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::cubic::out_in(t, 0, 1, 1); }
}  // namespace cubic

namespace circ
{
inline real_t in(real_t t) { return detail::circ::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::circ::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::circ::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::circ::out_in(t, 0, 1, 1); }
}  // namespace circ

namespace bounce
{
inline real_t in(real_t t) { return detail::bounce::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::bounce::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::bounce::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::bounce::out_in(t, 0, 1, 1); }
}  // namespace bounce

namespace back
{
inline real_t in(real_t t) { return detail::back::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::back::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::back::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::back::out_in(t, 0, 1, 1); }
}  // namespace back

namespace spring
{
inline real_t in(real_t t) { return detail::spring::in(t, 0, 1, 1); }
inline real_t out(real_t t) { return detail::spring::out(t, 0, 1, 1); }
inline real_t in_out(real_t t) { return detail::spring::in_out(t, 0, 1, 1); }
inline real_t out_in(real_t t) { return detail::spring::out_in(t, 0, 1, 1); }
}  // namespace spring

// clang-format on

}  // namespace gs
#endif	// GEOMETRY_SANDBOX_EASING_H
