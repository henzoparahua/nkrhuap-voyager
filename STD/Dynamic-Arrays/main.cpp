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

void arrerase(const std::vector<Vertex>& vertices)
{
	vertices.erase(1 + vertices.begin());
	for (Vertex& v : vertices)
		std::cout << v << std::endl;
}

int main()
{
	std::vector<Vertex> vertices;
	
	vertices.push_back({ 1, 2, 3 });
	vertices.push_back({ 4, 5, 6 });
	for (const Vertex& v: vertices)
		std::cout << v << std::endl;

	std::cout << "- - - -" << std::endl;

	arrerase(vertices);
	
	return 0;
}