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
#include "stdafx.h"

UINT d912pxy_hlsl_generator::allowPP_suffix = 0;
UINT32 d912pxy_hlsl_generator::NaNguard_flag = 0;
const wchar_t* d912pxy_hlsl_generator::commonIncludeOverride = nullptr;

d912pxy_hlsl_generator::d912pxy_hlsl_generator(DWORD * src, UINT len, wchar_t * ofn, d912pxy_shader_uid uid) : 
	d912pxy_noncom(L"hlsl generator"),
	code(src)
{
	PSpositionUsed = 0;
	procIdent = 0;
	mUID = uid;
	
#ifdef _DEBUG
	LOG_DBG_DTDM("generating hlsl file %s", ofn);
	of = _wfopen(ofn, L"wb");
#endif

	procOffsetPredef = d912pxy_hlsl_generator_proc_predef_offset;
	headerOffsetI = 0;
	headerOffsetO = d912pxy_hlsl_generator_heado_offset;
	procOffset = d912pxy_hlsl_generator_proc_offset;
	ZeroMemory(lines, sizeof(char*) * d912pxy_hlsl_generator_max_code_lines);
	ZeroMemory(regDefined, 8 * ((D3DSPR_PREDICATE + 1) * HLSL_MAX_REG_FILE_LEN));
	ZeroMemory(regDefinedAsC, 8 * ((D3DSPR_PREDICATE + 1) * HLSL_MAX_REG_FILE_LEN));
	relLookupDefined = 0;
	relLookupGroup = 0;
	isDepthOutUsed = 0;
	writeMaskOverride = 0;
}

d912pxy_hlsl_generator::~d912pxy_hlsl_generator()
{
#ifdef _DEBUG
	fflush(of);
	fclose(of);
#endif
}

void d912pxy_hlsl_generator::overrideCommonInclude(const wchar_t* relPath)
{
	d912pxy::error::check(commonIncludeOverride == nullptr, L"Failed to override hlsl common include to %s as it is already overrided to %s", relPath, commonIncludeOverride);
	commonIncludeOverride = relPath;
}

d912pxy_hlsl_generator_memout* d912pxy_hlsl_generator::Process(UINT toMemory)
{
	if (!of && !toMemory)
		return 0;
		
#ifdef _DEBUG
	DumpDisassembly();
#endif

	INT sioTableOffset = LoadSMBlock();
	if (sioTableOffset == -1)
		return 0;

	WriteShaderHeadData();

	do {

		d912pxy_dxbc9::token* tok = code.Current();

		if (tok->iType == d912pxy_dxbc9::token_type::instruction)
			(this->*SIOhandlers[tok->ins.operation + sioTableOffset])(tok);

	} while (code.Next());

	WriteShaderTailData();
	return WriteOutput(toMemory);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstRegister(d912pxy_dxbc9::token_register* reg)
{
	d912pxy_hlsl_generator_regtext ret = { 0 };

	if (reg->target.relative)
		LOG_ERR_THROW2(-1, "hlsl generator got dst relative addresing, which is not done yet");
	else {
		UINT64 wmask = reg->target.dst.getWriteMaskStr(writeMaskOverride);

		sprintf(ret.t, "%s%u%s",
			GetRegTypeStr(reg->regType, 1),
			reg->regNum,
			(char*)&wmask
		);
	}

	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRegister(bool isDst, bool haveDst, UINT idx)
{
	if (isDst)
		return FormatRegister(code.FindDstRegTokenIdx());
	else 
		return FormatRegister(code.FindSrcRegTokenIdx(haveDst, idx));
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRegister(UINT tokOffset)
{
	auto tok = code.SubToken(tokOffset);

	RegEnsureDefined(&tok->reg);

	if (tok->iType == d912pxy_dxbc9::token_type::destination)
	{
		return FormatDstRegister(&tok->reg);
	}
	else {
		return FormatSrcRegister(
			&tok->reg,
			(code.FindDstRegToken()->iType == d912pxy_dxbc9::token_type::destination) ? &code.FindDstRegToken()->reg : nullptr,
			tok->reg.target.relative ? &code.SubToken(tokOffset + 1, d912pxy_dxbc9::token_type::relative)->reg : nullptr
		);
	}
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatSrcRegister(d912pxy_dxbc9::token_register* reg, d912pxy_dxbc9::token_register* dstReg, d912pxy_dxbc9::token_register* adrReg)
{
	UINT64 swizzle = reg->target.src.getSwizzleStr((dstReg && !writeMaskOverride) ? dstReg->target.dst.mask : writeMaskOverride);
		
	d912pxy_hlsl_generator_regtext ret;

	if (reg->target.relative)
	{
		ret = FormatRelativeSrcRegister(reg, adrReg, swizzle);
	}
	else {
		sprintf(ret.t, "%s%u%s",
			GetRegTypeStr(reg->regType, 1),
			reg->regNum,
			(char*)&swizzle
		);
	}

	ret = FormatSrcModifier(ret, &reg->target.src);

	if (dstReg && dstReg->target.dst.allowDstModForSrc)
		ret = FormatDstModifierForSrc(ret, &dstReg->target);

	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRelativeSrcRegister(d912pxy_dxbc9::token_register* reg, d912pxy_dxbc9::token_register* adrReg, UINT64 swizzle)
{
	d912pxy_hlsl_generator_regtext ret = { 0 };

	if (!adrReg)
	{
		LOG_ERR_THROW2(-1, "relative adressing is marked, but no relative adress token found");
		return ret;
	}

	if (reg->regType != D3DSPR_CONST)
		LOG_ERR_THROW2(-1, "relative adressing for registers other then constants are not supported yet");

	UINT64 adrComponent = adrReg->address.getComponent();

	//megai2: base index of relative adressed register is defined in constants
	//so we need to make copy of constants from zero rel. index to max found in code (at least)
	if (RegIsDefined(reg, 0) == 2)
	{
		UINT baseRelNum = reg->regNum;

		if (relLookupDefined != baseRelNum)
		{			
			relLookupDefined = 0;
			++relLookupGroup;
		}

		if (!relLookupDefined)
		{
			UINT relArrSz = 0;
			for (int i = 0; i != 255 - baseRelNum; ++i)
			{
				if (RegIsDefined(reg, i))
					++relArrSz;
				else
					break;//megai2: stop on first fail for now
			}

			HLSL_GEN_WRITE_PROC("float4 reg_consts_rel%u[%u] = { ", relLookupGroup, relArrSz);

			for (int i = 0; i != relArrSz; ++i)
			{
				if (i + 1 == relArrSz)
					HLSL_GEN_WRITE_PROC("	%s%u };", GetRegTypeStr(reg->regType, 1), i + baseRelNum);
				else
					HLSL_GEN_WRITE_PROC("	%s%u,", GetRegTypeStr(reg->regType, 1), i + baseRelNum);
			}

			relLookupDefined = baseRelNum;
		}

		//c23[a0.x].swizzle => getPassedVSFv(23 + a0.x).swizzle
		sprintf(ret.t, "reg_consts_rel%u[%s%u%s]%s",
			relLookupGroup,
			GetRegTypeStr(adrReg->regType, 1),
			adrReg->regNum,
			(char*)&adrComponent,
			(char*)&swizzle
		);
	}
	else {
		char constType[2] = "V";
		if (verToken.isPS)
			constType[0] = 'P';

		//c23[a0.x].swizzle => getPassedVSFv(23 + a0.x).swizzle
		sprintf(ret.t, "getPassed%sSFv(%u+%s%u%s)%s",
			constType,
			reg->regNum,
			GetRegTypeStr(adrReg->regType, 1),
			adrReg->regNum,
			(char*)&adrComponent,
			(char*)&swizzle
		);
	}

	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatSrcModifier(const d912pxy_hlsl_generator_regtext& statement, d912pxy_dxbc9::register_target_source* reg)
{
	d912pxy_hlsl_generator_regtext ret;

	const char* srcModFmt = "%s";

	switch (reg->mod)
	{
	case D3DSPSM_NONE:
		break;
	case D3DSPSM_NEG:
		srcModFmt = "(-%s)";
		break;
	case D3DSPSM_ABS:
		srcModFmt = "abs(%s)";
		break;
	case D3DSPSM_ABSNEG:
		srcModFmt = "(-abs(%s))";
		break;
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX src modifier", reg->mod);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed src modifier");
		break;
	}

	sprintf(ret.t, srcModFmt, statement.t);

	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstModifier(const d912pxy_hlsl_generator_regtext& statement, d912pxy_dxbc9::register_target* dstReg)
{
	d912pxy_hlsl_generator_regtext ret;

	if (!dstReg)
	{
		sprintf(ret.t, "%s", statement.t);
		return ret;
	}

	UINT dstLen = dstReg->dst.dstLength(writeMaskOverride);

	switch (dstReg->dst.mod)
	{
	case D3DSPDM_NONE:
		sprintf(ret.t, "%s", statement.t);
		break;
	case D3DSPDM_SATURATE:
		sprintf_s(ret.t, "saturate(%s)", statement.t);
		break;
	case D3DSPDM_PARTIALPRECISION:
		if (dstLen > 1)
			sprintf_s(ret.t, "(half%u)(%s)", dstLen, statement.t);
		else
			sprintf_s(ret.t, "(half)(%s)", statement.t);
		break;
	case D3DSPDM_PARTIALPRECISION | D3DSPDM_SATURATE:
		if (dstLen > 1)
			sprintf_s(ret.t, "saturate((half%u)(%s))", dstLen, statement.t);
		else
			sprintf_s(ret.t, "saturate((half)(%s))", statement.t);
		break;
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", dstReg->dst.mod);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
		sprintf(ret.t, "failure");
	}
	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstModifierForSrc(const d912pxy_hlsl_generator_regtext& statement, d912pxy_dxbc9::register_target* dstReg)
{
	d912pxy_hlsl_generator_regtext ret;

	switch (dstReg->dst.mod)
	{
	case D3DSPDM_NONE:
	case D3DSPDM_SATURATE:
		sprintf(ret.t, "%s", statement.t);
		break;
	case D3DSPDM_PARTIALPRECISION | D3DSPDM_SATURATE:
	case D3DSPDM_PARTIALPRECISION:
	{
		UINT dLen = dstReg->dst.dstLength(writeMaskOverride);
		if (dLen > 1)
			sprintf_s(ret.t, "((half%u)(%s))", dLen, statement.t);
		else
			sprintf_s(ret.t, "((half)(%s))", statement.t);
	}
		break;	
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", dstReg->dst.mod);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
		sprintf(ret.t, "failure");
	}
	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide1(const char * pre, const char * post, const d912pxy_hlsl_generator_regtext& op1)
{
	d912pxy_hlsl_generator_regtext empty = { 0 };

	return FormatRightSide2(pre, post, "", op1, empty);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide2(const char * pre, const char * post, const char * mid, const d912pxy_hlsl_generator_regtext& op1, const d912pxy_hlsl_generator_regtext& op2)
{
	d912pxy_hlsl_generator_regtext empty = { 0 };

	const char* midA[2]{
		mid,
		""
	};
	
	return FormatRightSide3(pre, post, midA, op1, op2, empty);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide3(const char * pre, const char * post, const char * mid[2], const d912pxy_hlsl_generator_regtext& op1, const d912pxy_hlsl_generator_regtext& op2, d912pxy_hlsl_generator_regtext op3)
{
	d912pxy_hlsl_generator_regtext ret;

	sprintf_s(ret.t, "%s%s%s%s%s%s%s", pre, op1.t, mid[0], op2.t, mid[1], op3.t, post);

	return FormatDstModifier(ret, (code.FindDstRegToken()->iType == d912pxy_dxbc9::token_type::destination) ? &code.FindDstRegToken()->reg.target : nullptr);
}

int d912pxy_hlsl_generator::WriteProcLinePredef(const char * fmt, ...)
{
	va_list args;

	char tb[d912pxy_hlsl_generator_max_line_length];

	PXY_MALLOC(lines[procOffsetPredef], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(tb, fmt, args);
	va_end(args);

	if (procIdent)
		sprintf(lines[procOffsetPredef], "	%s", tb);
	else 
		sprintf(lines[procOffsetPredef], "%s", tb);

	++procOffsetPredef;

	return procOffsetPredef - 1;
}

void d912pxy_hlsl_generator::WriteProcLine(const char * fmt, ...)
{
	va_list args;

	char tb[d912pxy_hlsl_generator_max_line_length];

	PXY_MALLOC(lines[procOffset], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(tb,fmt,args); 
	va_end(args);
	
	sprintf(lines[procOffset], "%*s%s", procIdent*4, "", tb);
	++procOffset;
}

void d912pxy_hlsl_generator::WriteHeadILine(UINT prio, const char * fmt, ...)
{
	va_list args;

	UINT idx = headerOffsetI;

	if (verToken.isPS)
	{
		idx += prio;
	}

	if (lines[idx]) {
		PXY_FREE(lines[idx]);
	}

	PXY_MALLOC(lines[idx], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(lines[idx], fmt, args);
	va_end(args);

	if (!verToken.isPS || prio == 0)
		++headerOffsetI;	
}

void d912pxy_hlsl_generator::WriteHeadOLine(UINT prio, const char * fmt, ...)
{
	va_list args;

	UINT idx = headerOffsetO;

	if (!verToken.isPS)
	{
		idx += prio;
	}

	if (lines[idx]) {
		PXY_FREE(lines[idx]);
	}

	PXY_MALLOC(lines[idx], d912pxy_hlsl_generator_max_line_length, char*);

	va_start(args, fmt);
	vsprintf(lines[idx], fmt, args);
	va_end(args);

	if (verToken.isPS || prio == 0)
		++headerOffsetO;
}

void d912pxy_hlsl_generator::RegEnsureDefined(d912pxy_dxbc9::token_register* reg)
{
	if (!RegIsDefined(reg, 0))
	{
		RegDefine(reg, false, false);
	}
}

void d912pxy_hlsl_generator::RegDefine(d912pxy_dxbc9::token_register* reg, bool asConstant, bool isIOreg)
{
	UINT num = reg->regNum;
	UINT ai = reg->regType * (D3DSPR_PREDICATE + 1) + (num >> 6);
	UINT64 rm = 1ULL << (num & 0x3F);

	regDefined[ai] |= rm;
	regDefinedAsC[ai] |= (rm * asConstant);

	if (isIOreg || asConstant)
		return;

	switch (reg->regType)
	{
	case D3DSPR_CONST:
	{
		HLSL_GEN_WRITE_PROC_PD("float4 %s%u = getPassed%sSFv(%u);",
			GetRegTypeStr(reg->regType, 1),
			num,
			verToken.isPS ? "P" : "V",
			num
		);
		break;
	}
	case D3DSPR_CONSTINT:
	{
		HLSL_GEN_WRITE_PROC_PD("uint4 %s%u = { 0, 0, 0, 0 };",
			GetRegTypeStr(reg->regType, 1),
			num
		);
		break;
	}
	case D3DSPR_COLOROUT:
	{
		if (!num)
		{
			HLSL_GEN_WRITE_HEADO(0, "	float4 %s0: SV_TARGET;",
				GetRegTypeStr(reg->regType, 0)
			);
			HLSL_GEN_WRITE_PROC_PD("#define dx9_ret_color_reg_ac %s0",
				GetRegTypeStr(reg->regType, 1)
			);
		}
		else {
			HLSL_GEN_WRITE_HEADO(0, "	float4 %s%u: SV_Target%u;",
				GetRegTypeStr(reg->regType, 0),
				num,
				num
			);
		}
		break;
	}
	case D3DSPR_TEXCRDOUT:
	{
		if (verToken.major <= 2)
		{
			HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_TEXC, num), "	float4 %s%u: TEXCOORD%u;",
				GetRegTypeStr(reg->regType, 0),
				num,
				num
			);
		}
		break;
	}
	case D3DSPR_RASTOUT:
	{
		if (verToken.major <= 2)
		{
			if (num == 0)
			{
				HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0), "	float4 %s%u: SV_POSITION;",
					GetRegTypeStr(reg->regType, 0),
					num
				);
				HLSL_GEN_WRITE_PROC_PD("#define dx9_halfpixel_pos_reg_ac %s%u",
					GetRegTypeStr(reg->regType, 1),
					num
				);
			}
			else {
				//megai2: i think this is fog & point size registers, that are not routed to PS, so we need to emulate them or just ignore
				HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_VS_NOT_USED, 0), "	float4 %s%u: FIXPIPE_UNIMPL%u;",
					GetRegTypeStr(reg->regType, 0),
					num, num
				);
			}
		}
		break;
	}
	case D3DSPR_DEPTHOUT:
	{
		lines[mainFunctionDeclStrIdx][strlen(lines[mainFunctionDeclStrIdx]) - 1] = 0;
		strcat(lines[mainFunctionDeclStrIdx], ", out float glob_depthOut: SV_Depth)");

		isDepthOutUsed = 1;
		HLSL_GEN_WRITE_PROC_PD("float4 %s%u = { 0, 0, 0, 0 };",
			GetRegTypeStr(reg->regType, 1),
			num
		);
		break;
	}
	case D3DSPR_ATTROUT:
	{
		if (verToken.major <= 2)
		{
			HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_NC, 0), "	float4 %s%u: PS2X_COLOR_OUT%u;",
				GetRegTypeStr(reg->regType, 0),
				num, num
			);
		}
		break;
	}
	default:
		HLSL_GEN_WRITE_PROC_PD("float4 %s%u = { 0, 0, 0, 0 };",
			GetRegTypeStr(reg->regType, 1),
			num
		);
		break;
	}
}

int d912pxy_hlsl_generator::RegIsDefined(d912pxy_dxbc9::token_register* reg, UINT numOffset)
{
	UINT num = reg->regNum + numOffset;
	UINT ai = reg->regType * (D3DSPR_PREDICATE + 1) + (num >> 6);
	UINT64 rm = 1ULL << (num & 0x3F);

	return ((regDefined[ai] & rm) != 0) * (1 + ((regDefinedAsC[ai] & rm) != 0));
}

UINT d912pxy_hlsl_generator::IsNaNGuardEnabled(UINT bit)
{
	return ((NaNguard_flag >> (verToken.isPS * PXY_SDB_HLSL_NAN_GUARD_PS_SHIFT)) & bit) > 0;	
}

void d912pxy_hlsl_generator::ProcSIO_DOTX(d912pxy_dxbc9::token * op, UINT sz)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);

	OverrideWriteMask(((1 << sz) - 1));
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 1);
	OverrideWriteMask(0);

	d912pxy_hlsl_generator_regtext rSide = FormatRightSide2("dot(", ")", ",", sSrc1, sSrc2);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_3OP(d912pxy_dxbc9::token* op, const char * pre, const char * mid[2], const char * post)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 1);
	d912pxy_hlsl_generator_regtext sSrc3 = FormatRegister(0, 1, 2);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide3(pre, post, mid, sSrc1, sSrc2, sSrc3);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_2OP(d912pxy_dxbc9::token* op, const char * pre, const char * mid, const char * post)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);
	d912pxy_hlsl_generator_regtext sSrc2 = FormatRegister(0, 1, 1);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide2(pre, post, mid, sSrc1, sSrc2);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_1OP(d912pxy_dxbc9::token* op, const char * pre, const char * post)
{
	d912pxy_hlsl_generator_regtext sDst = FormatRegister(1, 0, 0);
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(0, 1, 0);
	d912pxy_hlsl_generator_regtext rSide = FormatRightSide1(pre, post, sSrc1);

	HLSL_GEN_WRITE_PROC("%s = %s;",
		sDst.t,
		rSide.t
	);
}

INT d912pxy_hlsl_generator::LoadSMBlock()
{
	verToken = code.Current()->ver;

	switch (verToken.major)
	{
	case 3:
		return d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_3_x;
	case 2:
		return d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_2_x;
	case 1:
		return d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_1_x;
	default:
		LOG_ERR_DTDM("hlsl generator not support %u_%u shader model", verToken.major, verToken.minor);
	}

	return -1;
	
}

void d912pxy_hlsl_generator::DumpDisassembly()
{
//code for asm verification
	const char* copener = "/*\n";
	const char* ccloser = "\n*/\n";
	fwrite(copener, 3, 1, of);
	ID3DXBuffer * dasm = code.getDisassembly();
	if (!dasm)
	{
		LOG_DBG_DTDM3("dbg shader not disassembled");
	} else {
		fwrite(dasm->GetBufferPointer(), 1, dasm->GetBufferSize() - 1, of);
		fwrite(ccloser, 4, 1, of);
		fflush(of);
		dasm->Release();
	}
////////////////////////	
}

d912pxy_hlsl_generator_memout* d912pxy_hlsl_generator::WriteOutput(UINT toMemory)
{
	const char* newLine = "\r\n";
	size_t outputSz = 0;
	intptr_t outputMemPos;

	for (int i = 0; i != d912pxy_hlsl_generator_max_code_lines; ++i)
	{
		if (lines[i] != 0)
			outputSz += strlen(lines[i]) + 2;
	}

	d912pxy_hlsl_generator_memout* ret = NULL;
	PXY_MALLOC(ret, outputSz+4, d912pxy_hlsl_generator_memout*);

	ret->size = (UINT32)outputSz;

	outputMemPos = (intptr_t)(&ret->data[0]);	

	for (int i = 0; i != d912pxy_hlsl_generator_max_code_lines; ++i)
	{
		if (lines[i] != 0)
		{
			size_t linLen = strlen(lines[i]);
			memcpy((void*)outputMemPos, lines[i], linLen);
			outputMemPos += linLen;
			memcpy((void*)outputMemPos, newLine, 2);
			outputMemPos += 2;
			PXY_FREE(lines[i]);
		}
	}

	if (!toMemory)
	{
		fwrite(&ret->data[0], 1, outputSz, of);
		PXY_FREE(ret);
		return 0;
	}
	else {
#ifdef _DEBUG
		fwrite(&ret->data[0], 1, outputSz, of);
#endif

		return ret;
	}
}

void d912pxy_hlsl_generator::DeclareSampler(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token* dclTok, d912pxy_dxbc9::token* dstTok)
{
	const char* samplerType[] = {
		"unk       ",
		"unk       ",
		"tex2d     ",
		"texCube   ",
		"texVolume ",
		"depth     "
	};

	UINT texTypeO = dclTok->dcl.samplerTexType;
	UINT regNum = dstTok->reg.regNum;
	const char* texTypeFmt = "Texture2DArray %s%ut = textureBinds[texState.texture_s%u];";
	const char* regTypeStr = GetRegTypeStr(dstTok->reg.regType, 1);
	UINT samplerIdx = regNum + (verToken.isPS ? 0 : HLSL_GEN_VTEXTURE_OFFSET);

	///this is handled in RCE stage
	//if (genProfile.entryStageSelected(d912pxy_shader_profile::entry::pcf_sampler, samplerIdx))
	//  texTypeO = 5;

	if ((texTypeO == 0) || (texTypeO > 5))
	{
		LOG_ERR_THROW2(-1, "hlsl gen wrong sampler type");
	}	

	if (texTypeO == 3)
		texTypeFmt = "TextureCube %s%ut = textureBindsCubed[texState.texture_s%u];";	

	////

	HLSL_GEN_WRITE_PROC(
		texTypeFmt,
		regTypeStr, regNum, samplerIdx
	);

	HLSL_GEN_WRITE_PROC(
		"sampler %s%us = samplerBinds[texState.sampler_s%u];",
		regTypeStr, regNum, samplerIdx
	);
	HLSL_GEN_WRITE_PROC(
		"#define %s%u_deftype %s",
		regTypeStr, regNum, samplerType[texTypeO]
	);
	HLSL_GEN_WRITE_PROC(
		"#define %s%u_srgb_flag %u",
		regTypeStr, regNum, 1 << samplerIdx
	);
	HLSL_GEN_WRITE_PROC(" ");
}

void d912pxy_hlsl_generator::DeclareMisc(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token* dclTok, d912pxy_dxbc9::token* dstTok)
{
	UINT regNum = dstTok->reg.regNum;
	const char* regNameStr = GetRegTypeStr(dstTok->reg.regType, 0);

	if (regNum == D3DSMO_POSITION)
	{
		HLSL_GEN_WRITE_HEADI(
			HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, regNum),
			"/*default*/    float4 %s%u: SV_POSITION;",
			regNameStr, regNum
		);

		HLSL_GEN_WRITE_PROC_PD("float4 %s%u = inp.%s%u;", regNameStr, regNum, regNameStr, regNum);

		if (verToken.isPS)
		{
			const char* regNameStrProc = GetRegTypeStr(dstTok->reg.regType, 1);

			PSpositionUsed = 1;
			HLSL_GEN_WRITE_PROC("%s%u = %s%u - 0.5f;",
				regNameStrProc,
				regNum,
				regNameStrProc,
				regNum
			);

			HLSL_GEN_WRITE_PROC("#define ps_ros_reg_ac %s%u",
				regNameStrProc,
				regNum
			);
		}
	}
	else if (regNum == D3DSMO_FACE)
	{
		lines[mainFunctionDeclStrIdx][strlen(lines[mainFunctionDeclStrIdx]) - 1] = 0;
		strcat(lines[mainFunctionDeclStrIdx], ", bool glob_isFrontFace: SV_IsFrontFace)");

		HLSL_GEN_WRITE_PROC_PD("float4 glob_vecOne = { 1, 1, 1, 1 };");
		HLSL_GEN_WRITE_PROC_PD("float4 glob_vecMinusOne = { -1, -1, -1, -1 };");

		HLSL_GEN_WRITE_PROC_PD("float4 %s%u = glob_isFrontFace ? glob_vecOne : glob_vecMinusOne;", regNameStr, regNum);
	}
	else
		LOG_ERR_THROW2(-1, "hlsl reg type misc unk");
}

void d912pxy_hlsl_generator::ProcSIO_UNK(d912pxy_dxbc9::token* op)
{
	LOG_DBG_DTDM("unknown opcode %u length %u", op->ins.operation, op->ins.length);
	HLSL_GEN_WRITE_PROC("error //UNK OP %u length %u", op->ins.operation, op->ins.length);

	for (int i = 1; i != op->ins.length + 1; ++i)
	{
		DWORD opVal = code.SubToken(i, d912pxy_dxbc9::token_type::raw)->raw.ui32;
		LOG_DBG_DTDM("op par %u = %08lX", i - 1, opVal);
		HLSL_GEN_WRITE_PROC("error //op par %u = %08lX", i - 1, opVal);
	}
}

void d912pxy_hlsl_generator::WriteShaderHeadData()
{
	HLSL_GEN_WRITE_HEADI(0, "#define dx9_texture_srgb_read(a,b) dx9_texture_srgb_read_proc(a,b)");
	HLSL_GEN_WRITE_HEADI(0, "#define srgb_write_color_lin2s(color) color_lin2s_cond(color, texState.texture_s31 >> 13)");

	if (commonIncludeOverride)
		HLSL_GEN_WRITE_HEADI(0, "#include \"%S\"", commonIncludeOverride);
	else
		HLSL_GEN_WRITE_HEADI(0, "#include \"../common.hlsli\"");
	HLSL_GEN_WRITE_HEADI(0, "	");

	HLSL_GEN_WRITE_PROC("	");
	HLSL_GEN_WRITE_PROC_PD(" ");

	if (verToken.isPS)
	{
		HLSL_GEN_WRITE_HEADI(0, "struct PS_INPUT");
		HLSL_GEN_WRITE_HEADI(0, "{");

		HLSL_GEN_WRITE_HEADO(0, "struct PS_OUTPUT");
		HLSL_GEN_WRITE_HEADO(0, "{");

		mainFunctionDeclStrIdx = HLSL_GEN_WRITE_PROC_PD("PS_OUTPUT main(PS_INPUT inp)");
		HLSL_GEN_WRITE_PROC_PD("{ ");
		++procIdent;
		HLSL_GEN_WRITE_PROC_PD("PS_OUTPUT ret; ");
		HLSL_GEN_WRITE_PROC_PD("");
	}
	else {
		HLSL_GEN_WRITE_HEADI(0, "struct VS_INPUT");
		HLSL_GEN_WRITE_HEADI(0, "{");

		HLSL_GEN_WRITE_HEADO(0, "struct VS_OUTPUT");
		HLSL_GEN_WRITE_HEADO(0, "{");

		HLSL_GEN_WRITE_PROC_PD("vs_clip_plane0_def");

		mainFunctionDeclStrIdx = HLSL_GEN_WRITE_PROC_PD("VS_OUTPUT main(VS_INPUT inp)");


		HLSL_GEN_WRITE_PROC_PD("{");
		++procIdent;
		HLSL_GEN_WRITE_PROC_PD("VS_OUTPUT ret;");
		HLSL_GEN_WRITE_PROC_PD("");
	}
}

void d912pxy_hlsl_generator::WriteShaderTailData()
{
	if (verToken.isPS)
	{
		if (!PSpositionUsed)
		{
			HLSL_GEN_WRITE_HEADI(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0), "/*default*/    float4 unusedPos: SV_POSITION;");
		}
	}

	HLSL_GEN_WRITE_PROC_PD(" ");
	HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_END, 0), "};");
	HLSL_GEN_WRITE_HEADI(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_END, 0), "};");
	HLSL_GEN_WRITE_HEADI(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_END, 1), "	");

	HLSL_GEN_WRITE_PROC("");

	if (isDepthOutUsed)
	{		
		HLSL_GEN_WRITE_PROC("glob_depthOut = reg_depth_out0.x;");
	}

	if (verToken.isPS)
	{
		//		HLSL_GEN_WRITE_PROC("*/");
		if (IsNaNGuardEnabled(PXY_SDB_HLSL_NAN_GUARD_RET))
		{
			HLSL_GEN_WRITE_PROC("dx9_ps_nan_cull_emulation(dx9_ret_color_reg_ac);");
		}

		HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_at_srgb(dx9_ret_color_reg_ac);");
	}
	else {
		HLSL_GEN_WRITE_PROC("dx9_halfpixel_pos_reg_ac = dx9_fix_halfpixel_offset(dx9_halfpixel_pos_reg_ac);");

		if (IsNaNGuardEnabled(PXY_SDB_HLSL_NAN_GUARD_RET))
		{
			HLSL_GEN_WRITE_PROC("dx9_vs_nan_cull_emulation(dx9_halfpixel_pos_reg_ac);");
		}
	}

	HLSL_GEN_WRITE_PROC("");
	HLSL_GEN_WRITE_PROC("return ret;");
	HLSL_GEN_WRITE_PROC("");
	--procIdent;
	HLSL_GEN_WRITE_PROC("}");
}