#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "utils.h"
#include "permutations.h"

namespace {

constexpr int c_max_num_vertices = 18;

class Edges
{
public:
	int getWeight(int v1, int v2) const
	{
		return v1 < v2 ? weights[v1][v2] : weights[v2][v1];
	}

	void setWeight(int v1, int v2, int weight)
	{
		(v1 < v2 ? weights[v1][v2] : weights[v2][v1]) = weight;
	}

private:
	// Only elements as defined by Vertex::neighbors are valid.
	// weight==0 if not yet filled
	// All existing edges must have a weight, from the set {1, 2, ..., num_edges}.
	uint8_t weights[c_max_num_vertices][c_max_num_vertices];
};

int num_vertices;
int num_edges;

// index 0 is unused
std::vector<bool> available_weights;
int num_available_weights; // number of true elements in available_weights

int secret_start_vertex;
int secret_final_vertex;

struct Vertex
{
	std::vector<int> neighbors;
	int sum_of_weights = 0; // sum of weights of adjacent edges; 0 if no constraint
};

std::vector<Vertex> vertices;

// constraints on path weight starting from a vertex
// vector of pair: { vertex id, path weight }
std::vector<std::pair<int, int>> vertex_path_weight_constraints;

Edges edges;

void check_vertex_id(int v)
{
	if (v < 0 || v >= num_vertices)
		throw std::runtime_error("invalid vertex id");
}

std::vector<unsigned> make_available_weights_vec()
{
	std::vector<unsigned> weights;
	weights.reserve(num_edges);
	for (int i = 1; i <= num_edges; ++i)
	{
		if (available_weights[i])
		{
			weights.push_back(i);
		}
	}
	assert((int)weights.size() == num_available_weights);
	return weights;
}

void print_graph_weights()
{
	for (int v = 0; v < num_vertices; ++v)
	{
		Vertex & vertex = vertices[v];
		for (int neigh_v : vertex.neighbors)
		{
			if (v < neigh_v)
			{
				std::cout << "(" << v << ", " << neigh_v << ") => " << edges.getWeight(v, neigh_v) << "\n";
			}
		}
	}
}

void read_data()
{
	std::cin.exceptions(std::ios::failbit);
	skipComments(std::cin);
	std::cin >> num_vertices >> num_edges;
	if (num_vertices <= 0 || num_vertices > c_max_num_vertices)
		throw std::runtime_error("invalid num_vertices");
	if (num_edges <= 0)
		throw std::runtime_error("invalid num_edges");

	vertices.resize(num_vertices);

	available_weights.resize(num_edges + 1, true);
	available_weights[0] = false;
	num_available_weights = num_edges;

	for (int i = 0; i < num_edges; ++i)
	{
		int v1, v2, weight;
		skipComments(std::cin);
		std::cin >> v1 >> v2 >> weight;
		check_vertex_id(v1);
		check_vertex_id(v2);
		if (weight < 0 || weight > num_edges)
			throw std::runtime_error("invalid weight");

		if (weight > 0)
		{
			if (!available_weights[weight])
				throw std::runtime_error("weight was already used");
			available_weights[weight] = false;
			--num_available_weights;
			edges.setWeight(v1, v2, weight);
		}

		vertices[v1].neighbors.push_back(v2);
		vertices[v2].neighbors.push_back(v1);
	}

	for (int v = 0; v < num_vertices; ++v)
	{
		Vertex & vertex = vertices[v];
		// Check duplicate edges.
		std::sort(vertex.neighbors.begin(), vertex.neighbors.end());
		for (int i = 1; i < (int)vertex.neighbors.size(); ++i)
		{
			if (vertex.neighbors[i] == vertex.neighbors[i-1])
				throw std::runtime_error("duplicate edge");
		}
	}

	int num_constraints;
	skipComments(std::cin);
	std::cin >> num_constraints;
	for (int i = 0; i < num_constraints; ++i)
	{
		int v, sum;
		skipComments(std::cin);
		std::cin >> v >> sum;
		check_vertex_id(v);
		if (sum <= 0)
			throw std::runtime_error("invalid sum of edge weights");
		vertices[v].sum_of_weights = sum;
	}

	skipComments(std::cin);
	std::cin >> num_constraints;
	for (int i = 0; i < num_constraints; ++i)
	{
		int v, path_weight;
		skipComments(std::cin);
		std::cin >> v >> path_weight;
		check_vertex_id(v);
		if (path_weight <= 0)
			throw std::runtime_error("invalid path_weight");
		vertex_path_weight_constraints.emplace_back(v, path_weight);
	}

	skipComments(std::cin);
	std::cin >> secret_start_vertex >> secret_final_vertex;
	check_vertex_id(secret_start_vertex);
	check_vertex_id(secret_final_vertex);

	skipComments(std::cin);
}

void all_constraints_satisfied()
{
	std::cout << "===== found solution =====\n";
	print_graph_weights();
}

// Finds a non-self-intersecting path.
class FindPathOfGivenWeight
{
public:
	FindPathOfGivenWeight(int desired_path_weight):
		on_current_path(num_vertices),
		desired_path_weight(desired_path_weight)
	{
	}

	bool run(int start_vertex)
	{
		return rec_find(start_vertex, 0);
	}

private:
	bool rec_find(int v, int current_path_weight)
	{
		if (current_path_weight >= desired_path_weight)
		{
			return current_path_weight == desired_path_weight;
		}

		assert(!on_current_path[v]);
		on_current_path[v] = true;

		for (int neigh_v : vertices[v].neighbors)
		{
			if (!on_current_path[neigh_v] &&
					rec_find(neigh_v, current_path_weight + edges.getWeight(v, neigh_v)))
				return true;
		}

		on_current_path[v] = false;
		return false;
	}

	std::vector<bool> on_current_path;
	int desired_path_weight;
};

void all_edge_weights_filled()
{
	for (auto const & [v, path_weight] : vertex_path_weight_constraints)
	{
		FindPathOfGivenWeight finder(path_weight);
		if (!finder.run(v))
			return;
	}

	all_constraints_satisfied();
}

void sum_of_weights_constraints_satisfied()
{
	// All sum_of_weights constraints are satisfied. We must fill in remaining edges which are not adjacent to any
	// vertex with this constraint.
	if (num_available_weights > 0)
	{
		// Find all unfilled edges, then for each permutation of available_weights, fill the edges with the permutation.
		throw std::runtime_error("unimplemented: num_available_weights>0");
	}
	else
	{
		all_edge_weights_filled();
	}
}

std::vector<int> vertices_for_sum_of_weights;

void rec_solve(int vertices_for_sum_of_weights_idx)
{
	//std::cout << "rec_solve(" << vertices_for_sum_of_weights_idx << ")\n";
	if (vertices_for_sum_of_weights_idx == (int)vertices_for_sum_of_weights.size())
	{
		sum_of_weights_constraints_satisfied();
	}
	else
	{
		int const v = vertices_for_sum_of_weights[vertices_for_sum_of_weights_idx];
		Vertex & vertex = vertices[v];
		// We must try to satisfy the sum_of_weights constraint. It may happen that all adjacent edges are already
		// filled. In this case we try to generate a zero-length permutation, which only succeeds if the sum is exactly
		// as expected. Therefore it serves as a check for the constraint, so we must not skip it.
		int current_weight_sum = 0;
		std::vector<int> neighbors_with_unfilled_edge;
		for (int neigh_v : vertex.neighbors)
		{
			int const weight = edges.getWeight(v, neigh_v);
			current_weight_sum += weight;
			if (weight == 0)
			{
				neighbors_with_unfilled_edge.push_back(neigh_v);
			}
		}
		int const remaining_sum = vertex.sum_of_weights - current_weight_sum;
		PermutationsWithSumGenerator generator(make_available_weights_vec(), neighbors_with_unfilled_edge.size(), remaining_sum,
			[&](UintVec const & weights_to_fill) {
				assert(weights_to_fill.size() == neighbors_with_unfilled_edge.size());
				for (int i = 0; i < (int)weights_to_fill.size(); ++i)
				{
					int const neigh_v = neighbors_with_unfilled_edge[i];
					int const weight = weights_to_fill[i];
					assert(edges.getWeight(v, neigh_v) == 0);
					edges.setWeight(v, neigh_v, weight);
					assert(available_weights[weight]);
					available_weights[weight] = false;
				}
				num_available_weights -= (int)weights_to_fill.size();
				rec_solve(vertices_for_sum_of_weights_idx + 1);
				num_available_weights += (int)weights_to_fill.size();
				for (int i = 0; i < (int)weights_to_fill.size(); ++i)
				{
					int const neigh_v = neighbors_with_unfilled_edge[i];
					int const weight = weights_to_fill[i];
					assert(edges.getWeight(v, neigh_v) == weight);
					edges.setWeight(v, neigh_v, 0);
					assert(!available_weights[weight]);
					available_weights[weight] = true;
				}
		});
		generator.run();
	}
}

void solve()
{
	for (int v = 0; v < num_vertices; ++v)
	{
		Vertex & vertex = vertices[v];
		if (vertex.sum_of_weights)
		{
			vertices_for_sum_of_weights.push_back(v);
		}
	}
	std::sort(vertices_for_sum_of_weights.begin(), vertices_for_sum_of_weights.end(),
		[](int v1, int v2) { return vertices[v1].sum_of_weights < vertices[v2].sum_of_weights; }
	);

	// Finding solution is faster if we check paths starting from the shortest.
	std::sort(vertex_path_weight_constraints.begin(), vertex_path_weight_constraints.end(),
		[](std::pair<int, int> const & p1, std::pair<int, int> const & p2) {
			return p1.second < p2.second;
	});

	rec_solve(0);
}

} // namespace

int main()
{
	std::cout << "Hello world from bugbyte!\n";
	std::cout << "Reading data from stdin...\n";
	try
	{
		read_data();
	}
	catch (std::ios::failure & err)
	{
		std::cerr << "error parsing data: " << err.what() << "\n";
		return -1;
	}
	catch (std::runtime_error & exc)
	{
		std::cerr << "error in data: " << exc.what() << "\n";
		return -1;
	}
	std::cout << "Read all data.\n";
	std::cout << "num_vertices: " << num_vertices << "\n";
	std::cout << "num_edges: " << num_edges << "\n";
	std::cout << "num_available_weights: " << num_available_weights << "\n";
	std::cout << "secret_start_vertex: " << secret_start_vertex << "\n";
	std::cout << "secret_final_vertex: " << secret_final_vertex << "\n";

	solve();
}
