/*
MIT License

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
#pragma once
#include "stdafx.h"

#define d912pxy_hlsl_generator_op_handler_group_size 97
#define d912pxy_hlsl_generator_op_handler_2_x 0
#define d912pxy_hlsl_generator_op_handler_3_x 1
#define d912pxy_hlsl_generator_op_handler_1_x 2
#define d912pxy_hlsl_generator_op_handler_cnt 3

#define d912pxy_hlsl_generator_max_code_lines (4096*10)
#define d912pxy_hlsl_generator_head_priority_group_size 16
#define d912pxy_hlsl_generator_heado_offset 512
#define d912pxy_hlsl_generator_proc_predef_offset 1024
#define d912pxy_hlsl_generator_proc_offset 1324
#define d912pxy_hlsl_generator_max_line_length 1024

#define HLSL_HIO_PRIOG_POS 1
#define HLSL_HIO_PRIOG_TEXC 2
#define HLSL_HIO_PRIOG_VS_NOT_USED 3
#define HLSL_HIO_PRIOG_NC 30
#define HLSL_HIO_PRIOG_END 31
#define HLSL_HIO_PRIORITY(a,b) (d912pxy_hlsl_generator_head_priority_group_size * (a) + (b))

static const UINT HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[] = {
	1,//D3DDECLUSAGE_POSITION = 0,
	2,//D3DDECLUSAGE_BLENDWEIGHT,   // 1
	3,//D3DDECLUSAGE_BLENDINDICES,  // 2
	4,//D3DDECLUSAGE_NORMAL,        // 3
	5,//D3DDECLUSAGE_PSIZE,         // 4
	14,//D3DDECLUSAGE_TEXCOORD,      // 5
	6,//D3DDECLUSAGE_TANGENT,       // 6
	7,//D3DDECLUSAGE_BINORMAL,      // 7
	8,//D3DDECLUSAGE_TESSFACTOR,    // 8
	9,//D3DDECLUSAGE_POSITIONT,     // 9
	10,//D3DDECLUSAGE_COLOR,         // 10
	15,//D3DDECLUSAGE_FOG,           // 11
	12,//D3DDECLUSAGE_DEPTH,         // 12
	13//D3DDECLUSAGE_SAMPLE// 13
};

#define HLSL_GEN_WRITE_PROC_PD(fmt, ...) WriteProcLinePredef(fmt, __VA_ARGS__)
#define HLSL_GEN_WRITE_PROC(fmt, ...) WriteProcLine(fmt, __VA_ARGS__)
#define HLSL_GEN_WRITE_HEADI(prio, fmt, ...) WriteHeadILine(prio, fmt, __VA_ARGS__)
#define HLSL_GEN_WRITE_HEADO(prio, fmt, ...) WriteHeadOLine(prio, fmt, __VA_ARGS__)
#define HLSL_GEN_VTEXTURE_OFFSET 17

#define HLSL_MAX_REG_FILE_LEN 32

typedef struct d912pxy_hlsl_generator_regtext {
	char t[512];
} d912pxy_hlsl_generator_regtext;

class d912pxy_hlsl_generator;

typedef void (d912pxy_hlsl_generator::*d912pxy_hlsl_generator_sio_handler)(d912pxy_dxbc9::token* op);

#pragma pack(push, 1)
typedef struct d912pxy_hlsl_generator_memout {
	UINT32 size;
	char data[1024];
} d912pxy_hlsl_generator_memout;
#pragma pack(pop)

#define PXY_SDB_HLSL_NAN_GUARD_RCP 1
#define PXY_SDB_HLSL_NAN_GUARD_RSQ 2
#define PXY_SDB_HLSL_NAN_GUARD_RET 4
#define PXY_SDB_HLSL_NAN_GUARD_NRM 8
#define PXY_SDB_HLSL_NAN_GUARD_PS_SHIFT 4

#define PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_SRGBW 1
#define PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_SRGBR 2
#define PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_ALPHATEST 4
#define PXY_SDB_HLSL_SRGB_ALPHATEST_COND_SRGBW 8

class d912pxy_hlsl_generator : public d912pxy_noncom
{
public:
	d912pxy_hlsl_generator(DWORD* src, UINT len, wchar_t* ofn, d912pxy_shader_uid uid);
	~d912pxy_hlsl_generator();
	
	d912pxy_hlsl_generator_memout* Process(UINT toMemory);
		
	static void FillHandlers();
	static UINT allowPP_suffix;
	static UINT32 NaNguard_flag;
	static const wchar_t* commonIncludeOverride;
	
	static void overrideCommonInclude(const wchar_t* relPath);
private:
	const char* GetRegTypeStr(DWORD regType, UINT8 proc);
	const char* GetUsageString(UINT usage, UINT type);

	//dxbc reg access formatters

	d912pxy_hlsl_generator_regtext FormatRegister(bool isDst, bool haveDst, UINT idx);
	d912pxy_hlsl_generator_regtext FormatRegister(UINT tokOffset);
	d912pxy_hlsl_generator_regtext FormatDstRegister(d912pxy_dxbc9::token_register* reg);
	d912pxy_hlsl_generator_regtext FormatSrcRegister(d912pxy_dxbc9::token_register* reg, d912pxy_dxbc9::token_register* dstReg, d912pxy_dxbc9::token_register* adrReg);
	d912pxy_hlsl_generator_regtext FormatRelativeSrcRegister(d912pxy_dxbc9::token_register* reg, d912pxy_dxbc9::token_register* adrReg, UINT64 swizzle);

	d912pxy_hlsl_generator_regtext FormatSrcModifier(const d912pxy_hlsl_generator_regtext& statement, d912pxy_dxbc9::register_target_source* reg);
	d912pxy_hlsl_generator_regtext FormatDstModifier(const d912pxy_hlsl_generator_regtext& statement, d912pxy_dxbc9::register_target* dstReg);
	d912pxy_hlsl_generator_regtext FormatDstModifierForSrc(const d912pxy_hlsl_generator_regtext& statement, d912pxy_dxbc9::register_target* dstReg);
	d912pxy_hlsl_generator_regtext FormatRightSide1(const char* pre, const char* post, const d912pxy_hlsl_generator_regtext& op1);
	d912pxy_hlsl_generator_regtext FormatRightSide2(const char* pre, const char* post, const char* mid, const d912pxy_hlsl_generator_regtext& op1, const d912pxy_hlsl_generator_regtext& op2);
	d912pxy_hlsl_generator_regtext FormatRightSide3(const char* pre, const char* post, const char* mid[2], const d912pxy_hlsl_generator_regtext& op1, const d912pxy_hlsl_generator_regtext& op2, d912pxy_hlsl_generator_regtext op3);


	//sio handlers
	void ProcSIO_DEF(d912pxy_dxbc9::token* op);
	void ProcSIO_DCL_sm1(d912pxy_dxbc9::token* op);
	void ProcSIO_DCL_sm2(d912pxy_dxbc9::token* op);
	void ProcSIO_DCL_sm3(d912pxy_dxbc9::token* op);
	void ProcSIO_DCL_shared(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token** o_dstTok, d912pxy_dxbc9::token** o_dclTok);
	void ProcSIO_DP2(d912pxy_dxbc9::token* op);
	void ProcSIO_DP3(d912pxy_dxbc9::token* op);
	void ProcSIO_DP4(d912pxy_dxbc9::token* op);
	void ProcSIO_TEXLD(d912pxy_dxbc9::token* op);
	void ProcSIO_TEXLDL(d912pxy_dxbc9::token* op);
	void ProcSIO_MUL(d912pxy_dxbc9::token* op);
	void ProcSIO_MAD(d912pxy_dxbc9::token* op);
	void ProcSIO_MOV(d912pxy_dxbc9::token* op);
	void ProcSIO_REP(d912pxy_dxbc9::token* op);
	void ProcSIO_ENDREP(d912pxy_dxbc9::token* op);
	void ProcSIO_MAX(d912pxy_dxbc9::token* op);
	void ProcSIO_MIN(d912pxy_dxbc9::token* op);
	void ProcSIO_RCP(d912pxy_dxbc9::token* op);
	void ProcSIO_CMP(d912pxy_dxbc9::token* op);
	void ProcSIO_DP2ADD(d912pxy_dxbc9::token* op);
	void ProcSIO_FRC(d912pxy_dxbc9::token* op);
	void ProcSIO_POW(d912pxy_dxbc9::token* op);
	void ProcSIO_RSQ(d912pxy_dxbc9::token* op);
	void ProcSIO_NRM(d912pxy_dxbc9::token* op);
	void ProcSIO_LOG(d912pxy_dxbc9::token* op);
	void ProcSIO_EXP(d912pxy_dxbc9::token* op);
	void ProcSIO_EXPP(d912pxy_dxbc9::token* op);
	void ProcSIO_TEXKILL(d912pxy_dxbc9::token* op);
	void ProcSIO_IF(d912pxy_dxbc9::token* op);
	void ProcSIO_IFC(d912pxy_dxbc9::token* op);
	void ProcSIO_ELSE(d912pxy_dxbc9::token* op);
	void ProcSIO_ENDIF(d912pxy_dxbc9::token* op);
	void ProcSIO_BREAK(d912pxy_dxbc9::token* op);
	void ProcSIO_LRP(d912pxy_dxbc9::token* op);
	void ProcSIO_SLT(d912pxy_dxbc9::token* op);	
	void ProcSIO_ABS(d912pxy_dxbc9::token* op);
	void ProcSIO_SGE(d912pxy_dxbc9::token* op);
	void ProcSIO_SGN(d912pxy_dxbc9::token* op);
	void ProcSIO_SINCOS(d912pxy_dxbc9::token* op);
	void ProcSIO_ADD(d912pxy_dxbc9::token* op);
	void ProcSIO_DSX(d912pxy_dxbc9::token* op);
	void ProcSIO_DSY(d912pxy_dxbc9::token* op);

	//generic sio handlers
	void ProcSIO_DOTX(d912pxy_dxbc9::token * op, UINT sz);
	void ProcSIO_3OP(d912pxy_dxbc9::token * op, const char * pre, const char * mid[2], const char * post);
	void ProcSIO_2OP(d912pxy_dxbc9::token * op, const char * pre, const char * mid, const char * post);
	void ProcSIO_1OP(d912pxy_dxbc9::token * op, const char* pre, const char* post);
	
	void ProcSIO_UNK(d912pxy_dxbc9::token* op);

	//declaration subhandlers

	void DeclareSampler(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token* dclTok, d912pxy_dxbc9::token* dstTok);
	void DeclareMisc(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token* dclTok, d912pxy_dxbc9::token* dstTok);

	//process sub funcs
	void WriteShaderHeadData();
	void WriteShaderTailData();
	
	//extra conditional flags
	UINT8 PSpositionUsed;
	UINT8 relLookupDefined;
	UINT16 relLookupGroup;
	UINT8 isDepthOutUsed;

	//write mask override
	DWORD writeMaskOverride;

	void OverrideWriteMask(DWORD wmask)
	{
		writeMaskOverride = wmask;
	}
		
	//sm block data
	d912pxy_dxbc9::token_version verToken;

	INT LoadSMBlock();
	void DumpDisassembly();

	//dxbc stream
	d912pxy_dxbc9 code;
	d912pxy_shader_uid mUID;

	//output file
	FILE * of;
	d912pxy_hlsl_generator_memout* WriteOutput(UINT toMemory);

	//output buffering and sorting
	UINT procIdent;
	char* lines[d912pxy_hlsl_generator_max_code_lines];
	UINT headerOffsetO;
	UINT headerOffsetI;
	UINT procOffsetPredef;
	UINT procOffset;
	UINT mainFunctionDeclStrIdx;

	int WriteProcLinePredef(const char* fmt, ...);
	void WriteProcLine(const char* fmt, ...);
	void WriteHeadILine(UINT prio, const char* fmt, ...);
	void WriteHeadOLine(UINT prio, const char* fmt, ...);

	//megai2: register definition and tracking
	UINT64 regDefined[(D3DSPR_PREDICATE + 1) * HLSL_MAX_REG_FILE_LEN];
	UINT64 regDefinedAsC[(D3DSPR_PREDICATE + 1) * HLSL_MAX_REG_FILE_LEN];
	
	void RegEnsureDefined(d912pxy_dxbc9::token_register* reg);
	void RegDefine(d912pxy_dxbc9::token_register* reg, bool asConstant, bool isIOreg);
	int RegIsDefined(d912pxy_dxbc9::token_register* reg, UINT numOffset);
	
	//misc

	UINT IsNaNGuardEnabled(UINT bit);

	//SIO handlers array for different sm types

	static d912pxy_hlsl_generator_sio_handler SIOhandlers[d912pxy_hlsl_generator_op_handler_group_size*d912pxy_hlsl_generator_op_handler_cnt];		
};

