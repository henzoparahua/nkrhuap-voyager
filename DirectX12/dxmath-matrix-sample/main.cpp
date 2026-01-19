#include <DirectXMath.h>
#include <iostream>

std::ostream& XM_CALLCONV operator << (std::ostream& os, DirectX::FXMVECTOR v)
{
	DirectX::XMFLOAT4 dest;
	DirectX::XMStoreFloat4(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ")";

	return os;
}

std::ostream& XM_CALLCONV operator << (std::ostream& os, DirectX::FXMMATRIX m)
{
	for (int i = 0; i < 4; ++i)
	{
		os << DirectX::XMVectorGetX(m.r[i]) << "\t";
		os << DirectX::XMVectorGetY(m.r[i]) << "\t";
		os << DirectX::XMVectorGetZ(m.r[i]) << "\t";
		os << DirectX::XMVectorGetW(m.r[i]);
		os << std::endl;
	}

	return os;
}

int main()
{
	if (!DirectX::XMVerifyCPUSupport())
	{
		std::cout << "DirectX Math not supported" << std::endl;
		return 0;
	}

	DirectX::XMMATRIX A(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 4.0f, 0.0f,
		1.0f, 2.0f, 3.0f, 1.0f
	);

	DirectX::XMMATRIX B = DirectX::XMMatrixIdentity();

	DirectX::XMMATRIX AB = A * B;

	DirectX::XMMATRIX TRANSPOSE_A = DirectX::XMMatrixTranspose(A);

	DirectX::XMVECTOR det_A = DirectX::XMMatrixDeterminant(A);
	DirectX::XMMATRIX INVERSE_A = DirectX::XMMatrixInverse(&det_A, A);

	DirectX::XMMATRIX F = A * INVERSE_A;

	std::cout << "A = " << std::endl << A << std::endl;
	std::cout << "B = " << std::endl << B << std::endl;
	std::cout << "AB = " << std::endl << AB << std::endl;
	std::cout << "D = transpose(A) = " << std::endl << TRANSPOSE_A << std::endl;
	std::cout << "det = determinant(A)" << std::endl << det_A << std::endl;
	std::cout << "E = inverse(A)" << std::endl << INVERSE_A << std::endl;
	std::cout << "F = A*INVERSE_A = " << std::endl << F << std::endl;

	return 0;
}