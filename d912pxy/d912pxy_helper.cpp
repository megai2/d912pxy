/*
MIT License

Copyright(c) 2018 Jeremiah van Oosten
Copyright(c) 2018-2019 megai2

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
#include "d912pxy_helper.h"

#ifdef _DEBUG
	#define _DEBUG_DX 
#endif

using namespace Microsoft::WRL;
using namespace d912pxy_helper;

IP7_Trace* m_log = NULL;
IP7_Trace::hModule m_helperLGM;

UINT s_crashLine = 0;

D3D_FEATURE_LEVEL usingFeatures = D3D_FEATURE_LEVEL_11_0;


//megai2: from https://stackoverflow.com/a/41480827
LONG NTAPI d912pxy_helper::VexHandler(PEXCEPTION_POINTERS ExceptionInfo)
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
				GetLogger()->P7_ERROR(m_helperLGM, L"%s", pwz);

//				WriteConsoleW(hOut, pwz, len - 1, &len, 0);
			}

		}
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void d912pxy_helper::InstallVehHandler()
{
	AddVectoredExceptionHandler(TRUE, VexHandler);
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
		wsprintf(buf, L"%u | %S | hr = %08lX \r\n", s_crashLine, reason, hr);
		++s_crashLine;

		FILE* f = fopen("d912pxy_crash.txt", "ab");
		fwrite(buf, 2, lstrlenW(buf), f);
		fclose(f);

		//	MessageBox(0, buf, L"d912pxy", MB_ICONERROR);
		throw std::exception();
	}
}

void d912pxy_helper::d3d12_EnableDebugLayer()
{
#if defined(_DEBUG_DX)
	// Always enable the debug layer before doing anything DX12 related
	// so all possible errors generated while creating DX12 objects
	// are caught by the debug layer.
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)), "dbg layer manipulation error");
	debugInterface->EnableDebugLayer();
#endif
}

typedef HRESULT (WINAPI *dxgi_dbg_get_proto)(REFIID riid, void **ppDebug);
typedef GUID DXGI_DEBUG_ID;

void d912pxy_helper::d3d12_ReportLeaks()
{
#ifdef _DEBUG
	ComPtr<IDXGIDebug> debugItf;

	const GUID DXGI_DEBUG_ALL
		= { 0xe48ae283,  0xda80, 0x490b, { 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 } };

	HMODULE hModule = NULL;
	dxgi_dbg_get_proto pfn = NULL;

	hModule = LoadLibraryA("Dxgidebug.dll");
	if (hModule)
	{
		pfn = (dxgi_dbg_get_proto)GetProcAddress(hModule, "DXGIGetDebugInterface");
		if (pfn)
		{
			ThrowIfFailed(pfn(IID_PPV_ARGS(&debugItf)), "dxgi debug error 2");
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

IP7_Trace* d912pxy_helper::GetLogger()
{
	//create P7 trace object 1

	if (m_log == NULL)
	{
		IP7_Client *l_pClient = P7_Get_Shared(TM("logger"));
		m_log = P7_Create_Trace(l_pClient, TM("d912pxy"));
		m_log->Register_Thread(TM("main thread"), 0);

		m_log->Register_Module(L"helper", &m_helperLGM);

		l_pClient->Release();		
	}

	return m_log;
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
	case D3DFMT_A16B16G16R16: return DXGI_FORMAT_R16G16B16A16_TYPELESS; break;//36,
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
		case D3DFMT_NULL: return DXGI_FORMAT_UNKNOWN; break;//megai2: ignore it
		default: 
		{
			GetLogger()->P7_ERROR(m_helperLGM, L"D3D9 fmt to DXGI fmt not matched for %u", fmt);
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