#include "heap.h"

#include <random>
#include <iostream>

static std::random_device seed_device;

struct MyHeapEntry
{
	int prio;
	HeapPosition pos_in_heap;
	bool erased;
};

struct MyHeapCompare
{
	bool operator()(MyHeapEntry * a, MyHeapEntry * b) const
	{
		return a->prio <= b->prio;
	}
};

struct MyHeapSetPosition
{
	void operator()(MyHeapEntry * entry, HeapPosition pos) const
	{
		entry->pos_in_heap = pos;
	}
};

using MyHeap = Heap<MyHeapEntry *, MyHeapCompare, MyHeapSetPosition>;

void validate_heap_positions(MyHeap const & my_heap, std::vector<MyHeapEntry> const & entries)
{
	std::cout << __func__ << " BEGIN\n";
	int const cur_size = my_heap.size();
	std::vector<bool> seen(cur_size);
	for (int heap_pos = 1; heap_pos <= cur_size; ++heap_pos)
	{
		MyHeapEntry * entry = my_heap.at(HeapPosition{heap_pos});
		int const idx = entry - &entries[0];
		//std::cout << "entry with prio " << entry->prio
		//	<< " and idx " << idx
		//	<< " is at heap position " << heap_pos << "\n";
		assert(!entry->erased);
		assert(entry->pos_in_heap.val == heap_pos);
		assert(!seen[idx]);
		seen[idx] = true;
		if (heap_pos > 1)
		{
			int const parent_pos = heap_pos / 2;
			MyHeapEntry * parent_entry = my_heap.at(HeapPosition{parent_pos});
			assert(MyHeapCompare()(parent_entry, entry) == true);
		}
	}
	std::cout << __func__ << " END\n";
}

void test_heap()
{
	auto seed = seed_device();
	std::default_random_engine rnd(seed);
	std::cout << "BEGIN " << __func__ << ", seed=" << seed << "\n";

	MyHeap my_heap;
	assert(my_heap.empty());

	std::uniform_int_distribution<> heap_size_distrib_small(1, 10);
	std::uniform_int_distribution<> heap_size_distrib_large(5, 100);
	std::uniform_int_distribution<> priority_distrib(-100, 1000);
	for (int test = 0; test < 10; ++test)
	{
		int const n = test < 5 ? heap_size_distrib_small(rnd) : heap_size_distrib_large(rnd);
		std::cout << __func__ << " test no " << test << ", building heap with " << n << " elements\n";

		// Fill priorities.
		std::vector<MyHeapEntry> entries(n);
		for (int i = 0; i < n; ++i)
		{
			entries[i].prio = priority_distrib(rnd);
		}

		// Build heap containing pointers to entries.
		my_heap.clear();
		assert(my_heap.empty());
		for (int i = 0; i < n; ++i)
		{
			my_heap.uninitializedAdd(&entries[i]);
		}
		my_heap.initialize();

		assert(!my_heap.empty());
		assert(my_heap.size() == n);

		validate_heap_positions(my_heap, entries);

		// decrease priority of some elements
		for (int i = 0; i < n; ++i)
		{
			if (i % 3 == 0)
			{
				MyHeapEntry * entry = &entries[i];
				int const new_prio = priority_distrib(rnd);
				if (new_prio < entry->prio)
				{
					std::cout << "decreasing priority of entry idx " << i
						<< " heap position " << entry->pos_in_heap.val << " : "
						<< entry->prio << " => " << new_prio << "\n";
					entry->prio = new_prio;
					my_heap.keyChangedTowardsTop(entry->pos_in_heap);
				}
			}
		}

		validate_heap_positions(my_heap, entries);

		// erase some elements
		int num_erased = 0;
		for (int i = 0; i < n; ++i)
		{
			if (i % 7 == 0)
			{
				MyHeapEntry * entry = &entries[i];
				std::cout << "erasing entry with prio " << entry->prio
					<< " idx " << i
					<< " heap position " << entry->pos_in_heap.val << "\n";
				my_heap.erase(entry->pos_in_heap);
				assert(entry->pos_in_heap.val == 0);
				entry->erased = true;
				++num_erased;
			}
		}
		std::cout << "erased " << num_erased << " elements from heap\n";

		validate_heap_positions(my_heap, entries);

		int const cur_size = my_heap.size();
		std::cout << "heap size now: " << cur_size << "\n";
		assert(cur_size == n - num_erased);
		int last_prio = std::numeric_limits<int>::min();
		for (int i = 0; i < cur_size; ++i)
		{
			MyHeapEntry * entry = my_heap.extract();
			//int const idx = entry - &entries[0];
			//std::cout << "i=" << i << " extracted entry with prio " << entry->prio
			//	<< " and idx " << idx << "\n";
			assert(!entry->erased);
			assert(entry->pos_in_heap.val == 0);
			assert(last_prio <= entry->prio);
			last_prio = entry->prio;
		}
		assert(my_heap.empty());
	}

	std::cout << "END " << __func__ << "\n";
}

int main()
{
	test_heap();
}
