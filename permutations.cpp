#include "permutations.h"
#include "utils.h"

#include <algorithm>
#include <cassert>
#include <iostream>

PermutationsWithSumGenerator::PermutationsWithSumGenerator(UintVec const & v, unsigned k, int target_sum,
		std::function<void(UintVec const &)> callback):
	v(v),
	used(v.size()),
	perm(k),
	k(k),
	target_sum(target_sum),
	callback(callback)
{
	assert(std::is_sorted(v.begin(), v.end()));
}

void PermutationsWithSumGenerator::run()
{
	std::cout << "PermutationsWithSumGenerator::run():"
		<< " v=" << v
		<< " k=" << k
		<< " target_sum=" << target_sum
		<< "\n";
	if (k > v.size() || target_sum < 0)
	{
		return; // no solutions
	}
	if (k == 0)
	{
		if (target_sum == 0)
		{
			// exactly one solution
			callback(perm);
		}
		return; // no other solutions
	}
	unsigned max_possible_sum = 0;
	for (unsigned i = v.size() - k; i < v.size(); ++i)
	{
		max_possible_sum += v[i];
	}
	if (max_possible_sum < (unsigned)target_sum)
	{
		return; // no solutions
	}
	do_run(0, 0);
}

void PermutationsWithSumGenerator::do_run(unsigned pos, unsigned cur_sum)
{
	assert(pos < k);
	assert(cur_sum <= (unsigned)target_sum);
	if (pos == k - 1)
	{
		unsigned needed = (unsigned)target_sum - cur_sum;
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
		for (unsigned i = 0; i < v.size(); ++i)
		{
			if (cur_sum + v[i] > (unsigned)target_sum)
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
