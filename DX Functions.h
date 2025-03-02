#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
void DXRender();
int Setup();
void Cleanup();

FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;
static int const                    NUM_BACK_BUFFERS = 3;

extern ID3D12Device* g_pd3dDevice;
extern ID3D12DescriptorHeap* g_pd3dRtvDescHeap;
extern ID3D12DescriptorHeap* g_pd3dSrvDescHeap;
extern ID3D12CommandQueue* g_pd3dCommandQueue;
extern ID3D12GraphicsCommandList* g_pd3dCommandList;
extern ID3D12Fence* g_fence;
extern HANDLE                       g_fenceEvent;
extern UINT64                       g_fenceLastSignaledValue;
extern IDXGISwapChain3* g_pSwapChain;
extern HANDLE                       g_hSwapChainWaitableObject;
extern ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS];
extern D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];
extern HWND hwnd;
extern WNDCLASSEXW wc;
extern int mouse_x, mouse_y;
