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
	// Only elements as defined by neighbors are valid.
	// weight==0 if not yet filled
	// All existing edges must have a weight, from the set {1, 2, ..., num_edges}.
	uint8_t weights[c_max_num_vertices][c_max_num_vertices];
};

int num_vertices;
int num_edges;

int secret_start_vertex;
int secret_final_vertex;

struct Vertex
{
	std::vector<int> neighbors;
	int sum_of_weights = 0; // 0 if no constraint
	std::vector<int> path_weights;
};

std::vector<Vertex> vertices;

Edges edges;

void checkVertexId(int v)
{
	if (v < 0 || v >= num_vertices)
		throw std::runtime_error("invalid vertex id");
}

void skipComments()
{
	auto oldmask = std::cin.exceptions();
	std::cin.exceptions(std::ios::goodbit);
	char c;
	while (std::cin.get(c))
	{
		if (std::isspace(c))
		{
			// ignore
		}
		else if (c == '#')
		{
			// ignore until the end of line
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		else
		{
			// put it back and return
			std::cin.putback(c);
			break;
		}
	}
	// clear failbit if there was EOF
	std::cin.clear();
	std::cin.exceptions(oldmask);
}

void readData()
{
	std::cin.exceptions(std::ios::failbit);
	skipComments();
	std::cin >> num_vertices >> num_edges;
	if (num_vertices <= 0 || num_vertices > c_max_num_vertices)
		throw std::runtime_error("invalid num_vertices");
	if (num_edges <= 0)
		throw std::runtime_error("invalid num_edges");

	vertices.resize(num_vertices);

	for (int i = 0; i < num_edges; ++i)
	{
		int v1, v2, weight;
		skipComments();
		std::cin >> v1 >> v2 >> weight;
		checkVertexId(v1);
		checkVertexId(v2);
		if (weight < 0 || weight > num_edges)
			throw std::runtime_error("invalid weight");

		edges.setWeight(v1, v2, weight);
		vertices[v1].neighbors.push_back(v2);
		vertices[v2].neighbors.push_back(v1);
	}

	for (int v = 0; v < num_vertices; ++v)
	{
		Vertex & vertex = vertices[v];
		std::sort(vertex.neighbors.begin(), vertex.neighbors.end());
		for (int i = 1; i < (int)vertex.neighbors.size(); ++i)
		{
			if (vertex.neighbors[i] == vertex.neighbors[i-1])
				throw std::runtime_error("duplicate neighbor");
		}
	}

	int num_constraints;
	skipComments();
	std::cin >> num_constraints;
	for (int i = 0; i < num_constraints; ++i)
	{
		int v, sum;
		skipComments();
		std::cin >> v >> sum;
		checkVertexId(v);
		if (sum <= 0)
			throw std::runtime_error("invalid sum of edge weights");
		vertices[v].sum_of_weights = sum;
	}

	skipComments();
	std::cin >> num_constraints;
	for (int i = 0; i < num_constraints; ++i)
	{
		int v, path_weight;
		skipComments();
		std::cin >> v >> path_weight;
		checkVertexId(v);
		if (path_weight <= 0)
			throw std::runtime_error("invalid path_weight");
		vertices[v].path_weights.push_back(path_weight);
	}

	skipComments();
	std::cin >> secret_start_vertex >> secret_final_vertex;
	checkVertexId(secret_start_vertex);
	checkVertexId(secret_final_vertex);

	skipComments();
}

} // namespace

int main()
{
	std::cout << "Hello world from bugbyte!\n";
	std::cout << "Reading data from stdin...\n";
	try
	{
		readData();
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
	std::cout << "secret_start_vertex: " << secret_start_vertex << "\n";
	std::cout << "secret_final_vertex: " << secret_final_vertex << "\n";
}
