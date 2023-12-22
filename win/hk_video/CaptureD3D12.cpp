#include "CaptureD3D12.h"

#include <atlbase.h>
#include "common/RLog.h"

namespace tc
{
	ID3D12Device* device = nullptr;
	HRESULT APIENTRY hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags) {
		LOG_INFO("Hook12 present ...");

		if (!device) {
			HRESULT hr = pSwapChain->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&device));
			if (FAILED(hr)) {
				LOG_INFO("%s: Failed to get device from swap, #0x%08X\n", __func__, hr);
				return hr;
			}
			LOG_INFO("GetD3D12 Success !");
			//device_->GetImmediateContext(&context_);
			//ULONG ref = context_->Release();
			//LOG_INFO(" // ID3D11DeviceContext::Release() = %d", ref);
			//auto ref = device->Release();
			//LOG_INFO(" // ID3D11Device::Release() = %d", ref);
		}

		return origin_Present(pSwapChain, SyncInterval, Flags);
	}

	HRESULT Hook_ResizeBuffers(IDXGISwapChain3* pSwapChain,
		UINT        BufferCount,
		UINT        Width,
		UINT        Height,
		DXGI_FORMAT NewFormat,
		UINT        SwapChainFlags
	) {
		LOG_INFO("Hook_ResizeBuffers %d x %d", Width, Height);
		return origin_ResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}

	void Hook_ExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* ppCommandLists) {
		auto cmd_list = dynamic_cast<ID3D12GraphicsCommandList*>(ppCommandLists);
		LOG_INFO("----> ExecCommandLists %p", cmd_list);
		origin_ExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
	}

	void APIENTRY Hook_DrawInstanced(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {
		LOG_INFO("++++> hkDrawInstanced");
		return oDrawInstanced(dCommandList, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
	}

	void APIENTRY Hook_DrawIndexedInstanced(ID3D12GraphicsCommandList* dCommandList, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
		LOG_INFO("****> hkDrawIndexedInstanced");
		return oDrawIndexedInstanced(dCommandList, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
	}

	HRESULT Hook_CreateCommandList(
		ID3D12Device* Device,
		UINT nodeMask,
		D3D12_COMMAND_LIST_TYPE type,
		ID3D12CommandAllocator* pCommandAllocator,
		ID3D12PipelineState* pInitialState,
		REFIID riid,
		void** ppCommandList) {

		LOG_INFO("Create CommandList -=-=-=-=-=-=-= %p ", origin_CreateCommandList);
		HRESULT result = origin_CreateCommandList(device, nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);
		LOG_INFO("Create CommandList -=-=-=-=-=-=-= result %p", result);
		return result;
	}

	CaptureD3D12::CaptureD3D12() {

	}

	CaptureD3D12::~CaptureD3D12() {

	}

	bool CaptureD3D12::InitWindow() {

		WindowClass.cbSize = sizeof(WNDCLASSEXA);
		WindowClass.style = CS_HREDRAW | CS_VREDRAW;
		WindowClass.lpfnWndProc = DefWindowProc;
		WindowClass.cbClsExtra = 0;
		WindowClass.cbWndExtra = 0;
		WindowClass.hInstance = GetModuleHandle(NULL);
		WindowClass.hIcon = NULL;
		WindowClass.hCursor = NULL;
		WindowClass.hbrBackground = NULL;
		WindowClass.lpszMenuName = NULL;
		WindowClass.lpszClassName = "MJ";
		WindowClass.hIconSm = NULL;
		RegisterClassExA(&WindowClass);
		WindowHwnd = CreateWindowA(WindowClass.lpszClassName, "DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);
		if (WindowHwnd == NULL) {
			LOG_INFO("CreateWindow Failed for d3d12.");
			return false;
		}
		return true;
	}

	bool CaptureD3D12::DeleteWindow() {
		DestroyWindow(WindowHwnd);
		UnregisterClassA(WindowClass.lpszClassName, WindowClass.hInstance);
		if (WindowHwnd != NULL) {
			return false;
		}
		return true;
	}

	bool CaptureD3D12::Init() {
		LOG_INFO("CaptureD3D12 INIT>>>>>");
		if (InitWindow() == false) {
			LOG_INFO("Init window failed for d3d12.");
			return false;
		}


		HMODULE D3D12Module = GetModuleHandleA("d3d12.dll");
		HMODULE DXGIModule = GetModuleHandleA("dxgi.dll");
		if (DXGIModule == NULL) {
			LOG_INFO("DXGIModule == NULL");
			LoadLibraryA("dxgi");
			DXGIModule = GetModuleHandleA("dxgi.dll");
		}

		if (D3D12Module == NULL) {
			LOG_INFO("D3D12Module == NULL");
			LoadLibraryA("D3D12.dll");
			D3D12Module = GetModuleHandleA("d3d12.dll");
			if (!D3D12Module) {
				DeleteWindow();
				return false;
			}
		}

		LOG_INFO("Continue to hookd3d12 for capture");

		void* CreateDXGIFactory = GetProcAddress(DXGIModule, "CreateDXGIFactory");
		if (CreateDXGIFactory == NULL) {
			DeleteWindow();
			return false;
		}

		IDXGIFactory* Factory;
		if (((long(__stdcall*)(const IID&, void**))(CreateDXGIFactory))(__uuidof(IDXGIFactory), (void**)&Factory) < 0) {
			DeleteWindow();
			return false;
		}

		IDXGIAdapter* Adapter;
		if (Factory->EnumAdapters(0, &Adapter) == DXGI_ERROR_NOT_FOUND) {
			DeleteWindow();
			return false;
		}

		void* D3D12CreateDevice = GetProcAddress(D3D12Module, "D3D12CreateDevice");
		if (D3D12CreateDevice == NULL) {
			DeleteWindow();
			return false;
		}

		ID3D12Device* Device;
		if (((long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**))(D3D12CreateDevice))(Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&Device) < 0) {
			DeleteWindow();
			return false;
		}

		D3D12_COMMAND_QUEUE_DESC QueueDesc;
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		QueueDesc.Priority = 0;
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.NodeMask = 0;

		ID3D12CommandQueue* CommandQueue;
		if (Device->CreateCommandQueue(&QueueDesc, __uuidof(ID3D12CommandQueue), (void**)&CommandQueue) < 0) {
			DeleteWindow();
			return false;
		}

		ID3D12CommandAllocator* CommandAllocator;
		if (Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&CommandAllocator) < 0) {
			DeleteWindow();
			return false;
		}

		ID3D12GraphicsCommandList* CommandList;
		if (Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&CommandList) < 0) {
			DeleteWindow();
			return false;
		}

		DXGI_RATIONAL RefreshRate;
		RefreshRate.Numerator = 60;
		RefreshRate.Denominator = 1;

		DXGI_MODE_DESC BufferDesc;
		BufferDesc.Width = 100;
		BufferDesc.Height = 100;
		BufferDesc.RefreshRate = RefreshRate;
		BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC SampleDesc;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;

		DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
		SwapChainDesc.BufferDesc = BufferDesc;
		SwapChainDesc.SampleDesc = SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 2;
		SwapChainDesc.OutputWindow = WindowHwnd;
		SwapChainDesc.Windowed = 1;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* SwapChain;
		if (Factory->CreateSwapChain(CommandQueue, &SwapChainDesc, &SwapChain) < 0) {
			DeleteWindow();
			return false;
		}

		MethodsTable = (uintx_t*)::calloc(150, sizeof(uintx_t));
		memcpy(MethodsTable, *(uintx_t**)Device, 44 * sizeof(uintx_t));
		memcpy(MethodsTable + 44, *(uintx_t**)CommandQueue, 19 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19, *(uintx_t**)CommandAllocator, 9 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9, *(uintx_t**)CommandList, 60 * sizeof(uintx_t));
		memcpy(MethodsTable + 44 + 19 + 9 + 60, *(uintx_t**)SwapChain, 18 * sizeof(uintx_t));

		Device->Release();
		Device = NULL;
		CommandQueue->Release();
		CommandQueue = NULL;
		CommandAllocator->Release();
		CommandAllocator = NULL;
		CommandList->Release();
		CommandList = NULL;
		SwapChain->Release();
		SwapChain = NULL;
		DeleteWindow();
		LOG_INFO("After obtain method");

		NTSTATUS status;
		origin_Present = (FUNC_Present)MethodsTable[140];
		tc::HookAllThread(api_Present, origin_Present, hkPresent);

		origin_ResizeBuffers = (FUNC_ResizeBuffers)MethodsTable[145];
		tc::HookAllThread(api_ResizeBuffers, origin_ResizeBuffers, Hook_ResizeBuffers);

		origin_ExecuteCommandLists = (FUNC_ExecuteCommandLists)MethodsTable[54];
		tc::HookAllThread(api_ExecuteCommandLists, origin_ExecuteCommandLists, Hook_ExecuteCommandLists);

		//origin_CreateCommandList = (FUNC_CreateCommandList)MethodsTable[12];
		//status = tc::HookAllThread(api_CreateCommandList, origin_CreateCommandList, Hook_CreateCommandList);
		//if (!NT_SUCCESS(status)) {
		//	LOG_ERROR("Hook CreateCommandList failed");
		//}

		LOG_INFO("Init D3D12 hook. ::::");

		is_hooked = true;
		return true;
	}

	bool CaptureD3D12::IsHooked() {
		return is_hooked;
	}

}