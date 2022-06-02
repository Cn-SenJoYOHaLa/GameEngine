#pragma once
#include <stdint.h>
#include <d3d12.h>
#include <DXGI1_4.h>
#include "GraphicsManager.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

namespace My {
    class D3d12GraphicsManager : public GraphicsManager
    {
    public:
       	virtual int Initialize();
	    virtual void Finalize();

	    virtual void Tick();

    private:
        HRESULT CreateDescriptorHeaps();
        HRESULT CreateRenderTarget();
        HRESULT CreateDepthStencil();
        HRESULT CreateCommandList();
        HRESULT CreateGraphicsResources();
        HRESULT CreateSamplerBuffer();
        HRESULT CreateConstantBuffer(const Buffer& buffer);
        HRESULT CreateConstantBuffer();


        HRESULT CreateIndexBuffer(const Buffer& buffer);
        HRESULT CreateVertexBuffer(const Buffer& buffer);
        HRESULT CreateTextureBuffer(const Image& image);

        HRESULT WaitForPreviousFrame(uint32_t frame_index);

    private:
        static const uint32_t           kFrameCount  = 2;
        ID3D12Device*                   m_pDev       = nullptr;             // the pointer to our Direct3D device interface
        D3D12_VIEWPORT                  m_ViewPort;                         // viewport structure
        D3D12_RECT                      m_ScissorRect;                      // scissor rect structure
        IDXGISwapChain3*                m_pSwapChain = nullptr;             // the pointer to the swap chain interface
        ID3D12Resource*                 m_pRenderTargets[kFrameCount];      // the pointer to rendering buffer. [descriptor]
        ID3D12Resource*                 m_pDepthStencilBuffer;              // the pointer to the depth stencil buffer
        ID3D12CommandAllocator*         m_pGraphicsCommandAllocator[kFrameCount]; 
        ID3D12GraphicsCommandList*      m_pGraphicsCommandList[kFrameCount];
        ID3D12CommandQueue*             m_pCommandQueue = nullptr;          // the pointer to command queue
        ID3D12RootSignature*            m_pRootSignature = nullptr;         // a graphics root signature defines what resources are bound to the pipeline
        ID3D12DescriptorHeap*           m_pRtvHeap = nullptr;               // an array of descriptors of GPU objects
        ID3D12DescriptorHeap*           m_pDsvHeap = nullptr;               // an array of descriptors of GPU objects
        ID3D12DescriptorHeap*           m_pCbvSrvUavHeap;                   // an array of descriptors of GPU objects
        ID3D12DescriptorHeap*           m_pSamplerHeap;                     // an array of descriptors of GPU objects
        
        ID3D12DescriptorHeap*           m_pExtraCbvHeap = nullptr;
        
        ID3D12PipelineState*            m_pPipelineState = nullptr;         // an object maintains the state of all currently set shaders
                                                                            // and certain fixed function state objects
                                                                            // such as the input assembler, tesselator, rasterizer and output manager
        ID3D12GraphicsCommandList*      m_pCommandList = nullptr;           // a list to store GPU commands, which will be submitted to GPU to execute when done

        uint32_t                        m_nRtvDescriptorSize;
        uint32_t                        m_nCbvSrvDescriptorSize;

        ID3D12Resource*                 m_pVertexBuffer = nullptr;          // the pointer to the vertex buffer
        D3D12_VERTEX_BUFFER_VIEW        m_VertexBufferView;                 // a view of the vertex buffer
        ID3D12Resource*                 m_pIndexBuffer = nullptr;           // the pointer to the vertex buffer
        D3D12_INDEX_BUFFER_VIEW         m_IndexBufferView;                  // a view of the vertex buffer
        ID3D12Resource*                 m_pTextureBuffer = nullptr;         // the pointer to the texture buffer
        ID3D12Resource*                 m_pConstantUploadBuffer = nullptr;  // the pointer to the depth stencil buffer

        uint8_t*                        m_pPerFrameCbvDataBegin[kFrameCount];
        uint8_t*                        m_pCbvDataBegin;
        ID3D12Resource*                 m_pPerFrameConstantUploadBuffer[kFrameCount];
        // Synchronization objects
        uint32_t                        m_nFrameIndex;
        HANDLE                          m_hGraphicsFenceEvent;;
        ID3D12Fence*                    m_pFence = nullptr;
        ID3D12Fence*                    m_pGraphicsFence[kFrameCount];
        uint64_t                        m_nGraphicsFenceValue[kFrameCount];


        const bool VSYNC_ENABLED = true;
        const float screenDepth = 1000.0f;
        const float screenNear = 0.1f;

        float m_positionX = 0, m_positionY = 0, m_positionZ = -10;
        float m_rotationX = 0, m_rotationY = 0, m_rotationZ = 0;
        Matrix4X4f m_worldMatrix;
        Matrix4X4f m_viewMatrix;
        Matrix4X4f m_projectionMatrix;

        void CalculateCameraPosition();
    };
}
