#include <Windows.h>
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
#include <iostream>

int main()
{
	{
		using namespace std;
		using namespace DirectX;
		using namespace DirectX::PackedVector;

		cout.setf(ios_base::boolalpha);
		cout.precision(8);

		if (!XMVerifyCPUSupport())
		{
			cout << "DirectXMath is not supported on this CPU" << endl;
			return 0;
		}

		XMVECTOR u = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
		XMVECTOR n = XMVector3Normalize(u);
		float LU = XMVectorGetX(XMVector3Length(n));

		// Mathematically the length should be 1. Is it numerically?
		cout << LU << endl;

		if (LU == 1.0f)
		{
			cout << "Length is exactly 1.0f" << endl;
		}
		else
		{
			cout << "Length is NOT exactly 1.0f" << endl;
		}

		//	Raising 1 to any power should be 1. Is it?
		float powLU = powf(LU, 1.0e6f);
		cout << powLU << endl;
	}
}