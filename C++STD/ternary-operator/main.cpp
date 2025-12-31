#include <iostream>

int main()
{
	int x{ 0 }, y{ 0 }, max{ 0 };
	std::cout << "Finish by typing -1 in x or y." << std::endl;;

	while (true)
	{
		std::cout << "Enter two integers: ";
		std::cin >> x >> y;
		if (x == -1 || y == -1)
			break;
		max = (max < x) ? (x < y ? y : x) : (max < y ? y : max);
		std::cout << "The maximum value is: " << max << std::endl;
	}

	return 0;
}