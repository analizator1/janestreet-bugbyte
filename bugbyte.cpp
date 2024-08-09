#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

using UintVec = std::vector<uint>;

static std::random_device seed_device;

template<class T>
std::ostream & operator<<(std::ostream & out, std::vector<T> const & vec)
{
	out << "{ ";
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (it != vec.begin())
		{
			out << ", ";
		}
		out << *it;
	}
	out << " }";
	return out;
}

class PermutationsWithSumGenerator
{
public:
	/** Generates permutations of input vector v.
	 *
	 * Params:
	 * v           input vector with non-negative unique numbers, sorted in ascending order
	 * k           length of permutations
	 * target_sum  the sum of elements of each generated permutation
	 */
	PermutationsWithSumGenerator(UintVec const & v, uint k, uint target_sum, std::function<void(UintVec const &)> callback):
		v(v),
		used(v.size()),
		perm(k),
		k(k),
		target_sum(target_sum),
		callback(callback)
	{
		assert(k > 0);
		assert(std::is_sorted(v.begin(), v.end()));
	}

	void run()
	{
		if (k > v.size())
		{
			return;
		}
		uint max_possible_sum = 0;
		for (uint i = v.size() - k; i < v.size(); ++i)
		{
			max_possible_sum += v[i];
		}
		if (max_possible_sum < target_sum)
		{
			return;
		}
		do_run(0, 0);
	}

private:
	void do_run(uint pos, uint cur_sum)
	{
		assert(pos < k);
		assert(cur_sum <= target_sum);
		if (pos == k - 1)
		{
			uint needed = target_sum - cur_sum;
			auto it = std::lower_bound(v.begin(), v.end(), needed);
			if (it != v.end() && *it == needed)
			{
				if (!used[it - v.begin()])
				{
					perm[pos] = needed;
					callback(perm);
				}
			}
		}
		else
		{
			for (uint i = 0; i < v.size(); ++i)
			{
				if (cur_sum + v[i] > target_sum)
				{
					break;
				}
				if (!used[i])
				{
					perm[pos] = v[i];
					used[i] = true;
					do_run(pos + 1, cur_sum + v[i]);
					used[i] = false;
				}
			}
		}
	}

	UintVec const & v;
	std::vector<bool> used;
	UintVec perm;
	uint const k;
	uint const target_sum;
	std::function<void(UintVec const &)> callback;
};

void test_permutations_case(UintVec const & v, uint k, uint target_sum)
{
	std::cout << "Permutations of v=" << v << " with " << k << " elements and sum=" << target_sum << " are:\n";
	PermutationsWithSumGenerator generator(v, k, target_sum, [](UintVec const & perm) {
		std::cout << "=> " << perm << "\n";
	});
	generator.run();
}

void test_permutations()
{
	std::cout << "BEGIN " << __func__ << "\n";

	UintVec v(10);
	for (uint i = 0; i < 10; ++i)
	{
		v[i] = i + 1;
	}
	test_permutations_case(v, 2, 40);
	test_permutations_case(v, 2, 19);
	test_permutations_case(v, 3, 19);
	test_permutations_case(v, 1, 42);
	test_permutations_case(v, 1, 8);
	test_permutations_case(v, 1, 10);
	test_permutations_case(v, 2, 10);

	std::cout << "END " << __func__ << "\n";
}

struct HeapPosition
{
	int val;
};

/**
 * Represents a heap.
 *
 * This can be a max heap if Compare is std::greater_equal
 * or a min heap if Compare is std::less_equal.
 *
 * T - type of elements kept in heap; must be default constructible
 * Compare - defines heap property:
 *           cmp(heap[parent(i)], heap[i]) == true   for every i > 1
 *           cmp(heap[i], heap[i]) == true           for every i >= 1
 * SetPosition - saves position in heap when called as:
 *               setPosition(heap[i], HeapPosition{i})   where i >= 1
 *               or setPosition(elem, HeapPosition{0})   when elem goes out of heap
 */
template<class T, class Compare, class SetPosition>
class Heap
{
public:
	Heap(Compare cmp = Compare(), SetPosition setPosition = SetPosition()):
		heap(1),
		cmp(cmp),
		setPosition(setPosition)
	{
	}

	Heap(Heap const &) = delete;

	template<class Iter>
	void buildFromRange(Iter first, Iter last)
	{
		int const n = std::distance(first, last);
		heap.resize(1 + n);
		for (int i = 1; i <= n; ++i)
		{
			heap[i] = *first++;
			setPosition(heap[i], HeapPosition{i});
		}
		buildHeap();
	}

	void clear()
	{
		heap.resize(1);
	}

	void uninitializedAdd(T elem)
	{
		heap.push_back(std::move(elem));
		int const n = size();
		setPosition(heap[n], HeapPosition{n});
	}

	void initialize()
	{
		buildHeap();
	}

	int size() const
	{
		assert(heap.size() >= 1);
		return heap.size() - 1;
	}

	bool empty() const
	{
		return size() == 0;
	}

	T const & at(HeapPosition const pos) const
	{
		int i = pos.val;
		assert(i >= 1);
		assert(i <= size());
		return heap[i];
	}

	T extract()
	{
		return extractFrom(1);
	}

	void erase(HeapPosition const pos)
	{
		extractFrom(pos.val);
	}

	void insert(T elem)
	{
		heap.push_back(std::move(elem));
		heapifyUp(size());
	}

	// in a min heap this would be called keyDecreased
	void keyChangedTowardsTop(HeapPosition const pos)
	{
		heapifyUp(pos.val);
	}

private:
	static int parent(int i)
	{
		assert(i > 1);
		return i / 2;
	}

	static int left(int i)
	{
		assert(i >= 1);
		return 2 * i;
	}

	static int right(int i)
	{
		assert(i >= 1);
		return 2 * i + 1;
	}

	// left and right of i are heaps, but at i we need to restore heap property
	void heapifyDown(int i)
	{
		int const n = size();
		while (true)
		{
			assert(i <= n);
			int best = i;
			if (left(i) <= n && !cmp(heap[best], heap[left(i)]))
			{
				best = left(i);
			}
			if (right(i) <= n && !cmp(heap[best], heap[right(i)]))
			{
				best = right(i);
			}
			if (best == i)
			{
				break;
			}
			// swap elements i and best
			std::swap(heap[i], heap[best]);
			setPosition(heap[i], HeapPosition{i});
			setPosition(heap[best], HeapPosition{best});
			// move down to best
			i = best;
		}
	}

	void heapifyUp(int i)
	{
		T elem = std::move(heap[i]);
		while (i > 1 && !cmp(heap[parent(i)], elem))
		{
			heap[i] = std::move(heap[parent(i)]);
			setPosition(heap[i], HeapPosition{i});
			i = parent(i);
		}
		heap[i] = std::move(elem);
		setPosition(heap[i], HeapPosition{i});
	}

	T extractFrom(int i)
	{
		int const n = size();
		assert(i >= 1);
		assert(i <= n);
		setPosition(heap[i], HeapPosition{0});
		T const result = std::move(heap[i]);
		if (n > i)
		{
			heap[i] = std::move(heap[n]);
			setPosition(heap[i], HeapPosition{i});
		}
		heap.pop_back();
		// same n as above, despite pop_back
		if (n > i)
		{
			heapifyDown(i);
		}
		return result;
	}

	void buildHeap()
	{
		for (int i = size() / 2; i >= 1; --i)
		{
			heapifyDown(i);
		}
	}

	// element 0 is unused
	std::vector<T> heap;
	Compare cmp;
	SetPosition setPosition;
};

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
		std::cout << "entry with prio " << entry->prio
			<< " and idx " << idx
			<< " is at heap position " << heap_pos << "\n";
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

	std::uniform_int_distribution<> heap_size_distrib(5, 100);
	std::uniform_int_distribution<> priority_distrib(-100, 1000);
	for (int test = 0; test < 10; ++test)
	{
		int const n = heap_size_distrib(rnd);
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
				std::cout << "erasing entry idx " << i
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
			int const idx = entry - &entries[0];
			std::cout << "i=" << i << " extracted entry with prio " << entry->prio
				<< " and idx " << idx << "\n";
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
	test_permutations();
	test_heap();
}
