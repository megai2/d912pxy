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

const char* d912pxy_hlsl_generator_reg_names[20] = {
	"reg_t", // Temporary Register File
	"reg_inp", // Input Register File
	"reg_const", // Constant Register File
	"reg_at",
	"reg_rastout",
	"reg_attrout",
	"reg_tc_vo",
	"reg_iconst",
	"reg_clr_out",
	"reg_depth_out",
	"reg_sampler",
	"reg_const2",
	"reg_const3",
	"reg_const4",
	"reg_constb",
	"reg_loopc",
	"reg_t16",
	"reg_misc",
	"reg_lab",
	"reg_pred"
};

const char* d912pxy_hlsl_generator_reg_names_proc_ps[20] = {
	"reg_t", // Temporary Register File
	"inp.reg_inp", // Input Register File
	"reg_const", // Constant Register File
	"inp.reg_at",
	"ret.reg_rastout",
	"reg_attrout",
	"ret.reg_tc_vo",
	"reg_iconst",
	"ret.reg_clr_out",
	"reg_depth_out",
	"reg_sampler",
	"reg_const2",
	"reg_const3",
	"reg_const4",
	"reg_constb",
	"reg_loopc",
	"reg_t16",
	"reg_misc",
	"reg_lab",
	"reg_pred"
};

const char* d912pxy_hlsl_generator_reg_names_proc_vs[20] = {
	"reg_t", // Temporary Register File
	"inp.reg_inp", // Input Register File
	"reg_const", // Constant Register File
	"reg_at",
	"ret.reg_rastout",
	"reg_attrout",
	"ret.reg_tc_vo",
	"reg_iconst",
	"ret.reg_clr_out",
	"reg_depth_out",
	"reg_sampler",
	"reg_const2",
	"reg_const3",
	"reg_const4",
	"reg_constb",
	"reg_loopc",
	"reg_t16",
	"reg_misc",
	"reg_lab",
	"reg_pred"
};

d912pxy_hlsl_generator_sio_handler d912pxy_hlsl_generator::SIOhandlers[d912pxy_hlsl_generator_op_handler_group_size*d912pxy_hlsl_generator_op_handler_cnt] = { 0 };

UINT d912pxy_hlsl_generator::allowPP_suffix = 0;
UINT32 d912pxy_hlsl_generator::NaNguard_flag = 0;
UINT32 d912pxy_hlsl_generator::sRGB_alphatest_bits = 0;

d912pxy_hlsl_generator::d912pxy_hlsl_generator(DWORD * src, UINT len, wchar_t * ofn, d912pxy_shader_uid uid) : 
	d912pxy_noncom(L"hlsl generator"),
	genProfile(d912pxy_shader_profile::load(uid)),
	code(src)
{
	PSpositionUsed = 0;
	procIdent = 0;
	mUID = uid;
	
	LoadGenProfile();

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
	WriteExtraUnusedRegs();
	
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
		if (!relLookupDefined)
		{
			UINT relArrSz = 0;
			for (int i = 0; i != 255 - baseRelNum; ++i)
			{
				if (RegIsDefined(reg, 0))
					++relArrSz;
				else
					break;//megai2: stop on first fail for now
			}

			HLSL_GEN_WRITE_PROC("float4 reg_consts_rel[%u] = { ", relArrSz);

			for (int i = 0; i != relArrSz; ++i)
			{
				if (i + 1 == relArrSz)
					HLSL_GEN_WRITE_PROC("	%s%u };", GetRegTypeStr(reg->regType, 1), i + baseRelNum);
				else
					HLSL_GEN_WRITE_PROC("	%s%u,", GetRegTypeStr(reg->regType, 1), i + baseRelNum);
			}

			relLookupDefined = baseRelNum;
		}

		if (relLookupDefined != baseRelNum)
			LOG_ERR_THROW2(-1, "reg_consts_rel go wrong 1");

		//c23[a0.x].swizzle => getPassedVSFv(23 + a0.x).swizzle
		sprintf(ret.t, "reg_consts_rel[%s%u%s]%s",
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

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatSrcModifier(d912pxy_hlsl_generator_regtext statement, d912pxy_dxbc9::register_target_source* reg)
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

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstModifier(d912pxy_hlsl_generator_regtext statement, d912pxy_dxbc9::register_target* dstReg)
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
		sprintf(ret.t, "saturate(%s)", statement.t);
		break;
	case D3DSPDM_PARTIALPRECISION:
		if (dstLen > 1)
			sprintf(ret.t, "(half%u)(%s)", dstLen, statement.t);
		else
			sprintf(ret.t, "(half)(%s)", statement.t);
		break;
	case D3DSPDM_PARTIALPRECISION | D3DSPDM_SATURATE:
		if (dstLen > 1)
			sprintf(ret.t, "saturate((half%u)(%s))", dstLen, statement.t);
		else
			sprintf(ret.t, "saturate((half)(%s))", statement.t);
		break;
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", dstReg->dst.mod);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
		sprintf(ret.t, "failure");
	}
	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatDstModifierForSrc(d912pxy_hlsl_generator_regtext statement, d912pxy_dxbc9::register_target* dstReg)
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
			sprintf(ret.t, "((half%u)(%s))", dLen, statement.t);
		else
			sprintf(ret.t, "((half)(%s))", statement.t);
	}
		break;	
	default:
		LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", dstReg->dst.mod);
		LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
		sprintf(ret.t, "failure");
	}
	return ret;
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide1(const char * pre, const char * post, d912pxy_hlsl_generator_regtext op1)
{
	d912pxy_hlsl_generator_regtext empty = { 0 };

	return FormatRightSide2(pre, post, "", op1, empty);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide2(const char * pre, const char * post, const char * mid, d912pxy_hlsl_generator_regtext op1, d912pxy_hlsl_generator_regtext op2)
{
	d912pxy_hlsl_generator_regtext empty = { 0 };

	const char* midA[2]{
		mid,
		""
	};
	
	return FormatRightSide3(pre, post, midA, op1, op2, empty);
}

d912pxy_hlsl_generator_regtext d912pxy_hlsl_generator::FormatRightSide3(const char * pre, const char * post, const char * mid[2], d912pxy_hlsl_generator_regtext op1, d912pxy_hlsl_generator_regtext op2, d912pxy_hlsl_generator_regtext op3)
{
	d912pxy_hlsl_generator_regtext ret;

	sprintf(ret.t, "%s%s%s%s%s%s%s", pre, op1.t, mid[0], op2.t, mid[1], op3.t, post);

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

const char * d912pxy_hlsl_generator::GetRegTypeStr(DWORD regType, UINT8 proc)
{
	if (proc)
		if (verToken.isPS)
			return d912pxy_hlsl_generator_reg_names_proc_ps[regType];
		else
			return d912pxy_hlsl_generator_reg_names_proc_vs[regType];
	else
		return d912pxy_hlsl_generator_reg_names[regType];
}

const char * d912pxy_hlsl_generator::GetUsageString(UINT usage, UINT type)
{
	if (!type)
	{
		switch (usage)
		{
		case D3DDECLUSAGE_POSITION:
			return "POSN";
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			return "BLWE";
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			return "BLIN"; 
			break;
		case D3DDECLUSAGE_NORMAL:
			return "NORM";
			break;
		case D3DDECLUSAGE_PSIZE:
			return "PSIZ"; 
			break;
		case D3DDECLUSAGE_TEXCOORD:
			return "TEXC"; 
			break;
		case D3DDECLUSAGE_TANGENT:
			return "TANG";
			break;
		case D3DDECLUSAGE_BINORMAL:
			return "BINO";
			break;
		case D3DDECLUSAGE_TESSFACTOR:
			return "TESF"; 
			break;
		case D3DDECLUSAGE_POSITIONT:
			return "POST"; 
			break;
		case D3DDECLUSAGE_COLOR:
			return "COLR"; 
			break;
		case D3DDECLUSAGE_FOG:
			return "FOGG";
			break;
		case D3DDECLUSAGE_DEPTH:
			return "DEPH";
			break;
		case D3DDECLUSAGE_SAMPLE:
			return "SAPL"; 
			break;
		default:
			LOG_ERR_THROW(-1/*vdecl usage unk*/);
		}
	}
	else {
		switch (usage)
		{
		case D3DDECLUSAGE_POSITION:
			return "POSITION";
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			return "BLENDWEIGHT";
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			return "BLENDINDICES";
			break;
		case D3DDECLUSAGE_NORMAL:
			return "NORMAL";
			break;
		case D3DDECLUSAGE_PSIZE:
			return "PSIZE";
			break;
		case D3DDECLUSAGE_TEXCOORD:
			return "TEXCOORD";
			break;
		case D3DDECLUSAGE_TANGENT:
			return "TANGENT";
			break;
		case D3DDECLUSAGE_BINORMAL:
			return "BINORMAL";
			break;
		case D3DDECLUSAGE_TESSFACTOR:
			return "TESSFACTOR";
			break;
		case D3DDECLUSAGE_POSITIONT:
			return "POSITIONT";
			break;
		case D3DDECLUSAGE_COLOR:
			return "COLOR";
			break;
		case D3DDECLUSAGE_FOG:
			return "FOG";
			break;
		case D3DDECLUSAGE_DEPTH:
			return "DEPTH";
			break;
		case D3DDECLUSAGE_SAMPLE:
			return "SAMPLE";
			break;
		default:
			LOG_ERR_THROW(-1/*vdecl usage unk*/);
		}
	}
	return "ERRUSAGE";
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
		HLSL_GEN_WRITE_HEADO(0, "	float4 %s%u: SV_TARGET;",
			GetRegTypeStr(reg->regType, 0),
			num
		);
		HLSL_GEN_WRITE_PROC_PD("#define dx9_ret_color_reg_ac %s%u",
			GetRegTypeStr(reg->regType, 1),
			num
		);
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
				HLSL_GEN_WRITE_HEADO(HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_POS, 0), "	float4 %s%u: FIXPIPE_UNIMPL%u;",
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
		//megai2: drop to default case from here
	}
	default:
		/*if (isDst)
		{
			switch (reg->target.dst.mod)
			{
			case D3DSPDM_NONE:
			case D3DSPDM_SATURATE:
			{
				HLSL_GEN_WRITE_PROC_PD("float4 %s%u = { 0, 0, 0, 0 };",
					GetRegTypeStr(reg->regType, 1),
					num
				);
			}
			break;
			case D3DSPDM_PARTIALPRECISION:
			case D3DSPDM_PARTIALPRECISION | D3DSPDM_SATURATE:
			{
				HLSL_GEN_WRITE_PROC_PD("half4 %s%u = { 0, 0, 0, 0 };",
					GetRegTypeStr(reg->regType, 1),
					num
				);
			}
			break;
			default:
				LOG_ERR_DTDM("hlsl generator not support %08lX dst mod", reg->target.dst.mod);
				LOG_ERR_THROW2(-1, "hlsl generator not support passed dst modifier");
			}
		}
		else {*/
			HLSL_GEN_WRITE_PROC_PD("float4 %s%u = { 0, 0, 0, 0 };",
				GetRegTypeStr(reg->regType, 1),
				num
			);
		/*}
		break;*/
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

void d912pxy_hlsl_generator::LoadGenProfile()
{	
	if ((sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_SRGBR) != 0)
		genProfile.entryEnable(d912pxy_shader_profile::entry::srgb_read);

	if ((sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_ALPHATEST) != 0)
		genProfile.entryEnable(d912pxy_shader_profile::entry::alpha_test);

	if ((sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_FORCE_SRGBW) != 0)
		genProfile.entryEnable(d912pxy_shader_profile::entry::srgb_write);
	
	genProfile.ignoreChanges();
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

void d912pxy_hlsl_generator::ProcSIO_ADD(d912pxy_dxbc9::token* op)
{
	ProcSIO_2OP(op, "", " + ", "");
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

void d912pxy_hlsl_generator::ProcSIO_DEF(d912pxy_dxbc9::token* op)
{
	d912pxy_dxbc9::token_register* dstReg = &code.FindDstRegToken()->reg;
	UINT dstRegOffset = code.FindDstRegTokenIdx()+1;

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
					"/*default*/    float4 %s%u: INPUT_REG%u;",
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

void d912pxy_hlsl_generator::DeclareSampler(d912pxy_dxbc9::token* op, d912pxy_dxbc9::token* dclTok, d912pxy_dxbc9::token* dstTok)
{
	const char* samplerType[] = {
		"unk",
		"unk",
		"tex2d",
		"texCube",
		"texVolume",
		"depth"
	};

	UINT texTypeO = dclTok->dcl.samplerTexType;
	UINT regNum = dstTok->reg.regNum;
	const char* texTypeFmt = "Texture2DArray %s%ut = textureBinds[texState.texture_s%u];";
	const char* regTypeStr = GetRegTypeStr(dstTok->reg.regType, 1);
	UINT samplerIdx = regNum + (verToken.isPS ? 0 : HLSL_GEN_VTEXTURE_OFFSET);

	///

	if (genProfile.entryStageSelected(d912pxy_shader_profile::entry::pcf_sampler, samplerIdx))
		texTypeO = 5;

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

				if ((dclTok->dcl.id > 0) && (!d912pxy_pso_cache::allowRealtimeChecks))
				{
					UINT itr = priority + headerOffsetI;
					do
					{
						--itr;
						--dclTok->dcl.id;

						if (lines[itr] == 0)
						{
							HLSL_GEN_WRITE_HEADI(
								itr - headerOffsetI,
								"/*default*/    float4 %s%u_s%u: %s%u;",
								"unused_ireg_", regNum, dclTok->dcl.id, usageStr, dclTok->dcl.id
							);
						}
					} while (dclTok->dcl.id != 0);
				}
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

//FormatRightSide2(DWORD dstOp, char* pre, char* post, char* mid, d912pxy_hlsl_generator_regtext op1, d912pxy_hlsl_generator_regtext op2)

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
	sprintf(pre.t, "dx9texld(%s_deftype, %s_srgb_flag, ", sSrc2.t, sSrc2.t);

	d912pxy_hlsl_generator_regtext mid;
	sprintf(mid.t, "t, %ss, ", sSrc2.t);
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
	sprintf(pre.t, "dx9texldl(%s_deftype, %s_srgb_flag, ", sSrc2.t, sSrc2.t);

	const char* mid[2] = { 0, ", " };
	d912pxy_hlsl_generator_regtext mid0;
	sprintf(mid0.t, "t, %ss, ", sSrc2.t);
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
	HLSL_GEN_WRITE_PROC("--loopStartCounter%u;", procIdent-1);
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
	d912pxy_hlsl_generator_regtext sSrc1 = FormatRegister(1,0,0);
	
	HLSL_GEN_WRITE_PROC("clip(%s);",
		sSrc1.t
	);
}

void d912pxy_hlsl_generator::ProcSIO_IF(d912pxy_dxbc9::token* op)
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
	ProcSIO_IF(op);
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

	HLSL_GEN_WRITE_PROC("%s = tmp;", sDst.t);

	--procIdent;
	HLSL_GEN_WRITE_PROC("}");	
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
	if (genProfile.entryEnabled(d912pxy_shader_profile::entry::srgb_read))
		HLSL_GEN_WRITE_HEADI(0, "#define dx9_texture_srgb_read(a,b) dx9_texture_srgb_read_proc(a,b)");
	else
		HLSL_GEN_WRITE_HEADI(0, "#define dx9_texture_srgb_read(a,b) ");

	if (sRGB_alphatest_bits & PXY_SDB_HLSL_SRGB_ALPHATEST_COND_SRGBW)
		HLSL_GEN_WRITE_HEADI(0, "#define srgb_write_color_lin2s(color) color_lin2s_cond(color, texState.texture_s31 >> 13)");
	else
		HLSL_GEN_WRITE_HEADI(0, "#define srgb_write_color_lin2s(color) color_lin2s_thru(color)");

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

		if (genProfile.entryEnabled(d912pxy_shader_profile::entry::clipplane0))
		{
			HLSL_GEN_WRITE_PROC_PD("vs_clip_plane0_def");
		}

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

		if (genProfile.entryEnabled(d912pxy_shader_profile::entry::alpha_test))
		{
			if (genProfile.entryEnabled(d912pxy_shader_profile::entry::srgb_write))
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_at_srgb(dx9_ret_color_reg_ac);");
			else
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_at(dx9_ret_color_reg_ac);");
		}
		else {
			if (genProfile.entryEnabled(d912pxy_shader_profile::entry::srgb_write))
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation_srgb(dx9_ret_color_reg_ac);");
			else
				HLSL_GEN_WRITE_PROC("dx9_ps_write_emulation(dx9_ret_color_reg_ac);");
		}

		/*if (genVSClipplane0)
		{
			if (PSpositionUsed)
			{
				HLSL_GEN_WRITE_PROC("dx9_clip_plane_ps(ps_ros_reg_ac, 0);");
			} else {
				HLSL_GEN_WRITE_PROC("dx9_clip_plane_ps(inp.unusedPos, 0);");
			}
		}*/
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

void d912pxy_hlsl_generator::WriteExtraUnusedRegs()
{
	//megai2: defining extra unused regs kills gpu memory bandwith
	if (d912pxy_s.config.GetValueUI32(PXY_CFG_SDB_FORCE_UNUSED_REGS))
	{
		//megai2: force define not used registers to align vs-ps io properly
		if (verToken.isPS)
		{
			UINT dclId = 15;

			UINT itr = (HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[D3DDECLUSAGE_TEXCOORD], dclId) + headerOffsetI);
			do
			{
				--itr;
				--dclId;

				if (lines[itr] == 0)
				{
					HLSL_GEN_WRITE_HEADI(
						itr - headerOffsetI,
						"/*default*/    float4 %s%u_s%u: %s%u;",
						"unused_ireg_", 0, dclId, GetUsageString(D3DDECLUSAGE_TEXCOORD, 1), dclId
					);
				}
			} while (dclId != 0);
		}
		else {
			UINT dclId = 15;

			UINT itr = (HLSL_HIO_PRIORITY(HLSL_HIO_PRIOG_FROM_D3DDECLUSAGE[D3DDECLUSAGE_TEXCOORD], dclId) + headerOffsetO);
			do
			{
				--itr;
				--dclId;

				if (lines[itr] == 0)
				{
					HLSL_GEN_WRITE_HEADO(
						itr - headerOffsetO,
						"	float4 %s%u_s%u: %s%u;",
						"unused_ireg_", 0, dclId, GetUsageString(D3DDECLUSAGE_TEXCOORD, 1), dclId
					);
				}
			} while (dclId != 0);
		}
	}
}

void d912pxy_hlsl_generator::FillHandlers()
{

	for (int i = 0; i != d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_cnt; ++i)
		SIOhandlers[i] = &d912pxy_hlsl_generator::ProcSIO_UNK;

	//megai2: sm 1_0, uses same handles as sm 2 and 3 , cuz i'm lazy and hope it will work
#define __SIOtOF d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_1_x
	SIOhandlers[__SIOtOF + D3DSIO_DEF] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DEFI] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DCL] = &d912pxy_hlsl_generator::ProcSIO_DCL_sm1;
	SIOhandlers[__SIOtOF + D3DSIO_DP3] = &d912pxy_hlsl_generator::ProcSIO_DP3;
	SIOhandlers[__SIOtOF + D3DSIO_DP4] = &d912pxy_hlsl_generator::ProcSIO_DP4;
	SIOhandlers[__SIOtOF + D3DSIO_TEX] = &d912pxy_hlsl_generator::ProcSIO_TEXLD;
	SIOhandlers[__SIOtOF + D3DSIO_MUL] = &d912pxy_hlsl_generator::ProcSIO_MUL;
	SIOhandlers[__SIOtOF + D3DSIO_MAD] = &d912pxy_hlsl_generator::ProcSIO_MAD;
	SIOhandlers[__SIOtOF + D3DSIO_ADD] = &d912pxy_hlsl_generator::ProcSIO_ADD;
	SIOhandlers[__SIOtOF + D3DSIO_MOV] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_MOVA] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_REP] = &d912pxy_hlsl_generator::ProcSIO_REP;
	SIOhandlers[__SIOtOF + D3DSIO_ENDREP] = &d912pxy_hlsl_generator::ProcSIO_ENDREP;
	SIOhandlers[__SIOtOF + D3DSIO_MIN] = &d912pxy_hlsl_generator::ProcSIO_MIN;
	SIOhandlers[__SIOtOF + D3DSIO_MAX] = &d912pxy_hlsl_generator::ProcSIO_MAX;
	SIOhandlers[__SIOtOF + D3DSIO_RCP] = &d912pxy_hlsl_generator::ProcSIO_RCP;
	SIOhandlers[__SIOtOF + D3DSIO_CMP] = &d912pxy_hlsl_generator::ProcSIO_CMP;
	SIOhandlers[__SIOtOF + D3DSIO_DP2ADD] = &d912pxy_hlsl_generator::ProcSIO_DP2ADD;
	SIOhandlers[__SIOtOF + D3DSIO_FRC] = &d912pxy_hlsl_generator::ProcSIO_FRC;
	SIOhandlers[__SIOtOF + D3DSIO_POW] = &d912pxy_hlsl_generator::ProcSIO_POW;
	SIOhandlers[__SIOtOF + D3DSIO_RSQ] = &d912pxy_hlsl_generator::ProcSIO_RSQ;
	SIOhandlers[__SIOtOF + D3DSIO_NRM] = &d912pxy_hlsl_generator::ProcSIO_NRM;
	SIOhandlers[__SIOtOF + D3DSIO_LOG] = &d912pxy_hlsl_generator::ProcSIO_LOG;
	SIOhandlers[__SIOtOF + D3DSIO_EXP] = &d912pxy_hlsl_generator::ProcSIO_EXP;
	SIOhandlers[__SIOtOF + D3DSIO_EXPP] = &d912pxy_hlsl_generator::ProcSIO_EXPP;
	SIOhandlers[__SIOtOF + D3DSIO_TEXKILL] = &d912pxy_hlsl_generator::ProcSIO_TEXKILL;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_ELSE] = &d912pxy_hlsl_generator::ProcSIO_ELSE;
	SIOhandlers[__SIOtOF + D3DSIO_ENDIF] = &d912pxy_hlsl_generator::ProcSIO_ENDIF;
	SIOhandlers[__SIOtOF + D3DSIO_BREAKC] = &d912pxy_hlsl_generator::ProcSIO_BREAK;
	SIOhandlers[__SIOtOF + D3DSIO_TEXLDL] = &d912pxy_hlsl_generator::ProcSIO_TEXLDL;
	SIOhandlers[__SIOtOF + D3DSIO_LRP] = &d912pxy_hlsl_generator::ProcSIO_LRP;
	SIOhandlers[__SIOtOF + D3DSIO_SLT] = &d912pxy_hlsl_generator::ProcSIO_SLT;
	SIOhandlers[__SIOtOF + D3DSIO_ABS] = &d912pxy_hlsl_generator::ProcSIO_ABS;
	SIOhandlers[__SIOtOF + D3DSIO_SGE] = &d912pxy_hlsl_generator::ProcSIO_SGE;
	SIOhandlers[__SIOtOF + D3DSIO_SGN] = &d912pxy_hlsl_generator::ProcSIO_SGN;
	SIOhandlers[__SIOtOF + D3DSIO_SINCOS] = &d912pxy_hlsl_generator::ProcSIO_SINCOS;
#undef __SIOtOF

	//sm 2_0
#define __SIOtOF d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_2_x
	SIOhandlers[__SIOtOF + D3DSIO_DEF] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DEFI] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DCL] = &d912pxy_hlsl_generator::ProcSIO_DCL_sm2;
	SIOhandlers[__SIOtOF + D3DSIO_DP3] = &d912pxy_hlsl_generator::ProcSIO_DP3;
	SIOhandlers[__SIOtOF + D3DSIO_DP4] = &d912pxy_hlsl_generator::ProcSIO_DP4;
	SIOhandlers[__SIOtOF + D3DSIO_TEX] = &d912pxy_hlsl_generator::ProcSIO_TEXLD;
	SIOhandlers[__SIOtOF + D3DSIO_MUL] = &d912pxy_hlsl_generator::ProcSIO_MUL;
	SIOhandlers[__SIOtOF + D3DSIO_MAD] = &d912pxy_hlsl_generator::ProcSIO_MAD;
	SIOhandlers[__SIOtOF + D3DSIO_ADD] = &d912pxy_hlsl_generator::ProcSIO_ADD;
	SIOhandlers[__SIOtOF + D3DSIO_MOV] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_MOVA] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_REP] = &d912pxy_hlsl_generator::ProcSIO_REP;
	SIOhandlers[__SIOtOF + D3DSIO_ENDREP] = &d912pxy_hlsl_generator::ProcSIO_ENDREP;
	SIOhandlers[__SIOtOF + D3DSIO_MIN] = &d912pxy_hlsl_generator::ProcSIO_MIN;
	SIOhandlers[__SIOtOF + D3DSIO_MAX] = &d912pxy_hlsl_generator::ProcSIO_MAX;
	SIOhandlers[__SIOtOF + D3DSIO_RCP] = &d912pxy_hlsl_generator::ProcSIO_RCP;
	SIOhandlers[__SIOtOF + D3DSIO_CMP] = &d912pxy_hlsl_generator::ProcSIO_CMP;
	SIOhandlers[__SIOtOF + D3DSIO_DP2ADD] = &d912pxy_hlsl_generator::ProcSIO_DP2ADD;
	SIOhandlers[__SIOtOF + D3DSIO_FRC] = &d912pxy_hlsl_generator::ProcSIO_FRC;
	SIOhandlers[__SIOtOF + D3DSIO_POW] = &d912pxy_hlsl_generator::ProcSIO_POW;
	SIOhandlers[__SIOtOF + D3DSIO_RSQ] = &d912pxy_hlsl_generator::ProcSIO_RSQ;
	SIOhandlers[__SIOtOF + D3DSIO_NRM] = &d912pxy_hlsl_generator::ProcSIO_NRM;
	SIOhandlers[__SIOtOF + D3DSIO_LOG] = &d912pxy_hlsl_generator::ProcSIO_LOG;
	SIOhandlers[__SIOtOF + D3DSIO_EXP] = &d912pxy_hlsl_generator::ProcSIO_EXP;
	SIOhandlers[__SIOtOF + D3DSIO_EXPP] = &d912pxy_hlsl_generator::ProcSIO_EXPP;
	SIOhandlers[__SIOtOF + D3DSIO_TEXKILL] = &d912pxy_hlsl_generator::ProcSIO_TEXKILL;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_ELSE] = &d912pxy_hlsl_generator::ProcSIO_ELSE;
	SIOhandlers[__SIOtOF + D3DSIO_ENDIF] = &d912pxy_hlsl_generator::ProcSIO_ENDIF;
	SIOhandlers[__SIOtOF + D3DSIO_BREAKC] = &d912pxy_hlsl_generator::ProcSIO_BREAK;
	SIOhandlers[__SIOtOF + D3DSIO_TEXLDL] = &d912pxy_hlsl_generator::ProcSIO_TEXLDL;
	SIOhandlers[__SIOtOF + D3DSIO_LRP] = &d912pxy_hlsl_generator::ProcSIO_LRP;
	SIOhandlers[__SIOtOF + D3DSIO_SLT] = &d912pxy_hlsl_generator::ProcSIO_SLT;
	SIOhandlers[__SIOtOF + D3DSIO_ABS] = &d912pxy_hlsl_generator::ProcSIO_ABS;
	SIOhandlers[__SIOtOF + D3DSIO_SGE] = &d912pxy_hlsl_generator::ProcSIO_SGE;
	SIOhandlers[__SIOtOF + D3DSIO_SGN] = &d912pxy_hlsl_generator::ProcSIO_SGN;
	SIOhandlers[__SIOtOF + D3DSIO_SINCOS] = &d912pxy_hlsl_generator::ProcSIO_SINCOS;

#undef __SIOtOF

	//sm 3_0
#define __SIOtOF d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_3_x
	SIOhandlers[__SIOtOF + D3DSIO_DEF] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DEFI] = &d912pxy_hlsl_generator::ProcSIO_DEF;
	SIOhandlers[__SIOtOF + D3DSIO_DCL] = &d912pxy_hlsl_generator::ProcSIO_DCL_sm3;
	SIOhandlers[__SIOtOF + D3DSIO_DP3] = &d912pxy_hlsl_generator::ProcSIO_DP3;
	SIOhandlers[__SIOtOF + D3DSIO_DP4] = &d912pxy_hlsl_generator::ProcSIO_DP4;
	SIOhandlers[__SIOtOF + D3DSIO_TEX] = &d912pxy_hlsl_generator::ProcSIO_TEXLD;
	SIOhandlers[__SIOtOF + D3DSIO_MUL] = &d912pxy_hlsl_generator::ProcSIO_MUL;
	SIOhandlers[__SIOtOF + D3DSIO_MAD] = &d912pxy_hlsl_generator::ProcSIO_MAD;
	SIOhandlers[__SIOtOF + D3DSIO_ADD] = &d912pxy_hlsl_generator::ProcSIO_ADD;
	SIOhandlers[__SIOtOF + D3DSIO_MOV] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_MOVA] = &d912pxy_hlsl_generator::ProcSIO_MOV;
	SIOhandlers[__SIOtOF + D3DSIO_REP] = &d912pxy_hlsl_generator::ProcSIO_REP;
	SIOhandlers[__SIOtOF + D3DSIO_ENDREP] = &d912pxy_hlsl_generator::ProcSIO_ENDREP;
	SIOhandlers[__SIOtOF + D3DSIO_MIN] = &d912pxy_hlsl_generator::ProcSIO_MIN;
	SIOhandlers[__SIOtOF + D3DSIO_MAX] = &d912pxy_hlsl_generator::ProcSIO_MAX;
	SIOhandlers[__SIOtOF + D3DSIO_RCP] = &d912pxy_hlsl_generator::ProcSIO_RCP;
	SIOhandlers[__SIOtOF + D3DSIO_CMP] = &d912pxy_hlsl_generator::ProcSIO_CMP;
	SIOhandlers[__SIOtOF + D3DSIO_DP2ADD] = &d912pxy_hlsl_generator::ProcSIO_DP2ADD;
	SIOhandlers[__SIOtOF + D3DSIO_FRC] = &d912pxy_hlsl_generator::ProcSIO_FRC;
	SIOhandlers[__SIOtOF + D3DSIO_POW] = &d912pxy_hlsl_generator::ProcSIO_POW;
	SIOhandlers[__SIOtOF + D3DSIO_RSQ] = &d912pxy_hlsl_generator::ProcSIO_RSQ;
	SIOhandlers[__SIOtOF + D3DSIO_NRM] = &d912pxy_hlsl_generator::ProcSIO_NRM;
	SIOhandlers[__SIOtOF + D3DSIO_LOG] = &d912pxy_hlsl_generator::ProcSIO_LOG;
	SIOhandlers[__SIOtOF + D3DSIO_EXP] = &d912pxy_hlsl_generator::ProcSIO_EXP;
	SIOhandlers[__SIOtOF + D3DSIO_EXPP] = &d912pxy_hlsl_generator::ProcSIO_EXPP;
	SIOhandlers[__SIOtOF + D3DSIO_TEXKILL] = &d912pxy_hlsl_generator::ProcSIO_TEXKILL;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_ELSE] = &d912pxy_hlsl_generator::ProcSIO_ELSE;
	SIOhandlers[__SIOtOF + D3DSIO_ENDIF] = &d912pxy_hlsl_generator::ProcSIO_ENDIF;
	SIOhandlers[__SIOtOF + D3DSIO_BREAKC] = &d912pxy_hlsl_generator::ProcSIO_BREAK;
	SIOhandlers[__SIOtOF + D3DSIO_TEXLDL] = &d912pxy_hlsl_generator::ProcSIO_TEXLDL;
	SIOhandlers[__SIOtOF + D3DSIO_LRP] = &d912pxy_hlsl_generator::ProcSIO_LRP;
	SIOhandlers[__SIOtOF + D3DSIO_SLT] = &d912pxy_hlsl_generator::ProcSIO_SLT;
	SIOhandlers[__SIOtOF + D3DSIO_ABS] = &d912pxy_hlsl_generator::ProcSIO_ABS;
	SIOhandlers[__SIOtOF + D3DSIO_SGE] = &d912pxy_hlsl_generator::ProcSIO_SGE;
	SIOhandlers[__SIOtOF + D3DSIO_SGN] = &d912pxy_hlsl_generator::ProcSIO_SGN;
	SIOhandlers[__SIOtOF + D3DSIO_SINCOS] = &d912pxy_hlsl_generator::ProcSIO_SINCOS;
#undef __SIOtOF
}
