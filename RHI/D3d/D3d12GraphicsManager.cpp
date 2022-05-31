#include <objbase.h>
#include <d3dcompiler.h>
#include <iostream>
#include "D3d12GraphicsManager.hpp"
#include "WindowsApplication.hpp"
#include "portable.hpp"
#include <windows.h>
#include <windowsx.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <DXGI1_4.h>
#include <wrl/client.h>
#include <exception>



using namespace My;
using namespace std;
using namespace Microsoft::WRL;

wstring g_AssetsPath;

std::wstring GetAssetFullPath(LPCWSTR assetName) {
    return g_AssetsPath + assetName;
}

void GetAssetsPath(WCHAR* path, UINT pathSize) {
    if (path == nullptr) {
        throw std::exception();
    }

    DWORD size = GetModuleFileNameW(nullptr, path, pathSize);
    if (size == 0 || size == pathSize) {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash) {
        *(lastSlash + 1) = L'\0';
    }
}

namespace My {

    class com_exception : public std::exception {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override {
            static char s_str[64] = {0};
            sprintf_s(s_str, "Failure with HRESULT of %08X",
                    static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

// Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            throw com_exception(hr);
        }
    }

    extern IApplication* g_pApp;

	template<class T>
	inline void SafeRelease(T **ppInterfaceToRelease)
	{
		if (*ppInterfaceToRelease != nullptr)
		{
			(*ppInterfaceToRelease)->Release();

			(*ppInterfaceToRelease) = nullptr;
		}
	}

    static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
    {
        IDXGIAdapter1* pAdapter = nullptr;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter); adapterIndex++)
        {
           DXGI_ADAPTER_DESC1 desc;
           pAdapter->GetDesc1(&desc);

           if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
           {
               // Don't select the Basic Render Driver adapter.
               continue;
           }

           // Check to see if the adapter supports Direct3D 12, but don't create the
           // actual device yet.
           if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
           {
               break;
           }
        }

        *ppAdapter = pAdapter;
    }
}

HRESULT My::D3d12GraphicsManager::CreateDescriptorHeaps() 
{
    HRESULT hr;

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = kFrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if(FAILED(hr = m_pDev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap)))) {
        return hr;
    }

    m_nRtvDescriptorSize = m_pDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if(FAILED(hr = m_pDev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)))) {
        return hr;
    }

    // Describe and create a Shader Resource View (SRV) and 
    // Constant Buffer View (CBV) and 
    // Unordered Access View (UAV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
    cbvSrvUavHeapDesc.NumDescriptors =
        kFrameCount                                         // FrameCount Cbvs.
        + 100;                                              // + 100 for the SRV(Texture).
    cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if(FAILED(hr = m_pDev->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&m_pCbvSrvUavHeap)))) {
        return hr;
    }

    m_nCbvSrvDescriptorSize = m_pDev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 2048; // this is the max D3d12 HW support currently
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if(FAILED(hr = m_pDev->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_pSamplerHeap)))) {
        return hr;
    }

  

    return hr;
}

HRESULT My::D3d12GraphicsManager::CreateRenderTarget() 
{
    HRESULT hr;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Create a RTV for each frame.
    for (uint32_t i = 0; i < kFrameCount; i++)
    {
        if (FAILED(hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pRenderTargets[i])))) {
            break;
        }
        m_pDev->CreateRenderTargetView(m_pRenderTargets[i], nullptr, rtvHandle);
        rtvHandle.ptr += m_nRtvDescriptorSize;
    }

    return hr;
}

HRESULT My::D3d12GraphicsManager::CreateDepthStencil()
{
    HRESULT hr;

    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if(FAILED(hr = m_pDev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_pDsvHeap)))) {
        return hr;
    }

	// Create the depth stencil view.
    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    uint32_t width = g_pApp->GetConfiguration().screenWidth;
    uint32_t height = g_pApp->GetConfiguration().screenHeight;
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 0;
    resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    if (FAILED(hr = m_pDev->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_pDepthStencilBuffer)
        ))) {
        return hr;
    }

    m_pDev->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilDesc, m_pDsvHeap->GetCPUDescriptorHandleForHeapStart());

    return hr;
}

HRESULT My::D3d12GraphicsManager::CreateCommandList()
{
    HRESULT hr = S_OK;
    for (uint32_t i = 0; i < kFrameCount; i++)
    {
        if (FAILED(
            hr = m_pDev->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_pGraphicsCommandAllocator[i]))
            )
        )
        {
            assert(0);
            return hr;
        }
        m_pGraphicsCommandAllocator[i]->SetName((wstring(L"Graphics Command Allocator") + to_wstring(i)).c_str());
        
        hr = m_pDev->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pGraphicsCommandAllocator[i], NULL, IID_PPV_ARGS(&m_pGraphicsCommandList[i])
        );

        if (SUCCEEDED(hr)) {
            m_pGraphicsCommandList[i]->SetName(
                (wstring(L"Graphics Command List") + to_wstring(i)).c_str());
        }

    }
    return hr;
}

HRESULT My::D3d12GraphicsManager::CreateVertexBuffer(const Buffer& buffer)
{
    HRESULT hr;

    // create vertex GPU heap 
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = buffer.m_szSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = m_pDev->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pVertexBuffer));

    return hr;
}


HRESULT My::D3d12GraphicsManager::CreateIndexBuffer(const Buffer& buffer)
{
    HRESULT hr;

    // create index GPU heap
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = buffer.m_szSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = m_pDev->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pIndexBuffer));

    return hr;
}

HRESULT My::D3d12GraphicsManager::CreateTextureBuffer(const Image& image)
{
    HRESULT hr;

    // Describe and create a Texture2D.
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = image.Width;
    textureDesc.Height = image.Height;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    hr = m_pDev->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_pTextureBuffer));

    return hr;
}

HRESULT My::D3d12GraphicsManager::CreateSamplerBuffer()
{
    // Describe and create a sampler.
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    m_pDev->CreateSampler(&samplerDesc, m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart());

    return S_OK;
}

HRESULT My::D3d12GraphicsManager::CreateConstantBuffer(const Buffer& buffer)
{

    return S_OK;
}

HRESULT My::D3d12GraphicsManager::CreateConstantBuffer()
{
    HRESULT hr;
    D3D12_HEAP_PROPERTIES prop = {D3D12_HEAP_TYPE_UPLOAD,
                                  D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                                  D3D12_MEMORY_POOL_UNKNOWN, 1, 1};
    
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = ::DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_RANGE readRange = {0, 0};

    static const size_t kSizePerFrameConstantBuffer = ALIGN( sizeof(PerFrameConstants), 256); 
    for (int32_t i = 0; i < kFrameCount; i++)
    {
        resourceDesc.Width = kSizePerFrameConstantBuffer;
        if (FAILED(hr = m_pDev->CreateCommittedResource(
                       &prop, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                       D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                       IID_PPV_ARGS(&m_pPerFrameConstantUploadBuffer[i])))) {
            return hr;
        }
        hr = m_pPerFrameConstantUploadBuffer[i]->Map(
            0, &readRange,
            reinterpret_cast<void**>(&m_pPerFrameCbvDataBegin[i]));
        m_pPerFrameConstantUploadBuffer[i]->SetName(
            L"Per Frame Constant Buffer");
    }
    

    return hr;
}



HRESULT My::D3d12GraphicsManager::CreateGraphicsResources()
{
    HRESULT hr;

#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug* pDebugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
		{
			pDebugController->EnableDebugLayer();
		}
		SafeRelease(&pDebugController);
	}
#endif

	IDXGIFactory4* pFactory;
	if (FAILED(hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)))) {
		return hr;
	}

	IDXGIAdapter1* pHardwareAdapter;
	GetHardwareAdapter(pFactory, &pHardwareAdapter);

	if (FAILED(D3D12CreateDevice(pHardwareAdapter,
		D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDev)))) {

		IDXGIAdapter* pWarpAdapter;
		if (FAILED(hr = pFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)))) {
	        SafeRelease(&pFactory);
            return hr;
        }

        if(FAILED(hr = D3D12CreateDevice(pWarpAdapter, D3D_FEATURE_LEVEL_12_0,
            IID_PPV_ARGS(&m_pDev)))) {
            SafeRelease(&pFactory);
            return hr;
        }
	}


    HWND hWnd = reinterpret_cast<WindowsApplication*>(g_pApp)->GetMainWindow();

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if(FAILED(hr = m_pDev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue)))) {
        SafeRelease(&pFactory);
        return hr;
    }

    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC1 scd;

    // clear out the struct for use
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));

    // fill the swap chain description struct
    scd.Width  = g_pApp->GetConfiguration().screenWidth;
    scd.Height = g_pApp->GetConfiguration().screenHeight;
    scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     	        // use 32-bit color
    scd.Stereo = FALSE;
    scd.SampleDesc.Count = 1;                               // multi-samples can not be used when in SwapEffect sets to
                                                            // DXGI_SWAP_EFFECT_FLOP_DISCARD
    scd.SampleDesc.Quality = 0;                             // multi-samples can not be used when in SwapEffect sets to
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
    scd.BufferCount = kFrameCount;                          // back buffer count
    scd.Scaling     = DXGI_SCALING_STRETCH;
    scd.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;        // DXGI_SWAP_EFFECT_FLIP_DISCARD only supported after Win10
                                                            // use DXGI_SWAP_EFFECT_DISCARD on platforms early than Win10
    scd.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
    scd.Flags    = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // allow full-screen transition

    IDXGISwapChain1* pSwapChain;
    if (FAILED(hr = pFactory->CreateSwapChainForHwnd(
                m_pCommandQueue,                            // Swap chain needs the queue so that it can force a flush on it
                hWnd,
                &scd,
                NULL,
                NULL,
                &pSwapChain
                )))
    {
        SafeRelease(&pFactory);
        return hr;
    }

    m_pSwapChain = reinterpret_cast<IDXGISwapChain3*>(pSwapChain);

    m_nFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

    for (uint32_t i = 0; i < kFrameCount; i++)
    {
        if (FAILED(
            hr = m_pDev->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                    IID_PPV_ARGS(&m_pGraphicsFence[i]))
        ))
        {
            return hr;
        }
        
    }
    
    memset(m_nGraphicsFenceValue, 0, sizeof(m_nGraphicsFenceValue));
    m_hGraphicsFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hGraphicsFenceEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (FAILED(hr)) return hr;
    }
    
    
    if (FAILED(hr = CreateDescriptorHeaps())) {
        return hr;
    }

    if (FAILED(hr = CreateRenderTarget())) {
        return hr;
    }

    if (FAILED(hr = CreateDepthStencil())) {
        return hr;
    }

    if (FAILED(hr = CreateCommandList())) {
        return hr;
    }
    
    if (FAILED(hr = CreateConstantBuffer())) {
        return hr;
    }

    if (FAILED(hr = CreateSamplerBuffer())) {
        return hr;
    }

    return hr;
}

int  My::D3d12GraphicsManager::Initialize()
{
    int result = 0;

    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    g_AssetsPath =  assetsPath;
    
    const GfxConfiguration& config = g_pApp->GetConfiguration();
        m_ViewPort = {0.0f,
                      0.0f,
                      static_cast<float>(config.screenWidth),
                      static_cast<float>(config.screenHeight),
                      0.0f,
                      1.0f};
        m_ScissorRect = {0, 0, static_cast<LONG>(config.screenWidth),
                         static_cast<LONG>(config.screenHeight)};
        result = static_cast<int>(CreateGraphicsResources());

    return result;
}

HRESULT My::D3d12GraphicsManager::WaitForPreviousFrame(uint32_t frame_index)
{
    HRESULT hr = S_OK;
    // Wait until the previous frame is finished.
    auto fence = m_nGraphicsFenceValue[frame_index];

    if (m_pGraphicsFence[frame_index]->GetCompletedValue() < fence) {
        if (FAILED(hr = m_pGraphicsFence[frame_index]->SetEventOnCompletion(
                       fence, m_hGraphicsFenceEvent))) {
            assert(0);
            return hr;
        }
        WaitForSingleObject(m_hGraphicsFenceEvent, INFINITE);

        // command list allocators can only be reset when the associated
        // command lists have finished execution on the GPU; apps should use
        // fences to determine GPU execution progress.
        if (SUCCEEDED(hr = m_pGraphicsCommandAllocator[frame_index]->Reset())) {
            // however, when ExecuteCommandList() is called on a particular
            // command list, that command list can then be reset at any time and
            // must be before re-recording.
            hr = m_pGraphicsCommandList[frame_index]->Reset(
                m_pGraphicsCommandAllocator[frame_index], NULL);
        } else {
            assert(0);
        }
    }

    return hr;
}

struct ObjectConstants
{
    // 世界视图投影矩阵
    Matrix4X4f WorldViewProj;
    ObjectConstants(){BuildIdentityMatrix(WorldViewProj);};
};

static bool Init = false;
void My::D3d12GraphicsManager::Tick()
{
    if (!Init)
    {
        Init = true;
        CD3DX12_ROOT_PARAMETER slotRootPram[1];
        CD3DX12_DESCRIPTOR_RANGE cbvTable;
        cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
        slotRootPram[0].InitAsDescriptorTable(1, &cbvTable);
        CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootPram, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        
        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;
        // 序列化根签名
        HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
        if (errorBlob != nullptr)
        {
            ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        ThrowIfFailed(hr);
        ThrowIfFailed(m_pDev->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&m_pRootSignature)));
        
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;
      
        D3DCompileFromFile(GetAssetFullPath(L"color.hlsl").c_str(), nullptr,
                       D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_0",
                       0, 0, &vertexShader, &errorBlob);

        if (errorBlob) {
            OutputDebugString((LPCTSTR)errorBlob->GetBufferPointer());
            errorBlob->Release();
            throw std::exception();
        }

        D3DCompileFromFile(GetAssetFullPath(L"color.hlsl").c_str(), nullptr,
                       D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_0",
                       0, 0, &pixelShader, &errorBlob);
        if (errorBlob) {
            OutputDebugString((LPCTSTR)errorBlob->GetBufferPointer());
            errorBlob->Release();
            throw std::exception();
        }

         D3D12_INPUT_ELEMENT_DESC ied[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psod = {};
        psod.InputLayout = {ied, _countof(ied)};
        psod.pRootSignature = m_pRootSignature;

        psod.VS = {reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()),
               vertexShader->GetBufferSize()};
        psod.PS = {reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()),
                pixelShader->GetBufferSize()};
        psod.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psod.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psod.DepthStencilState.DepthEnable = FALSE;
        psod.DepthStencilState.StencilEnable = FALSE;
        psod.SampleMask = UINT_MAX;
        psod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psod.NumRenderTargets = 1;
        psod.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psod.SampleDesc.Count = 1;
        ThrowIfFailed(m_pDev->CreateGraphicsPipelineState(
            &psod, IID_PPV_ARGS(&m_pPipelineState)));
        
        // ThrowIfFailed(m_pDev->CreateCommandList(
        // 0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pGraphicsCommandAllocator[1],
        // m_pPipelineState, IID_PPV_ARGS(&m_pCommandList)));

     
    }
    
}

void My::D3d12GraphicsManager::Finalize()
{
    SafeRelease(&m_pFence);
    SafeRelease(&m_pVertexBuffer);
    SafeRelease(&m_pCommandList);
    SafeRelease(&m_pPipelineState);
    SafeRelease(&m_pRtvHeap);
    SafeRelease(&m_pRootSignature);
    SafeRelease(&m_pCommandQueue);
    
    for (uint32_t i = 0; i < kFrameCount; i++) {
	    SafeRelease(&m_pRenderTargets[i]);
        SafeRelease(&m_pGraphicsCommandList[i]);
        SafeRelease(&m_pGraphicsCommandAllocator[i]);
    }
	SafeRelease(&m_pSwapChain);
	SafeRelease(&m_pDev);
}


