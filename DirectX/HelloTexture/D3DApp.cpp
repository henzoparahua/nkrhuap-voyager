#include "D3DApp.h"

D3DApp::D3DApp(UINT width, UINT height, std::wstring name) :
	DXSample(width, height, name),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0.0f, 0.0f, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_rtvDescriptorSize(0)
{

}

void D3DApp::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

//	Load the rendering pipeline dependencies.
void D3DApp::LoadPipeline()
{
	UINT dxgiFactoryFlags{ 0 };

#if defined(_DEBUG)
	//	Enable the debug layer (requires the Graphics Tool "optinal feature").
	//	NOTE:	Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			//	Enabling additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	//	Factories are the entry point to the DirectX 12 API, and allow
	//	us to find adapters that we can use to execute DirectX commands.
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	//	An adapter provides information on the physical properties of a
	//	given DirectX device. You can use it to query for the current GPU's
	//	name, manufacturer, how much memory it has, and so on.

	//	There are two types of adapters: software and hardware; MS Windows always 
	//	includes a software based DirectX implementation that can be used in the 
	//	event of there being no dedicated hardware such as discrete or integrated GPU.

	//	A device is the primary entry point to the DirectX API, giving you access
	//	to the inner parts of the API. This is the key to accessing important data
	//	structures such as pipelines, shader blobs, render states, resource barriers...
	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_1,
			IID_PPV_ARGS(&m_device)
		));
	}
	else {
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	//	Describe and create the Command Queue
	//	A command queue allows us to submit groups of draw calls, 
	//	known as command lists, together to execute in order, thus 
	//	allowing a GPU to stay busy and optimize its execution time.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	//	Describe and create the Swap Chain
	//	SwapChains handle swapping and allocating back buffers 
	//	to display what you are rendering to a given window.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(), // Swap Chain needs the queue so that it can force a flush on it.
		Win32App::GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain
	));

	//	TO DO :: Fullscreen Transition
	ThrowIfFailed(factory->MakeWindowAssociation(Win32App::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	//	Create descriptor heaps.
	//	Descriptor Heaps are objects that handle memory allocation required
	//	for storing the descriptions of objects that shaders reference.
	{
		//	Describe and create a render target view (RTV) Descriptor Heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		//	Describe and create a shader resource view (SRV) heap for texture.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};

		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//	Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT n{ 0 }; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

//	Load the sample assets.
void D3DApp::LoadAssets()
{
	//	Create an empty root signature.
	//	Root Signatures are objects that define what type of resource are accessible 
	//	to shaders, be it constant buffers, structured buffers, samplers, textures, etc. 
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	//	The Root Signature 1_1 is to enable applications to indicate to drivers when descriptors in a descriptor
	//	heap won't change or the data descriptors point to won't change. This allows the option for drivers to make
	//	optimizations that might be possible knowing that a descriptor or the memory it points to is static.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		
		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
		//	Root Signature 1.0 allows the contents of descriptor heaps and the memory they point to be freely
		//	changed by applications any time that command lists / bundles referencing them are potentially in
		//	flight on the GPU. However, in applications such as this one, don't actually need the flexibility 
		//	to change descriptors or memory after commands that reference them have been recorded.
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		
		CD3DX12_ROOT_PARAMETER1 rootParams[1];
		rootParams[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParams), rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	//	Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		//	Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags{ D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION };
#else
		UINT compileFlags{ 0 };
#endif

		//	Vertex Shaders execute per vertex, and are perfect for 
		//	transforming a given object, performing per vertex 
		//	animations with blend shapes, GPU skinning, etc.
		ThrowIfFailed(D3DCompileFromFile(
			GetAssetFullPath(L"shaders.hlsl").c_str(),
			nullptr, nullptr, "VSMain", "vs_5_0",
			compileFlags, 0, &vertexShader, nullptr));

		//	Pixel Shaders execute per each pixel of your output, including 
		//	the other attachments that correspond to that pixel coordinate.
		ThrowIfFailed(D3DCompileFromFile(
			GetAssetFullPath(L"shaders.hlsl").c_str(),
			nullptr, nullptr, "PSMain", "ps_5_0",
			compileFlags, 0, &pixelShader, nullptr));

		//	Define the vertex input layout
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		//	Describe and create the graphics pipeline state object (PSO)
		//	The Pipeline State describes everything necessary to execute a given raster based draw call.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	ThrowIfFailed(m_device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		m_pipelineState.Get(),
		IID_PPV_ARGS(&m_commandList)));

	//	Create the Vertex Buffer
	//	The Vertex Buffer stores the per vertex information available as attributes
	//	in the Vertex Shader. All buffers are ID3D12Resource objects in DirectX 12,
	//	be it Vertex Buffers, Index Buffers, Constant Buffers, etc.
	{
		//	Define the geometry of the object rendered
 		Vertex triangleVertices[] =
		{
			{ {   0.0f,  0.3f * m_aspectRatio, 0.0f }, {  0.5f, 0.0f } },
			{ {  0.5f, -0.25f * m_aspectRatio, 0.0f }, { 1.0f, 1.0f } },
			{ { -0.5f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f } }
		};

		const UINT vertexBufferSize{ sizeof(triangleVertices) };

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

		//	Using upload heaps to transfer static data like vert buffers is not recommended.
		//	Every time the GPU needs it, the upload heap will be marshalled over. TO DO.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&heapProperties, D3D12_HEAP_FLAG_NONE,
			&bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&m_vertexBuffer)));

		//	Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);	//	We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange,
			reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer->Unmap(0, nullptr);

		//	Initialize the Vertex Buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}

	ComPtr<ID3D12Resource> textureUploadHeap;

//	Create the texture
	{
		//	Describe and create a Texture2D
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.Width = textureWidth;
		textureDesc.Height = textureHeight;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		auto defaultTexHeapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };

		ThrowIfFailed(m_device->CreateCommittedResource(
			&defaultTexHeapProps, D3D12_HEAP_FLAG_NONE,
			&textureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, IID_PPV_ARGS(&m_texture)));

		const UINT64 uploadBufferSize{ GetRequiredIntermediateSize(m_texture.Get(), 0, 1) };

		auto uploadBufferHeapProps{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
		auto resourceBufferDescProps{ CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize) };

	//	Create the GPU upload buffer
		ThrowIfFailed(m_device->CreateCommittedResource(
			&uploadBufferHeapProps, D3D12_HEAP_FLAG_NONE, 
			&resourceBufferDescProps, D3D12_RESOURCE_STATE_GENERIC_READ, 
			nullptr, IID_PPV_ARGS(&textureUploadHeap)));

		//	Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the Texture2D.
		std::vector<UINT8> texture{ GenerateTextureData() };

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = &texture[0];
		textureData.RowPitch = textureWidth * texturePixelSize;
		textureData.SlicePitch = textureData.RowPitch * textureHeight;

		UpdateSubresources(m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);

		auto barrierTransitionProps{ CD3DX12_RESOURCE_BARRIER::Transition(
			m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) };

		m_commandList->ResourceBarrier(1, &barrierTransitionProps);

		//	Describe and create a SRV for the texture
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	//	Command Lists are created in the recording state. The main loop expects it to be closed, and so we do.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);


	//	Create synchronization objects and wait until assets have been uploaded to the GPU.
	//	A Fence lets your program know when certain tasks have been executed by the GPU, be
	//	it uploads to GPU exclusive memory, or when finished preseting to the screen.
	{
		ThrowIfFailed(m_device->CreateFence(0,
			D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		//	Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		if (m_fenceEvent == nullptr)
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

		//	Wait for the command list to execute; we just want to wait
		//	for the setup to complete before continuing.
		WaitForPreviousFrame();
	}
}

std::vector<UINT8> D3DApp::GenerateTextureData()
{
	const UINT rowPitch{ textureWidth * texturePixelSize };
	const UINT cellPitch{ rowPitch >> 3 };
	const UINT cellHeight{ textureWidth >> 3 };
	const UINT textureSize{ rowPitch * textureHeight };

	std::vector<UINT8> data(textureSize);
	UINT8* pData = &data[0];

	for (UINT n{ 0 }; n < textureSize; n += texturePixelSize)
	{
		UINT x{ n % rowPitch };
		UINT y{ n / rowPitch };
		UINT i{ x / cellPitch };
		UINT j{ y / cellHeight };

		if (i % 2 == j % 2)
		{
			pData[n] = 0x00;    	// R
			pData[n + 1] = 0x00;	// G
			pData[n + 2] = 0x00;	// B
			pData[n + 3] = 0xff;	// A
		}
		else {
			pData[n] = 0xff;		// R
			pData[n + 1] = 0xff;	// G
			pData[n + 2] = 0xff;	// B
			pData[n + 3] = 0xff;	// A
		}
	}

	return data;
}

void D3DApp::OnUpdate() {}

void D3DApp::OnRender()
{
	PopulateCommandList();

	ID3D12CommandList* ppCommandList[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3DApp::OnDestroy()
{
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void D3DApp::PopulateCommandList()
{
	//	Command List Allocators can only be reset when the associated command lists
	//	have finished execution on the GPU; apps should use fences to determine GPU
	//	execution progress
	ThrowIfFailed(m_commandAllocator->Reset());

	//	However, when ExecuteCommandList() is called on a particular command list,
	//	that command list can then be reset at any time and must be before re-recording
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	//	Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	
	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	auto resourceBarrierRenderTarget{ CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET) };

	//	A barrier lets the driver know how a resource should be used in upcoming commands.
	//	This can be useful if say, you're writing to a texture, and you want to copy that
	//	texture to another texture (such as the swapchain's render attachment).
	m_commandList->ResourceBarrier(1, &resourceBarrierRenderTarget);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize);

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	//	Record Commands
	const float	clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	auto resourceBarrierProps{ CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT) };

	//	Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &resourceBarrierProps);

	ThrowIfFailed(m_commandList->Close());
}

void D3DApp::WaitForPreviousFrame()
{
	const UINT64 fence{ m_fenceValue };
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}