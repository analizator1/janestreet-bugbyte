#ifndef _PERMUTATIONS_H_
#define _PERMUTATIONS_H_

#include <vector>
#include <functional>

using UintVec = std::vector<unsigned>;

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
	PermutationsWithSumGenerator(UintVec const & v, unsigned k, int target_sum,
			std::function<void(UintVec const &)> callback);

	void run();

private:
	void do_run(unsigned pos, unsigned cur_sum);

	UintVec const v;
	std::vector<bool> used;
	UintVec perm;
	unsigned const k;
	int const target_sum;
	std::function<void(UintVec const &)> callback;
};

#endif // _PERMUTATIONS_H_
