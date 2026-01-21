#include <benchmark/benchmark.h>

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

static void BM_Int_Increment(benchmark::State& state)
{
	int x = 0;
	while (state.KeepRunning())
	{
		++x;
	}
}
BENCHMARK(BM_Int_Increment);

static void BM_Array_Creation(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		[[maybe_unused]] std::array<char, 1000000> arr;
	}
}
BENCHMARK(BM_Array_Creation);

static void BM_Int_Division(benchmark::State& state)
{
	int x = 123123123;
	while (state.KeepRunning())
	{
		[[maybe_unused]] int y = x / 3;
	}
}
BENCHMARK(BM_Int_Division);

static void BM_Double_Division(benchmark::State& state)
{
	double x = 123123123123;
	while (state.KeepRunning())
	{
		[[maybe_unused]] double y = x / 3;
	}
}
BENCHMARK(BM_Double_Division);

static void BM_Lambda_Creation(benchmark::State& state)
{
	int x = 0;
	while (state.KeepRunning())
	{
		auto lambda = [&x]() { ++x; };
		lambda();
	}
}
BENCHMARK(BM_Lambda_Creation);

static void BM_Atomic_FetchAdd(benchmark::State& state)
{
	std::atomic_int x = 0;
	while (state.KeepRunning())
	{
		x.fetch_add(1);
	}
}
BENCHMARK(BM_Atomic_FetchAdd);

static void BM_Lock(benchmark::State& state)
{
	std::mutex mx;
	while (state.KeepRunning())
	{
		[[maybe_unused]] std::unique_lock lock{mx};
	}
}
BENCHMARK(BM_Lock);

static void BM_String_Creation(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		std::string{"0123456789"};
	}
}
BENCHMARK(BM_String_Creation);

static void BM_UniquePtr_Creation(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		[[maybe_unused]] auto ptr = std::make_unique<int>(1);
	}
}
BENCHMARK(BM_UniquePtr_Creation);

static void BM_Function_Creation(benchmark::State& state)
{
	int x = 0;
	while (state.KeepRunning())
	{
		std::function<void(void)> func = [&x]() { ++x; };
		func();
	}
}
BENCHMARK(BM_Function_Creation);

static void BM_Vector_Creation(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		[[maybe_unused]] std::vector<char> arr(state.range(0), 0);
	}
}
BENCHMARK(BM_Vector_Creation)
	->Arg(1000)	 // less has the same time
	->Arg(10000)
	->Arg(100000)
	->Arg(1000000);	 // more is better to execute to another thread

struct Node
{
	int val	   = 0;
	Node* next = nullptr;
};

static void BM_LinkedList_Access(benchmark::State& state)
{
	Node dummy;
	{
		Node* curr = &dummy;
		for (int i = 0; i < state.range(0); ++i)
		{
			curr->next = new Node();
			curr	   = curr->next;
		}
	}

	while (state.KeepRunning())
	{
		Node* curr = &dummy;
		while (curr)
		{
			curr = curr->next;
		}
	}
}
BENCHMARK(BM_LinkedList_Access)	 //
	->Arg(1)
	->Arg(10)
	->Arg(100)
	->Arg(1000)
	->Arg(10000);

static void BM_String_Hash(benchmark::State& state)
{
	std::string str = "";
	for (int i = 0; i < state.range(0); ++i)
	{
		str += '#';
	}
	std::hash<std::string> hasher;
	while (state.KeepRunning())
	{
		[[maybe_unused]] auto hash = hasher(str);
	}
}
BENCHMARK(BM_String_Hash)  //
	->Arg(1)
	->Arg(10)
	->Arg(100)
	->Arg(1000)
	->Arg(10000)
	->Arg(100000);

static void BM_Thread_Creation(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		std::jthread([]() {});
	}
}
BENCHMARK(BM_Thread_Creation);
