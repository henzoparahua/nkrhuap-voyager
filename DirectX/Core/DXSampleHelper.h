#pragma once
#include <stdexcept>
#include "stdafx.h"

using Microsoft::WRL::ComPtr;

inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
	const HRESULT m_hr;

public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
};

#define SAFE_RELEASE(p) if (p) (p)->Release()

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw HrException(hr);
};

inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
{
	if (path == nullptr)
		throw std::exception();

	DWORD size{ GetModuleFileName(nullptr, path, pathSize) };
	if (size == 0 || size == pathSize)
		throw std::exception();

	WCHAR* lastSlash = wcsrchr(path, L'\\');
	if (lastSlash)
		*(lastSlash + 1) = L'\0';
};

inline HRESULT ReadDataFromFile(LPCWSTR filename, byte** data, UINT* size)
{
	using namespace Microsoft::WRL;

#if WINVER >= _WIN32_WINNT_WIN8
	CREATEFILE2_EXTENDED_PARAMETERS extendedParams{};
	extendedParams.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	extendedParams.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	extendedParams.dwFileFlags = FILE_FLAG_SEQUENTIAL_SCAN;
	extendedParams.dwSecurityQosFlags = SECURITY_ANONYMOUS;
	extendedParams.lpSecurityAttributes = nullptr;
	extendedParams.hTemplateFile = nullptr;

	Wrappers::FileHandle file(CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams));
#else
	Wrappers::FileHandle file(CreateFile(filename, GENERIC_READ, FILE_SHARED_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, nullptr));
#endif

	if (file.Get() == INVALID_HANDLE_VALUE)
		throw std::exception();

	FILE_STANDARD_INFO fileInfo{};
	
	if (!GetFileInformationByHandleEx(file.Get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
		throw std::exception();

	if (fileInfo.EndOfFile.HighPart != 0)
		throw std::exception();
	
	*data = reinterpret_cast<byte*>(malloc(fileInfo.EndOfFile.LowPart));
	*size = fileInfo.EndOfFile.LowPart;

	if (!ReadFile(file.Get(), *data, fileInfo.EndOfFile.LowPart, nullptr, nullptr))
		throw std::exception();

	return S_OK;
}

inline HRESULT ReadDataFromDDSFile(LPCWSTR filename, byte** data, UINT* offset, UINT* size)
{
	if (FAILED(ReadDataFromFile(filename, data, size)))
		return E_FAIL;

	static const UINT DDS_MAGIC = 0x20534444;
	UINT magicNum = *reinterpret_cast<const UINT*>(*data);
	if (magicNum != DDS_MAGIC)
		return E_FAIL;

	struct DDS_PIXELFORMAT
	{
		UINT size;
		UINT flag;
		UINT fourCC;
		UINT rgbBitCount;
		UINT rBitMask;
		UINT gBitMask;
		UINT bBitMask;
		UINT aBitMask;
	};

	struct DDS_HEADER
	{
		UINT size;
		UINT flags;
		UINT height;
		UINT width;
		UINT pitchOrLinearSize;
		UINT depth;
		UINT mipMapLevel;
		UINT reserved1[11];
		DDS_PIXELFORMAT ddsPixelFormat;
		UINT caps;
		UINT caps2;
		UINT caps3;
		UINT caps4;
		UINT reserved2;
	};

	auto ddsHeader{ reinterpret_cast<const DDS_HEADER*>(*data + sizeof(UINT)) };
	if (ddsHeader->size != sizeof(DDS_HEADER) || ddsHeader->ddsPixelFormat.size != sizeof(DDS_PIXELFORMAT))
		return E_FAIL;

	const ptrdiff_t ddsDataOffset{ sizeof(UINT) + sizeof(DDS_HEADER) };
	
	*offset = ddsDataOffset;
	*size = *size - ddsDataOffset;

	return S_OK;
}

#if defined(_DEBUG) || defined(DBG)
inline void SetName(ID3D12Object* object, LPCWSTR name)
{
	object->SetName(name);
}

inline void SetNameIndexed(ID3D12Object* object, LPCWSTR name, UINT index)
{
	WCHAR fullname[50];
	if (swprintf_s(fullname, L"%s[%u]", name, index) > 0)
		object->SetName(fullname);
}

#else
inline void SetName(ID3D12Object* object, LPCWSTR name) { }
inline void SetNameIndexed(ID3D12Object* object, LPCWSTR name, UINT index) { }
#endif

#define NAME_D3D12_OBJECT(x) SetName((x).Get(), L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

inline UINT CalculateConstantBufferByteSize(UINT bytesize)
{
	return (bytesize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
}

#ifdef D3D_COMPILE_STANDARD_FILE_INCLUDE
inline Microsoft::WRL::ComPtr<ID3DBlob> CompilerShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileflags{ 0 };

#if defined(_DEBUG) || defined(DBG)
	compileflags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> bytecode{ nullptr };
	Microsoft::WRL::ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileflags, 0, &bytecode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());
	ThrowIfFailed(hr);

	return bytecode;
}
#endif

template<class T>
void ResetComPtrArray(T* comPtrArray)
{
	for (auto& i : *comPtrArray)
	{
		i.Reset();
	}
}

template<class T>
void ResetUniquePtrArray(T* uniquePtrArray)
{
	for (auto& i : *uniquePtrArray)
	{
		i.reset();
	}
}