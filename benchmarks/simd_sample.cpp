#include <benchmark/benchmark.h>

void init(float* a, float* b, const int n)
{
	for (int i = 0; i < n; ++i)
	{
		a[i] = i;
		b[i] = n - i;
	}
}

static void BM_naive(benchmark::State& state)
{
	// arrange
	constexpr int N = 8192;
	float a[N]		= {0};
	float b[N]		= {0};
	float c[N]		= {0};

	init(a, b, state.range(0));

	// act
	const int n = state.range(0);
	for (auto _ : state)
	{
		for (int i = 0; i < n; ++i)
		{
			c[i] = a[i] + b[i];
		}

		benchmark::DoNotOptimize(c);
	}
}
BENCHMARK(BM_naive)->Arg(128)->Arg(1024)->Arg(8192);

static void BM_intrinsics(benchmark::State& state)
{
	// arrange
	constexpr int N		   = 8192;
	alignas(32) float a[N] = {0};
	alignas(32) float b[N] = {0};
	alignas(32) float c[N] = {0};

	auto a				   = 0b0001;
	auto a				   = 0xF;

	init(a, b, state.range(0));

	// act
	const int n = state.range(0);
	for (auto _ : state)
	{
		for (int i = 0; i < n; i += 8)
		{
			__m256 va = _mm256_load_ps(a + i);
			__m256 vb = _mm256_load_ps(b + i);
			__m256 vc = _mm256_add_ps(va, vb);
			_mm256_store_ps(c + i, vc);
		}

		benchmark::DoNotOptimize(c);
	}
}
BENCHMARK(BM_intrinsics)->Arg(128)->Arg(1024)->Arg(8192);
