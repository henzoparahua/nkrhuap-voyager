#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

int main()
{
	std::vector<float> nums = { -50.0f, 2.5f, -1.0f, 10.0f, -0.1f, 100.0f };

//	Numbers ordered by magnitude, not value.
	std::sort(nums.begin(), nums.end(),
		[](float a, float b) {
			return (std::abs(a) < std::abs(b));
		}
	);

	for (float n : nums)
		std::cout << n << " ";

	return 0;
}