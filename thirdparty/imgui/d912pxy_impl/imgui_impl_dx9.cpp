// ImGui Win32 + DirectX9 binding
// In this binding, ImTextureID is used to store a 'LPDIRECT3DTEXTURE9' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <imgui.h>
#include "imgui_impl_dx9.h"

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// Data
static HWND                     g_hWnd = 0;
static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9  g_pVB = NULL;
static LPDIRECT3DINDEXBUFFER9   g_pIB = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

//d912pxy ==============

static float g_oldDisplaySize[2] = { 0,0 };
static int g_d912pxy_present = 0;
static IDirect3DVertexShader9* g_pVS = NULL;
static IDirect3DPixelShader9* g_pPS = NULL;
static IDirect3DVertexDeclaration9* g_pVDecl = NULL;
static void* g_pPSO = NULL;
static IDirect3DVertexBuffer9*  g_pVB2 = NULL;
#define D3DRS_ENABLE_D912PXY_API_HACKS (D3DRENDERSTATETYPE)220
#define D3DRS_D912PXY_ENQUEUE_PSO_COMPILE (D3DRENDERSTATETYPE)221
#define D3DRS_D912PXY_SETUP_PSO (D3DRENDERSTATETYPE)222

#define D3DDECLMETHOD_PER_VERTEX_CONSTANT 8

DWORD g_pVS_function[] = {
	0xFFFE0300, 0x0014FFFE, 0x42415443, 0x0000001C, 0x00000023, 0xFFFE0300, 0x00000000, 0x00000000, 0x00000100, 0x0000001C,
	0x335F7376, 0x4D00305F, 0x6F726369, 0x74666F73, 0x29522820, 0x534C4820, 0x6853204C, 0x72656461, 0x6D6F4320, 0x656C6970,
	0x30312072, 0xAB00312E, 0x05000051, 0xA00F0000, 0xBF000000, 0x3F000000, 0x3F800000, 0x00000000, 0x0200001F, 0x80000000,
	0x900F0000, 0x0200001F, 0x80000005, 0x900F0001, 0x0200001F, 0x8000000A, 0x900F0002, 0x0200001F, 0x80010005, 0x900F0003,
	0x0200001F, 0x80000000, 0xE00F0000, 0x0200001F, 0x8000000A, 0xE00F0001, 0x0200001F, 0x80000005, 0xE0030002, 0x04000004,
	0x800F0000, 0x90040000, 0xA0FA0000, 0xA0BF0000, 0x02000001, 0x80060001, 0xA0FF0000, 0x03000002, 0x800F0002, 0xA0140000,
	0x90500003, 0x02000006, 0x80010002, 0x80000002, 0x03000002, 0x80010001, 0x80000002, 0x80000002, 0x03000002, 0x80030003,
	0xA0550000, 0x91E10003, 0x02000006, 0x80010002, 0x80550003, 0x02000006, 0x80010003, 0x80000003, 0x03000002, 0x80020003,
	0x80000003, 0x80000003, 0x03000005, 0x80080001, 0x80000002, 0x80550002, 0x03000009, 0xE0010000, 0x80E40000, 0x80E40001,
	0x02000006, 0x80010000, 0x80FF0002, 0x03000005, 0x80080000, 0x80000000, 0x80AA0002, 0x03000005, 0x80070000, 0xA0FA0000,
	0x90C40000, 0x02000001, 0x800D0003, 0xA0B70000, 0x03000009, 0xE0020000, 0x80E40000, 0x80E40003, 0x02000001, 0xE00C0000,
	0xA0940000, 0x02000001, 0xE00F0001, 0x90C60002, 0x02000001, 0xE0030002, 0x90E40001, 0x0000FFFF	
};

DWORD g_pPS_function[] = {
	0xFFFF0300, 0x001FFFFE, 0x42415443, 0x0000001C, 0x0000004F, 0xFFFF0300, 0x00000001, 0x0000001C, 0x00000100, 0x00000048,
	0x00000030, 0x00000003, 0x00000001, 0x00000038, 0x00000000, 0x44325F73, 0xABABAB00, 0x000C0004, 0x00010001, 0x00000001,
	0x00000000, 0x335F7370, 0x4D00305F, 0x6F726369, 0x74666F73, 0x29522820, 0x534C4820, 0x6853204C, 0x72656461, 0x6D6F4320,
	0x656C6970, 0x30312072, 0xAB00312E, 0x0200001F, 0x8000000A, 0x900F0000, 0x0200001F, 0x80000005, 0x90030001, 0x0200001F,
	0x90000000, 0xA00F0800, 0x03000042, 0x800F0000, 0x90E40001, 0xA0E40800, 0x03000005, 0x800F0800, 0x80E40000, 0x90E40000,
	0x0000FFFF
};

void ImGui_ImplDX9_RenderDrawLists_d912pxy(ImDrawData* draw_data)
{
	// Avoid rendering when minimized
	ImGuiIO& io = ImGui::GetIO();
	if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
		return;

	//megai2: if PSO is not compiled yet, ignore all draws
	if (!g_pPSO)
		return;

	// Create and grow buffers if needed
	if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
	{
		if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
		g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		if (g_pd3dDevice->CreateVertexBuffer(g_VertexBufferSize * sizeof(ImDrawVert), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &g_pVB, NULL) < 0)
			return;
	}
	if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
	{
		if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
		g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		if (g_pd3dDevice->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, NULL) < 0)
			return;
	}

	// Copy and convert all vertices into a single contiguous buffer
	ImDrawVert* vtx_dst;
	ImDrawIdx* idx_dst;
	if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(ImDrawVert)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
		return;
	if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
		return;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	g_pVB->Unlock();
	g_pIB->Unlock();
	g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(ImDrawVert));
	g_pd3dDevice->SetStreamSource(1, g_pVB2, 0, 8);
	g_pd3dDevice->SetIndices(g_pIB);
	
	// Setup viewport
	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = (DWORD)io.DisplaySize.x;
	vp.Height = (DWORD)io.DisplaySize.y;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	g_pd3dDevice->SetViewport(&vp);

	// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);

	// Setup orthographic projection matrix
	// Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
	{
		float R = io.DisplaySize.x;
		float B = io.DisplaySize.y;

		if ((g_oldDisplaySize[0] != R) || (g_oldDisplaySize[1] != B))
		{
			g_oldDisplaySize[0] = R + 0.5f;
			g_oldDisplaySize[1] = B + 0.5f;

			float* viewRect;

			if (g_pVB2->Lock(0, 0, (void**)&viewRect, 0) < 0)
				return;

			memcpy(viewRect, g_oldDisplaySize, 8);

			g_pVB2->Unlock();
		}
	}

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				g_pd3dDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
				g_pd3dDevice->SetScissorRect(&r);

				g_pd3dDevice->GetRenderState(D3DRS_D912PXY_SETUP_PSO, (DWORD*)g_pPSO);

				g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount / 3);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}

	g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	g_pd3dDevice->SetStreamSource(1, NULL, 0, 0);
}

static bool ImGui_ImplDX9_Release_d912pxy_objects()
{
	if (g_pVB2)
	{
		g_pVB2->Release();
		g_pVB2 = NULL;
	}

	if (g_pVS)
	{
		g_pVS->Release();
		g_pVS = NULL;
	}

	if (g_pPS)
	{
		g_pPS->Release();
		g_pPS = NULL;
	}

	if (g_pVDecl)
	{
		g_pVDecl->Release();
		g_pVDecl = NULL;
	}

	g_pPSO = NULL;

	return true;
}

static bool ImGui_ImplDX9_Create_d912pxy_objects()
{
	if (g_pd3dDevice->CreateVertexShader(g_pVS_function, &g_pVS) < 0)
		return false;

	if (g_pd3dDevice->CreatePixelShader(g_pPS_function, &g_pPS) < 0)
		return false;

	/*WORD    Stream;     // Stream index
	WORD    Offset;     // Offset in the stream in bytes
	BYTE    Type;       // Data type
	BYTE    Method;     // Processing method
	BYTE    Usage;      // Semantics
	BYTE    UsageIndex; // Semantic index*/

	D3DVERTEXELEMENT9 vDclElements[] = {
		{0,0,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_POSITION,0},
		{0,8,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
		{0,16,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
		{1,0,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_PER_VERTEX_CONSTANT,D3DDECLUSAGE_TEXCOORD,1},
		{0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}
	};

	if (g_pd3dDevice->CreateVertexDeclaration(&vDclElements[0],&g_pVDecl) < 0)
		return false;

	if (g_pd3dDevice->CreateVertexBuffer(16, 0, 0, D3DPOOL_DEFAULT, &g_pVB2, 0) < 0)
		return false;

	g_pd3dDevice->SetPixelShader(g_pPS);
	g_pd3dDevice->SetVertexShader(g_pVS);
	g_pd3dDevice->SetVertexDeclaration(g_pVDecl);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);	
	g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);	
	
	if (g_pd3dDevice->GetRenderState(D3DRS_D912PXY_ENQUEUE_PSO_COMPILE, (DWORD*)&g_pPSO) < 0)
		return false;

	return true;
}

//d912pxy ==============

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplDX9_RenderDrawLists(ImDrawData* draw_data)
{
    // Avoid rendering when minimized
    ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
        return;

    // Create and grow buffers if needed
    if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
        g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (g_pd3dDevice->CreateVertexBuffer(g_VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL) < 0)
            return;
    }
    if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
        g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (g_pd3dDevice->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, NULL) < 0)
            return;
    }

    // Backup the DX9 state
    IDirect3DStateBlock9* d3d9_state_block = NULL;
    if (g_pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;

    // Copy and convert all vertices into a single contiguous buffer
    CUSTOMVERTEX* vtx_dst;
    ImDrawIdx* idx_dst;
    if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
        return;
    if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
        return;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000)>>16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    g_pVB->Unlock();
    g_pIB->Unlock();
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)io.DisplaySize.x;
    vp.Height = (DWORD)io.DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    g_pd3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
    g_pd3dDevice->SetPixelShader(NULL);
    g_pd3dDevice->SetVertexShader(NULL);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Setup orthographic projection matrix
    // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        const float L = 0.5f, R = io.DisplaySize.x+0.5f, T = 0.5f, B = io.DisplaySize.y+0.5f;
        D3DMATRIX mat_identity = { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } };
        D3DMATRIX mat_projection =
        {
            2.0f/(R-L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f/(T-B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f,
        };
        g_pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
                g_pd3dDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
                g_pd3dDevice->SetScissorRect(&r);
                g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount/3);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}

IMGUI_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        io.MouseDown[0] = true;
        return true;
    case WM_LBUTTONUP:
        io.MouseDown[0] = false;
        return true;
    case WM_RBUTTONDOWN:
        io.MouseDown[1] = true;
        return true;
    case WM_RBUTTONUP:
        io.MouseDown[1] = false;
        return true;
    case WM_MBUTTONDOWN:
        io.MouseDown[2] = true;
        return true;
    case WM_MBUTTONUP:
        io.MouseDown[2] = false;
        return true;
    case WM_MOUSEWHEEL:
        io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
        return true;
    case WM_MOUSEMOVE:
        io.MousePos.x = (signed short)(lParam);
        io.MousePos.y = (signed short)(lParam >> 16);
        return true;
    case WM_KEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return true;
    case WM_KEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return true;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (wParam > 0 && wParam < 0x10000)
            io.AddInputCharacter((unsigned short)wParam);
        return true;
    }
    return 0;
}

bool    ImGui_ImplDX9_Init(void* hwnd, IDirect3DDevice9* device)
{
    g_hWnd = (HWND)hwnd;
    g_pd3dDevice = device;

    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&g_TicksPerSecond))
        return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&g_Time))
        return false;

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

	g_d912pxy_present = device->SetRenderState(D3DRS_ENABLE_D912PXY_API_HACKS, 1) == 343434;

	if (g_d912pxy_present)
	    io.RenderDrawListsFn = ImGui_ImplDX9_RenderDrawLists_d912pxy;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	else 
		io.RenderDrawListsFn = ImGui_ImplDX9_RenderDrawLists;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.

    io.ImeWindowHandle = g_hWnd;

    return true;
}

void ImGui_ImplDX9_Shutdown()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui::Shutdown();
    g_pd3dDevice = NULL;
    g_hWnd = 0;
}

static bool ImGui_ImplDX9_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Upload texture to graphics system
    g_FontTexture = NULL;
    if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_FontTexture, NULL) < 0)
        return false;
    D3DLOCKED_RECT tex_locked_rect;
    if (g_FontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
        return false;
    for (int y = 0; y < height; y++)
        memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
    g_FontTexture->UnlockRect(0);

    // Store our identifier
    io.Fonts->TexID = (void *)g_FontTexture;

    return true;
}

bool ImGui_ImplDX9_CreateDeviceObjects()
{
    if (!g_pd3dDevice)
        return false;
    if (!ImGui_ImplDX9_CreateFontsTexture())
        return false;

	if (g_d912pxy_present)
		return ImGui_ImplDX9_Create_d912pxy_objects();			

    return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;
    if (g_pVB)
    {
        g_pVB->Release();
        g_pVB = NULL;
    }
    if (g_pIB)
    {
        g_pIB->Release();
        g_pIB = NULL;
    }
    if (LPDIRECT3DTEXTURE9 tex = (LPDIRECT3DTEXTURE9)ImGui::GetIO().Fonts->TexID)
    {
        tex->Release();
        ImGui::GetIO().Fonts->TexID = 0;
    }

	if (g_d912pxy_present)
		ImGui_ImplDX9_Release_d912pxy_objects();

    g_FontTexture = NULL;
}

void ImGui_ImplDX9_NewFrame()
{
    if (!g_FontTexture)
        ImGui_ImplDX9_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    GetClientRect(g_hWnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
    if (io.MouseDrawCursor)
        SetCursor(NULL);

    // Start the frame
    ImGui::NewFrame();
}
