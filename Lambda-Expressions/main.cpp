#include <algorithm>
#include <cmath>

void abssort(float* x, unsigned n)
{
	std::sort(x, x + n,
		// BEGINS Lambda expression
		[](float a, float b) {
			return (std::abs(a) < std::abs(b));
		}
		// ENDS Lambda expression
	);
}