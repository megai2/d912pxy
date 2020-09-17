/*
MIT License

Copyright(c) 2020 megai2

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

void d912pxy_hlsl_generator::ProcSIO_ADD(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "", " + ", "");
}

void d912pxy_hlsl_generator::ProcSIO_DP2(d912pxy_dxbc9::token* op)
{
	ProcSIO_DOTX(op, 2);
}

void d912pxy_hlsl_generator::ProcSIO_DP3(d912pxy_dxbc9::token* op)
{
	ProcSIO_DOTX(op, 3);
}

void d912pxy_hlsl_generator::ProcSIO_DP4(d912pxy_dxbc9::token* op)
{
	ProcSIO_DOTX(op, 4);
}

void d912pxy_hlsl_generator::ProcSIO_TEXLD(d912pxy_dxbc9::token* op)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);

	d912pxy_dxbc9::token_register* dstReg = &code.FindDstRegToken()->reg;

	OverrideWriteMask(0xF);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);

	dstReg->target.dst.allowDstModForSrc = false;
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 1);

	OverrideWriteMask(0);
	dstReg->target.dst.allowDstModForSrc = true;

	d912pxy_hlsl_generator_regtext pre;
	sprintf_s(pre.t, "dx9texld(%s_deftype, %s_srgb_flag, ", sSrc2.t, sSrc2.t);

	d912pxy_hlsl_generator_regtext mid;
	sprintf_s(mid.t, "t, %ss, ", sSrc2.t);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide2(pre.t, ")", mid.t, sSrc2, sSrc1);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_TEXLDL(d912pxy_dxbc9::token* op)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);

	d912pxy_dxbc9::token_register* dstReg = &code.FindDstRegToken()->reg;
	UINT actualMask = dstReg->target.dst.mask;

	OverrideWriteMask(0xF);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);
	OverrideWriteMask(0x8);
	d912pxy_hlsl_generator_regtext sSrc1W = FormatRegister(0, 1, 0);

	OverrideWriteMask(0xF);
	dstReg->target.dst.allowDstModForSrc = false;
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 1);

	OverrideWriteMask(0);
	dstReg->target.dst.allowDstModForSrc = true;

	d912pxy_hlsl_generator_regtext pre;
	sprintf_s(pre.t, "dx9texldl(%s_deftype, %s_srgb_flag, ", sSrc2.t, sSrc2.t);

	const char* mid[2] = { 0, ", " };
	d912pxy_hlsl_generator_regtext mid0;
	sprintf_s(mid0.t, "t, %ss, ", sSrc2.t);
	mid[0] = mid0.t;

	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3(pre.t, ")", mid, sSrc2, sSrc1, sSrc1W);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_MUL(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "", " * ", "");
}

void d912pxy_hlsl_generator::ProcSIO_MAD(d912pxy_dxbc9::token* op)
{
	const char* mid[2] = { " * ", " ) + " };
	ProcSIO_3OP(op, "(", mid, "");
}

void d912pxy_hlsl_generator::ProcSIO_MOV(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "", "");
}

void d912pxy_hlsl_generator::ProcSIO_REP(d912pxy_dxbc9::token* op)
{
	OverrideWriteMask(0x1);
	d912pxy_hlsl_generator_regtext counter = FormatRegister(0, 0, 0);
	OverrideWriteMask(0);
	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("{ ");
	++procIdent;
	HLSL_GEN_WRITE_PROC("int loopStartCounter%u = %s;", procIdent, counter.t);
	HLSL_GEN_WRITE_PROC("[loop]");
	HLSL_GEN_WRITE_PROC("while (loopStartCounter%u > 0)", procIdent);
	HLSL_GEN_WRITE_PROC("{");
	++procIdent;
	HLSL_GEN_WRITE_PROC("--loopStartCounter%u;", procIdent - 1);
	HLSL_GEN_WRITE_PROC(" ");
}

void d912pxy_hlsl_generator::ProcSIO_ENDREP(d912pxy_dxbc9::token* op)
{
	--procIdent;
	HLSL_GEN_WRITE_PROC("");
	HLSL_GEN_WRITE_PROC("}");
	--procIdent;
	HLSL_GEN_WRITE_PROC("");
	HLSL_GEN_WRITE_PROC("}");
}

void d912pxy_hlsl_generator::ProcSIO_MAX(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "dx9_max(", ", ", ")");
}

void d912pxy_hlsl_generator::ProcSIO_MIN(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "dx9_min(", ", ", ")");
}

void d912pxy_hlsl_generator::ProcSIO_RCP(d912pxy_dxbc9::token* op)
{
	if (IsNaNGuardEnabled(PXY_SDB_HLSL_NAN_GUARD_RCP))
		ProcSIO_1OP(op, "dx9_rcp_guarded(", ")");
	else
		ProcSIO_1OP(op, "dx9_rcp(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_CMP(d912pxy_dxbc9::token* op)
{
	const char* mid[2] = { " >= tmpCmpVec) ? ", " : " };

	//megai2: ensure that all regs are defined, to not bug out their defs in subblock

	RegEnsureDefined(&code.FindDstRegToken()->reg);
	RegEnsureDefined(&code.SubToken(code.FindSrcRegTokenIdx(true, 0))->reg);
	RegEnsureDefined(&code.SubToken(code.FindSrcRegTokenIdx(true, 1))->reg);
	RegEnsureDefined(&code.SubToken(code.FindSrcRegTokenIdx(true, 2))->reg);

	HLSL_GEN_WRITE_PROC("{");
	++procIdent;

	UINT wLen = code.FindDstRegToken()->reg.target.dst.dstLength(writeMaskOverride);
	switch (wLen)
	{
	case 1:
		HLSL_GEN_WRITE_PROC("float tmpCmpVec = 0;");
		break;
	case 2:
		HLSL_GEN_WRITE_PROC("float2 tmpCmpVec = {0, 0};");
		break;
	case 3:
		HLSL_GEN_WRITE_PROC("float3 tmpCmpVec = {0, 0, 0};");
		break;
	case 4:
		HLSL_GEN_WRITE_PROC("float4 tmpCmpVec = {0, 0, 0, 0};");
		break;
	default:
		LOG_ERR_THROW2(-1, "hlsl gen cmp write size not in [1,4] range");
		break;
	}

	ProcSIO_3OP(op, "(", mid, "");
	--procIdent;
	HLSL_GEN_WRITE_PROC("}");
}

void d912pxy_hlsl_generator::ProcSIO_DP2ADD(d912pxy_dxbc9::token* op)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);

	OverrideWriteMask(0x3);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 1);

	OverrideWriteMask(0x1);
	d912pxy_hlsl_generator_regtext sSrc3 = FormatRegister(0, 1, 2);

	OverrideWriteMask(0);

	const char* mid[2] = { ", ", ") + " };
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3("dot(", "", mid, sSrc1, sSrc2, sSrc3);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_FRC(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "dx9_frac(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_POW(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "dx9_pow(", ", ", ")");
}

void d912pxy_hlsl_generator::ProcSIO_RSQ(d912pxy_dxbc9::token* op)
{
	if (IsNaNGuardEnabled(PXY_SDB_HLSL_NAN_GUARD_RSQ))
		ProcSIO_1OP(op, "dx9_rsqrt_guarded(", ")");
	else
		ProcSIO_1OP(op, "dx9_rsqrt(", ")");
}



void d912pxy_hlsl_generator::ProcSIO_NRM(d912pxy_dxbc9::token* op)
{
	if (IsNaNGuardEnabled(PXY_SDB_HLSL_NAN_GUARD_NRM))
		ProcSIO_1OP(op, "dx9_normalize_guarded(", ")");
	else
		ProcSIO_1OP(op, "dx9_normalize(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_LOG(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "dx9_log(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_EXP(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "dx9_exp(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_EXPP(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "dx9_expp(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_TEXKILL(d912pxy_dxbc9::token* op)
{
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(1, 0, 0);

	HLSL_GEN_WRITE_PROC("clip(%s);",
		sSrc1.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_IF(d912pxy_dxbc9::token* op)
{
	OverrideWriteMask(0x1);
	d912pxy_hlsl_generator_regtext boolOp = FormatRegister(0, 0, 0);
	OverrideWriteMask(0);

	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("if (%s)",
		boolOp.t		
	);
	HLSL_GEN_WRITE_PROC("{");
	HLSL_GEN_WRITE_PROC(" ");
	++procIdent;
}


void d912pxy_hlsl_generator::ProcSIO_IFC(d912pxy_dxbc9::token* op)
{
	UINT64 cmpStr = code.Current()->ins.formatCmpString();

	OverrideWriteMask(0x1);
	d912pxy_hlsl_generator_regtext lOp = FormatRegister(0, 0, 0);
	d912pxy_hlsl_generator_regtext rOp = FormatRegister(0, 0, 1);
	OverrideWriteMask(0);

	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("if (%s%s%s)",
		lOp.t,
		(char*)&cmpStr,
		rOp.t
	);
	HLSL_GEN_WRITE_PROC("{");
	HLSL_GEN_WRITE_PROC(" ");
	++procIdent;
}

void d912pxy_hlsl_generator::ProcSIO_ELSE(d912pxy_dxbc9::token* op)
{
	--procIdent;
	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("} else {");
	HLSL_GEN_WRITE_PROC(" ");
	++procIdent;
}

void d912pxy_hlsl_generator::ProcSIO_ENDIF(d912pxy_dxbc9::token* op)
{
	--procIdent;
	HLSL_GEN_WRITE_PROC(" ");
	HLSL_GEN_WRITE_PROC("}");
	HLSL_GEN_WRITE_PROC(" ");
}

void d912pxy_hlsl_generator::ProcSIO_BREAK(d912pxy_dxbc9::token* op)
{
	ProcSIO_IFC(op);
	HLSL_GEN_WRITE_PROC("	break;");
	ProcSIO_ENDIF(op);

}

void d912pxy_hlsl_generator::ProcSIO_LRP(d912pxy_dxbc9::token* op)
{
	//#define asm_lrp(ret,s0,s1,s2) ret = lerp(s2,s1,s0) 
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);
	d912pxy_hlsl_generator_regtext sSrc0 = FormatRegister(0, 1, 0);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 2);

	const char* mid[2] = { ", ", ", " };
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3("dx9_lerp(", ")", mid, sSrc2, sSrc1, sSrc0);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_DSX(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "ddx(", ")");	
}

void d912pxy_hlsl_generator::ProcSIO_DSY(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "ddy(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_SLT(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "(", " < ", ") ? 1.0f : 0.0f");
}

void d912pxy_hlsl_generator::ProcSIO_ABS(d912pxy_dxbc9::token* op)
{
	ProcSIO_1OP(op, "abs(", ")");
}

void d912pxy_hlsl_generator::ProcSIO_SGE(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "(", " >= ", ") ? 1 : 0");
}

void d912pxy_hlsl_generator::ProcSIO_SGN(d912pxy_dxbc9::token* op)
{
	//megai2: TODO
	ProcSIO_2OP(op, "(", " < ", ") ? -1 : 1");
}

void d912pxy_hlsl_generator::ProcSIO_SINCOS(d912pxy_dxbc9::token* op)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);

	d912pxy_dxbc9::token_register* dstReg = &code.FindDstRegToken()->reg;

	DWORD writeMask = dstReg->target.dst.mask;
	OverrideWriteMask(0x1);

	d912pxy_hlsl_generator_regtext sSrc0 = FormatRegister(0, 1, 0);

	HLSL_GEN_WRITE_PROC("{ // sincos");
	++procIdent;

	HLSL_GEN_WRITE_PROC("float2 tmp = {0, 0};");

	if (writeMask & 0x1)
	{
		d912pxy_hlsl_generator_regtext rSide = FormatRightSide1("cos(", ")", sSrc0);

		HLSL_GEN_WRITE_PROC("tmp.x = %s;",
			rSide.t
		);
	}

	if (writeMask & 0x2)
	{
		d912pxy_hlsl_generator_regtext rSide = FormatRightSide1("sin(", ")", sSrc0);

		HLSL_GEN_WRITE_PROC("tmp.y = %s;",
			rSide.t
		);
	}

	OverrideWriteMask(0);

	UINT64 tmpReadMask = dstReg->target.dst.getWriteMaskStr(0);
	HLSL_GEN_WRITE_PROC("%s = tmp%s;", sDst.t, (char*)&tmpReadMask);

	--procIdent;
	HLSL_GEN_WRITE_PROC("}");
}

void d912pxy_hlsl_generator::ProcSIO_DEF(d912pxy_dxbc9::token* op)
{
	d912pxy_dxbc9::token_register* dstReg = &code.FindDstRegToken()->reg;
	UINT dstRegOffset = code.FindDstRegTokenIdx() + 1;

	LOG_DBG_DTDM("def regt = %u wm = %X num = %u", dstReg->regType, dstReg->target.dst.mask, dstReg->regNum);

	if (dstReg->target.dst.mask != 0xF)
		LOG_ERR_THROW2(-1, "hlsl gen stuck due to def dst reg being masked");

	RegDefine(dstReg, true, false);

	d912pxy_dxbc9::token_raw v4[4];

	for (int i = 0; i != 4; ++i)
		v4[i] = code.SubToken(dstRegOffset + i, d912pxy_dxbc9::token_type::raw)->raw;

	switch (dstReg->regType)
	{
	case D3DSPR_CONST:
		HLSL_GEN_WRITE_PROC(
			"float4 %s%u = { %.9f , %.9f , %.9f , %.9f };",
			GetRegTypeStr(dstReg->regType, 1),
			dstReg->regNum,
			v4[0].f, v4[1].f, v4[2].f, v4[3].f
		);
		break;
	case D3DSPR_CONSTINT:
		HLSL_GEN_WRITE_PROC(
			"int4 %s%u = { %li , %li , %li , %li };",
			GetRegTypeStr(dstReg->regType, 1),
			dstReg->regNum,
			v4[0].i32, v4[1].i32, v4[2].i32, v4[3].i32
		);
		break;
	default:
		LOG_ERR_THROW2(-1, "hlsl gen stuck on def sio due to wrong regType");
		break;
	}
}

void d912pxy_hlsl_generator::ProcSIO_DCL_sm1(d912pxy_dxbc9::token* op)
{
	ProcSIO_DCL_sm2(op);//megai2: TODO ... -_-
}

void d912pxy_hlsl_generator::ProcSIO_DCL_sm2(d912pxy_dxbc9::token* op)
{
	d912pxy_dxbc9::token* dstTok;
	d912pxy_dxbc9::token* dclTok;
	ProcSIO_DCL_shared(op, &dstTok, &dclTok);

	UINT usageType = dclTok->dcl.usage;
	UINT dclId = dclTok->dcl.id;
	const char* regNameStr = GetRegTypeStr(dstTok->reg.regType, 0);
	UINT regNum = dstTok->reg.regNum;

	if (verToken.isPS)
	{
		switch (dstTok->reg.regType)
		{
		case D3DSPR_SAMPLER:
		{
			DeclareSampler(op, dclTok, dstTok);
		}
		break;
		case D3DSPR_INPUT:
		{
			HLSL_GEN_WRITE_HEADI(
				HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_NC, regNum),
				"/*default*/    float4 %s%u: PS2X_COLOR_OUT%u;",
				regNameStr, regNum, regNum
			);
		}
		break;
		case D3DSPR_TEXTURE:
		{
			HLSL_GEN_WRITE_HEADI(
				HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_TEXC, regNum),
				"/*default*/    float4 %s%u: TEXCOORD%u;",
				regNameStr, regNum, regNum
			);
		}
		break;
		case D3DSPR_MISCTYPE:
			DeclareMisc(op, dclTok, dstTok);
			break;
		default:
			LOG_ERR_DTDM("hlsl gen dcl ps_2 reg type is %u", dstTok->reg.regType);
			LOG_ERR_THROW2(-1, "hlsl reg type");
			break;
		}
	}
	else {
		switch (dstTok->reg.regType)
		{
		case D3DSPR_SAMPLER:
		{
			LOG_ERR_THROW2(-1, "no samplers in vertex shaders for sm < 3");
		}
		break;
		case D3DSPR_INPUT:
		{
			HLSL_GEN_WRITE_HEADI(
				0,
				"/*default*/    float4 %s%u: %s%uE;",
				regNameStr, regNum, GetUsageString(usageType, 0), dclId
			);
		}
		break;
		default:
			LOG_ERR_THROW2(-1, "sm2 vs dcl op wrong regtype");
			break;
		}
	}
}

void d912pxy_hlsl_generator::ProcSIO_DCL_sm3(d912pxy_dxbc9::token* op)
{
	d912pxy_dxbc9::token* dstTok;
	d912pxy_dxbc9::token* dclTok;
	ProcSIO_DCL_shared(op, &dstTok, &dclTok);

	if (verToken.isPS)
	{
		switch (dstTok->reg.regType)
		{
		case D3DSPR_SAMPLER:
		{
			DeclareSampler(op, dclTok, dstTok);
		}
		break;
		case D3DSPR_INPUT:
		case D3DSPR_TEXTURE:
		{
			UINT regNum = dstTok->reg.regNum;
			UINT priority = HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[dclTok->dcl.usage], dclTok->dcl.id);
			const char* usageStr = GetUsageString(dclTok->dcl.usage, 1);

			HLSL_GEN_WRITE_HEADI(
				priority,
				"/*default*/    float4 %s%u: %s%u;",
				GetRegTypeStr(dstTok->reg.regType, 0), regNum, usageStr, dclTok->dcl.id
			);
		}
		break;
		case D3DSPR_MISCTYPE:
			DeclareMisc(op, dclTok, dstTok);
			break;
		default:
			LOG_ERR_DTDM("hlsl gen dcl ps_3 reg type is %u", dstTok->reg.regType);
			LOG_ERR_THROW2(-1, "hlsl reg type");
			break;
		}
	}
	else {
		UINT usageType = dclTok->dcl.usage;
		UINT dclId = dclTok->dcl.id;
		const char* regNameStr = GetRegTypeStr(dstTok->reg.regType, 0);
		UINT regNum = dstTok->reg.regNum;

		switch (dstTok->reg.regType)
		{
		case D3DSPR_SAMPLER:
		{
			DeclareSampler(op, dclTok, dstTok);
		}
		break;
		case D3DSPR_OUTPUT:
		{
			if ((usageType == D3DDECLUSAGE_POSITION) && (dclId == 0))
			{
				HLSL_GEN_WRITE_HEADO(
					HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0),
					"	float4 %s%u: SV_POSITION;",
					regNameStr, regNum
				);
				HLSL_GEN_WRITE_PROC_PD("#define dx9_halfpixel_pos_reg_ac %s%u", GetRegTypeStr(dstTok->reg.regType, 1), regNum);
			}
			else {
				HLSL_GEN_WRITE_HEADO(
					HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[usageType], dclId),
					"	float4 %s%u: %s%u;",
					regNameStr, regNum, GetUsageString(usageType, 1), dclId
				);
			}
		}
		break;
		case D3DSPR_INPUT:
		{
			const char* inputFmt[2] = {
				"/*default*/     uint4 %s%u: %s%uE;",
				"/*default*/    float4 %s%u: %s%uE;"
			};

			HLSL_GEN_WRITE_HEADI(
				0,
				inputFmt[usageType != D3DDECLUSAGE_BLENDINDICES],
				regNameStr, regNum, GetUsageString(usageType, 0), dclId
			);
		}
		break;
		default:
			LOG_ERR_THROW2(-1, "sm3 vs dcl op wrong regtype");
			break;
		}
	}
}

void d912pxy_hlsl_generator::ProcSIO_DCL_shared(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token** o_dstTok, d912pxy_dxbc9::token** o_dclTok)
{
	//1 is DCL instruct token
	//2 is DST token

	d912pxy_dxbc9::token* dstTok = code.SubToken(2, d912pxy_dxbc9::token_type::destination);
	d912pxy_dxbc9::token* dclTok = code.SubToken(1, d912pxy_dxbc9::token_type::dcl);

	LOG_DBG_DTDM("DCL of %S%u", GetRegTypeStr(dstTok->reg.regType, 0), dstTok->reg.regNum);

	RegDefine(&dstTok->reg, false, true);

	*o_dstTok = dstTok;
	*o_dclTok = dclTok;
}