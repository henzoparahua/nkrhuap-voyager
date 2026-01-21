#include <iostream>
#include <string>
#include <vector>

struct Vertex
{
	float x, y, z;
};

std::ostream& operator<<(std::ostream& stream, const Vertex& vertex)
{
	stream << vertex.x << ", " << vertex.y << ", " << vertex.z;
	return stream;
}

int main()
{
	std::vector<Vertex> vertices;
	vertices.reserve(3);

	vertices.emplace_back( 1, 2, 3 );
	vertices.emplace_back( 4, 5, 6 );
	vertices.emplace_back( 7, 8, 9 );

	for (const Vertex& v : vertices)
		std::cout << v << std::endl;


	return 0;
}