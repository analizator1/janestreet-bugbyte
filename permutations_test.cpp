#include "permutations.h"
#include "utils.h"

#include <iostream>

void test_permutations_case(UintVec const & v, unsigned k, int target_sum)
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
	for (unsigned i = 0; i < 10; ++i)
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
	test_permutations_case(v, 2, -1);
	test_permutations_case(v, 2, 0);
	test_permutations_case(v, 2, 1);
	test_permutations_case(v, 2, 2);
	test_permutations_case(v, 2, 3);
	test_permutations_case(v, 0, -1);
	test_permutations_case(v, 0, 0);
	test_permutations_case(v, 0, 1);

	std::cout << "END " << __func__ << "\n";
}

int main()
{
	test_permutations();
}
