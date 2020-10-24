/*
MIT License

Copyright(c) 2018 Jeremiah van Oosten
Copyright(c) 2018-2020 megai2

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "stdafx.h"
#include "d912pxy_stackwalker.h"

using namespace Microsoft::WRL;
using namespace d912pxy_helper;

static d912pxy_log_module LGC_DEFAULT;

LONG NTAPI d912pxy_helper::VexHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
	PEXCEPTION_RECORD ExceptionRecord = ExceptionInfo->ExceptionRecord;

	LOG_ERR_DTDM("Exception: %lX", ExceptionRecord->ExceptionCode);	

	switch (ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_STACK_OVERFLOW:		
	case EXCEPTION_PRIV_INSTRUCTION:
	{
		d912pxy_StackWalker sw(0x3F,0);		
		sw.ShowCallstack();
		return EXCEPTION_CONTINUE_SEARCH;
	}		
	default:		
		return EXCEPTION_CONTINUE_SEARCH;
	}	
}

//megai2: from https://stackoverflow.com/a/41480827
LONG NTAPI d912pxy_helper::VexDbgHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
	PEXCEPTION_RECORD ExceptionRecord = ExceptionInfo->ExceptionRecord;

	switch (ExceptionRecord->ExceptionCode)
	{
	case DBG_PRINTEXCEPTION_WIDE_C:
	case DBG_PRINTEXCEPTION_C:

		if (ExceptionRecord->NumberParameters >= 2)
		{
			ULONG len = (ULONG)ExceptionRecord->ExceptionInformation[0];

			union {
				ULONG_PTR up;
				PCWSTR pwz;
				PCSTR psz;
			};

			up = ExceptionRecord->ExceptionInformation[1];

			HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);

			if (ExceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C)
			{
				// localized text will be incorrect displayed, if used not CP_OEMCP encoding 
				// WriteConsoleA(hOut, psz, len, &len, 0);

				// assume CP_ACP encoding
				if (ULONG n = MultiByteToWideChar(CP_ACP, 0, psz, len, 0, 0))
				{
					PWSTR wz = (PWSTR)alloca(n * sizeof(WCHAR));

					if (len = MultiByteToWideChar(CP_ACP, 0, psz, len, wz, n))
					{
						pwz = wz;
					}
				}
			}

			if (len)
			{
				LOG_ERR_DTDM("%s", pwz);

				//megai2: relay dbg print messages to crashlog too
				d912pxy_s.log.text.WriteCrashLogLine((wchar_t*)pwz);

				//megai2: check for D3D12 device removal message
				if (wcsstr(pwz, L"D3D12: Removing Device"))
				{
					if (d912pxy_s.dx12.dev)
						LOG_ERR_DTDM("Recived D3D12 device removal message due to %lX", d912pxy_s.dx12.dev->GetDeviceRemovedReason());
					else
						LOG_ERR_DTDM("Recived D3D12 device removal message due to unknown error");

					d912pxy_StackWalker sw(0x3F,0);
					sw.ShowCallstack();


					ExceptionInfo->ExceptionRecord->ExceptionCode = EXCEPTION_NONCONTINUABLE_EXCEPTION;
				}
			}

		}
		return EXCEPTION_CONTINUE_EXECUTION;
	default:
		return VexHandler(ExceptionInfo);
	}	
}

void d912pxy_helper::InitLogModule()
{
	d912pxy_s.log.text.RegisterModule(L"helper", &LGC_DEFAULT);
}

void d912pxy_helper::InstallVehHandler()
{
	AddVectoredExceptionHandler(TRUE, VexDbgHandler);	
}

int d912pxy_helper::IsFileExist(const char *name)
{
	struct stat   buffer;
	return (stat(name, &buffer) == 0);
}


void d912pxy_helper::ThrowIfFailed(HRESULT hr, const char* reason)
{
	if (FAILED(hr))
	{
		wchar_t buf[1024];
		wsprintf(buf, L"%S | hr = %08lX", reason, hr);

		d912pxy_s.log.text.SyncCrashWrite(1);

		d912pxy_s.log.text.WriteCrashLogLine(buf);

		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			if (d912pxy_s.dx12.dev)
				wsprintf(buf, L"D3D12 device removal due to %lX", d912pxy_s.dx12.dev->GetDeviceRemovedReason());
			else
				wsprintf(buf, L"D3D12 device removal due to unknown error");

			d912pxy_s.log.text.WriteCrashLogLine(buf);
		}
		
		d912pxy_s.log.text.SyncCrashWrite(0);
			
		throw std::exception();
	}
}

void d912pxy_helper::d3d12_EnableDebugLayer()
{
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)), "dbg layer manipulation error");
	debugInterface->EnableDebugLayer();
}

typedef HRESULT (WINAPI *dxgi_dbg_get_proto)(REFIID riid, void **ppDebug);
typedef GUID DXGI_DEBUG_ID;

void d912pxy_helper::d3d12_ReportLeaks()
{
#ifdef _DEBUG
	static HMODULE hModule = LoadLibraryA("Dxgidebug.dll");
	if (hModule)
	{
		const auto pfn = (dxgi_dbg_get_proto)GetProcAddress(hModule, "DXGIGetDebugInterface");
		if (pfn)
		{
			ComPtr<IDXGIDebug> debugItf;
			ThrowIfFailed(pfn(IID_PPV_ARGS(&debugItf)), "dxgi debug error 2");

			const GUID DXGI_DEBUG_ALL
				= { 0xe48ae283,  0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

			debugItf->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		}
	}
#endif
}

ComPtr<ID3D12CommandQueue> d912pxy_helper::CreateCommandQueue(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)), "Device CCQ");

	return d3d12CommandQueue;
}

DXGI_FORMAT d912pxy_helper::DXGIFormatFromDX9FMT(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_UNKNOWN: return DXGI_FORMAT_UNKNOWN; break;//0,
	case D3DFMT_A8R8G8B8: return DXGI_FORMAT_B8G8R8A8_UNORM; break;//21,
	case D3DFMT_X8R8G8B8: return DXGI_FORMAT_B8G8R8X8_UNORM; break;//22,
	case D3DFMT_R5G6B5: return DXGI_FORMAT_B5G6R5_UNORM; break;//23,
	case D3DFMT_X1R5G5B5: return DXGI_FORMAT_B5G5R5A1_UNORM; break;//24,
	case D3DFMT_A1R5G5B5: return DXGI_FORMAT_B5G5R5A1_UNORM; break;//25,
	case D3DFMT_A4R4G4B4: return DXGI_FORMAT_B4G4R4A4_UNORM; break;//26,
	case D3DFMT_A8: return DXGI_FORMAT_A8_UNORM; break;//28,
	case D3DFMT_A8R3G3B2: return DXGI_FORMAT_UNKNOWN; break;//29,
	case D3DFMT_X4R4G4B4: return DXGI_FORMAT_B4G4R4A4_UNORM; break;//30,
	case D3DFMT_A2B10G10R10: return DXGI_FORMAT_R10G10B10A2_TYPELESS; break;//31,
	case D3DFMT_A8B8G8R8: return DXGI_FORMAT_R8G8B8A8_UNORM; break;//32,
	case D3DFMT_X8B8G8R8: return DXGI_FORMAT_R8G8B8A8_UNORM; break;//33,
	case D3DFMT_G16R16: return DXGI_FORMAT_R16G16_UNORM; break;//34,
	case D3DFMT_A2R10G10B10: return DXGI_FORMAT_R10G10B10A2_TYPELESS; break;//35,
	case D3DFMT_A16B16G16R16: return DXGI_FORMAT_R16G16B16A16_UNORM; break;//36,
	case D3DFMT_A8P8: return DXGI_FORMAT_A8P8; break;//40,
	case D3DFMT_P8: return DXGI_FORMAT_P8; break;//41,
	case D3DFMT_L8: return DXGI_FORMAT_R8_UNORM; break;//50,
	case D3DFMT_A8L8: return DXGI_FORMAT_R8G8_UNORM; break;//51,
		case D3DFMT_DXT1: return DXGI_FORMAT_BC1_UNORM; break;//MAKEFOURCC('D', 'X', 'T', '1'),
		case D3DFMT_DXT2: return DXGI_FORMAT_BC2_UNORM; break;//MAKEFOURCC('D', 'X', 'T', '3'),
		case D3DFMT_DXT3: return DXGI_FORMAT_BC2_UNORM; break;//MAKEFOURCC('D', 'X', 'T', '3'),
		case D3DFMT_DXT4: return DXGI_FORMAT_BC3_UNORM; break;//MAKEFOURCC('D', 'X', 'T', '5'),
		case D3DFMT_DXT5: return DXGI_FORMAT_BC3_UNORM; break;//MAKEFOURCC('D', 'X', 'T', '5'),
		case D3DFMT_D16_LOCKABLE: return DXGI_FORMAT_D16_UNORM; break;//70,
		case D3DFMT_D32: return DXGI_FORMAT_D32_FLOAT; break;//71,
		case D3DFMT_D24S8: return DXGI_FORMAT_D24_UNORM_S8_UINT; break;//75,
		case D3DFMT_D24X8: return DXGI_FORMAT_D24_UNORM_S8_UINT; break;//77,
		case D3DFMT_R16F: return DXGI_FORMAT_R16_FLOAT; break;//111,
		case D3DFMT_G16R16F: return DXGI_FORMAT_R16G16_FLOAT; break;//112,
		case D3DFMT_A16B16G16R16F: return DXGI_FORMAT_R16G16B16A16_FLOAT; break;//113,
		case D3DFMT_R32F: return DXGI_FORMAT_R32_FLOAT ; break;//114,
		case D3DFMT_G32R32F: return DXGI_FORMAT_R32G32_FLOAT; break;//115,
		case D3DFMT_V8U8: return DXGI_FORMAT_R8G8_SNORM; break;
		case D3DFMT_D16: return DXGI_FORMAT_D16_UNORM; break;
		case D3DFMT_INTZ: return DXGI_FORMAT_R32_TYPELESS; break;//FOURCC INTZ
		case 0x32495441: return DXGI_FORMAT_BC5_UNORM; break;//ATI2
		case D3DFMT_A32B32G32R32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case D3DFMT_X8L8V8U8: return DXGI_FORMAT_B8G8R8X8_UNORM;
		
		case D3DFMT_NULL: return DXGI_FORMAT_UNKNOWN; break;//megai2: ignore it
		default: 
		{
			LOG_ERR_DTDM("D3D9 fmt to DXGI fmt not matched for %u", fmt);
			return DXGI_FORMAT_UNKNOWN;
		}
	}
}

UINT8 d912pxy_helper::BitsPerPixel(DXGI_FORMAT fmt)
{
	switch (static_cast<int>(fmt))
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 128;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 96;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		return 64;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_YUY2:
		//case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
		//case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
		//case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
		return 32;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		//case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
		//case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
		//case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
		//case WIN10_DXGI_FORMAT_V408:
		return 24;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		//case WIN10_DXGI_FORMAT_P208:
		//case WIN10_DXGI_FORMAT_V208:
		return 16;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_NV11:
		return 12;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
		//case XBOX_DXGI_FORMAT_R4G4_UNORM:
		return 8;

	case DXGI_FORMAT_R1_UNORM:
		return 1;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		return 4;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return 8;

	default:
		ThrowIfFailed(-1, "DXGI_FORMAT size");
		return 0;
	}
}

static char CPUBrandString[0x40] = { 0 };


//megai2: some stackoverflow.com code. Thanks to Peter Thaus.
BOOL d912pxy_helper::GetTrueWindowsVersion(OSVERSIONINFOEX* pOSversion)
{
	// Function pointer to driver function
	DWORD (WINAPI *pRtlGetVersion)(
		PRTL_OSVERSIONINFOW lpVersionInformation) = NULL;

	// load/get the System-DLL
	HINSTANCE hNTdllDll = LoadLibrary(L"ntdll.dll");

	BOOL ret;

	// successfully loaded?
	if (hNTdllDll != NULL)
	{
		// get the function pointer to RtlGetVersion
		pRtlGetVersion = (DWORD (WINAPI *)(PRTL_OSVERSIONINFOW))
			GetProcAddress(hNTdllDll, "RtlGetVersion");

		// if successfull then read the function
		if (pRtlGetVersion != NULL)
			ret = (pRtlGetVersion((PRTL_OSVERSIONINFOW)pOSversion) & 0x3) == 0;
	} 

	// if function failed, get out of here
	if (pRtlGetVersion == NULL)
		ret = FALSE;
	
	return ret;
} 

char * d912pxy_helper::GetCPUBrandString()
{
	if (CPUBrandString[0] != 0)
		return &CPUBrandString[0];

	int CPUInfo[4] = { -1 };
	unsigned   nExIds, i = 0;	
	// Get the information associated with each extended ID.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	for (i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}

	return &CPUBrandString[0];
}

char * d912pxy_helper::StrGetCurrentLineStart(char * buffer)
{
	char* itr = buffer;

	while (itr[0] != '\n')
	{
		--itr;
	}

	return itr+1;
}

char * d912pxy_helper::StrNextLine(char * buffer)
{
	char* itr = buffer;

	while (itr[0] != '\n')
	{
		++itr;
	}

	return itr + 1;
}

bool d912pxy_helper::StrCutLastElementInPath(char* fn)
{
	char* lastDlmt = nullptr;
	while (*fn != 0)
	{
		if ((*fn == '\\') && (*(fn + 1) != 0))
			lastDlmt = fn;
		++fn;
	}

	if (lastDlmt)
	{
		++lastDlmt;
		*lastDlmt = 0;
		return true;
	} 

	return false;
}

UINT64 d912pxy_helper::GetClosestPow2(UINT64 size)
{
	UINT64 pow2 = 1;

	for (int i = 0; i != 64; ++i)
	{
		pow2 = 1ULL << (63ULL - i);
		if (pow2 & size)
		{
			if ((pow2 - 1) & size)
				return 63 - i + 1;
			else
				return 63 - i;

		}
	}

	return 63;
}

UINT64 d912pxy_helper::AlignValueByPow2(UINT64 val, UINT64 pow2val)
{
	UINT64 mask = (pow2val - 1);
	if (val & mask)
		return (val & ~mask) + pow2val;
	else
		return val;
}

d912pxy_file_path* currentFilePath = (d912pxy_file_path*)d912pxy_file_paths_default;

d912pxy_file_path * d912pxy_helper::GetFilePath(d912pxy_file_path_id fpId)
{
	return &currentFilePath[fpId];
}

void d912pxy_helper::SwitchFilePaths(d912pxy_file_path * newFpArray)
{
	currentFilePath = newFpArray;
}

bool d912pxy_helper::IsKeyDown(int vkcode)
{
	return (GetKeyState(vkcode) & 0x8000) != 0;
}

INT64 d912pxy_helper::SafeDiv(INT64 a, INT64 b)
{
	if (b)
		return a / b;
	else
		return 0;
}

wchar_t* d912pxy_helper::strdupw(const wchar_t* s)
{
	//TODO: u=ugly, clean this up
	wchar_t* ret = nullptr;
	int l = lstrlenW(s)+1;
	d912pxy_mem_block::allocZero(&ret, l);
	memcpy(ret, s, l * sizeof(wchar_t));
	return ret;
}
