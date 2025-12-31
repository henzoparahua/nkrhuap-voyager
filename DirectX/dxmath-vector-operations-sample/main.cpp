#include <Windows.h>
#include <DirectXPackedVector.h>
#include <DirectXMath.h>
#include <iostream>

std::ostream& XM_CALLCONV operator << (std::ostream& os, DirectX::FXMVECTOR v)
{
	DirectX::XMFLOAT3 dest;
	DirectX::XMStoreFloat3(&dest, v);

	os << " ( " << dest.x << ", " << dest.y << ", " << dest.z << " ) ";

	return os;
}

int main()
{
	{
		using namespace std;
		using namespace DirectX;
		using namespace DirectX::PackedVector;

		cout.setf(ios_base::boolalpha);

		if (!XMVerifyCPUSupport())
		{
			cout << "DirectXMath is not supported on this CPU" << endl;
			return 0;
		}


		XMVECTOR n = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
		XMVECTOR v = XMVectorSet(-2.0f, 1.0f, -3.0f, 0.0f);
		XMVECTOR w = XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

		//	Vector addition
		XMVECTOR a = u + v;

		//	Vector subtraction:
		XMVECTOR b = u - v;

		//	Scalar multiplication
		XMVECTOR c = 10.0f * u;

		//	Vector length ||u||
		XMVECTOR L = XMVector3Length(u);

		//	Vector normalization
		XMVECTOR d = XMVector3Normalize(u);

		//	Vector dot product
		XMVECTOR s = XMVector3Dot(u, v);

		//	Vector cross product
		XMVECTOR e = XMVector3Cross(u, v);

		//	Projection of proj_n(w) and perp_n(w)
		XMVECTOR projW;
		XMVECTOR perpW;
		XMVector3ComponentsFromNormal(&perpW, &projW, w, n);

		//	Confirming projW + perpW == w
		bool equal = XMVector3Equal(projW + perpW, w);
		bool notEqual = !XMVector3Equal(projW + perpW, w);

		//	The angle between projW and perpW should be 90 degrees (pi/2 radians)
		XMVECTOR angleVec = XMVector3AngleBetweenNormals(projW, perpW);
		float angleRadians = XMVectorGetX(angleVec);
		float angleDegrees = XMConvertToDegrees(angleRadians);

		cout << "u = " << u << endl;
		cout << "v = " << v << endl;
		cout << "w = " << w << endl;
		cout << "n = " << n << endl;
		cout << endl;
		cout << "a =  u + v = " << a << endl << endl;
		cout << "b =  u - v = " << b << endl << endl;
		cout << "c = 10 * v = " << c << endl << endl;
		cout << "d =  u / ||u||  = " << d << endl << endl;
		cout << "e =  u x v  = " << e << endl << endl;
		cout << "L = ||u|| = " << L << endl << endl;
		cout << "s =  u . v = " << s << endl << endl;
		cout << "projW = " << projW << endl << endl;
		cout << "perpW = " << perpW << endl << endl;
		cout << "projW + perpW == w " << equal << endl << endl;
		cout << "projW + perpW != w " << notEqual << endl << endl;
		cout << "angle = " << angleDegrees << endl << endl;
	}
}