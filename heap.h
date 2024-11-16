#ifndef _HEAP_H_
#define _HEAP_H_

#include <cassert>
#include <vector>

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
			assert(cmp(heap[i], heap[i]));
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
		assert(cmp(heap[n], heap[n]));
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

	void keyChanged(HeapPosition const pos)
	{
		heapifyDown(pos.val);
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

	// restore heap property by moving element down
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

	// restore heap property by moving element up
	void heapifyUp(int i)
	{
		if (i == 1)
		{
			// Not strictly necessary, but if we are called from extractFrom(1), this may allow compiler to optimize
			// away the rest of the function when inlining.
			return;
		}
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

	T extractFrom(int const i)
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
			heapifyUp(i);
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

#endif // _HEAP_H_
