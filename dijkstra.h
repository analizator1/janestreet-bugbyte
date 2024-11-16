#ifndef _DIJKSTRA_H_
#define _DIJKSTRA_H_

#include <cassert>
#include <vector>
#include <limits>

#include "heap.h"

template<class WeightT, class GetNeighbors, class GetWeight>
class Dijkstra
{
public:
	Dijkstra(std::vector<WeightT> & dist, std::vector<int> & pred, int n,
			GetNeighbors getNeighbors = GetNeighbors(),
			GetWeight getWeight = GetWeight()):
		dist(dist),
		pred(pred),
		n(n),
		getNeighbors(getNeighbors),
		getWeight(getWeight)
	{
		assert(n >= 0);
	}

	void run(int start)
	{
		assert(start >= 0);
		assert(start < n);

		for (int i = 0; i < n; ++i)
		{
			dist[i] = std::numeric_limits<WeightT>::max();
			pred[i] = -1;
		}
		dist[start] = 0;

		std::vector<HeapPosition> positions;
		Heap<int, HeapCompare, HeapSetPosition> q{HeapCompare{dist}, HeapSetPosition{positions}};
		for (int i = 0; i < n; ++i)
		{
			q.uninitializedAdd(i);
		}
		q.initialize();

		WeightT last_dist = 0;
		while (!q.empty())
		{
			int const v = q.extract();
			assert(v >= 0);
			assert(v < n);
			assert(dist[v] >= last_dist);
			last_dist = dist[v];
			for (int const neigh_v : getNeighbors(v))
			{
				assert(neigh_v >= 0);
				assert(neigh_v < n);
				if (dist[v] != std::numeric_limits<WeightT>::max() &&
						dist[v] + getWeight(v, neigh_v) < dist[neigh_v])
				{
					dist[neigh_v] = dist[v] + getWeight(v, neigh_v);
					pred[neigh_v] = v;
					q.keyChangedTowardsTop(positions[neigh_v]);
				}
			}
		}
	}

private:
	struct HeapCompare
	{
		std::vector<WeightT> & dist;

		void operator()(int v1, int v2)
		{
			return dist[v1] <= dist[v2];
		}
	};

	struct HeapSetPosition
	{
		std::vector<HeapPosition> & positions;

		void operator()(int v, HeapPosition pos)
		{
			positions[v] = pos;
		}
	};

	std::vector<WeightT> & dist;
	std::vector<int> & pred;
	int const n;
	GetNeighbors getNeighbors;
	GetWeight getWeight;
};

#endif // _DIJKSTRA_H_
