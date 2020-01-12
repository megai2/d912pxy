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

d912pxy_dxbc9::d912pxy_dxbc9(DWORD* code)
{
	streamBase = code;
	Reset();
}

d912pxy_dxbc9::~d912pxy_dxbc9()
{

}

d912pxy_dxbc9::token* d912pxy_dxbc9::Current()
{
	return &i_current;
}

bool d912pxy_dxbc9::Next()
{
	UINT adv = i_current.length(sm1);

	if (!adv)
		return false;

	subTokensLoadedFlags = 0;
	stream += adv;
	return i_current.load(stream[0], token_type::unk);
}

d912pxy_dxbc9::token* d912pxy_dxbc9::SubToken(UINT offset, token_type asType)
{
	if ((subTokensLoadedFlags & (1ULL << offset)) == 0)
	{
		if (subTokens[offset].load(stream[offset], asType))
			subTokensLoadedFlags |= (1ULL << offset);
		else
			return nullptr;
	}

	return SubToken(offset);
}

d912pxy_dxbc9::token* d912pxy_dxbc9::SubToken(UINT offset)
{
	return &subTokens[offset];
}

d912pxy_dxbc9::token* d912pxy_dxbc9::FindDstRegToken()
{
	return SubToken(DstRegSubTokenIdx(), token_type::destination);
}

UINT d912pxy_dxbc9::FindDstRegTokenIdx()
{
	SubToken(DstRegSubTokenIdx(), token_type::destination);
	return DstRegSubTokenIdx();
}

UINT d912pxy_dxbc9::FindSrcRegTokenIdx(bool haveDst, UINT idx)
{
	UINT ret = SrcRegSubTokenIdxBase();

	if (haveDst)
	{
		ret = DstRegSubTokenIdx();
		ret += SubToken(ret, token_type::destination)->reg.target.relative ? 2 : 1;
	}

	for (int i = 0; i!=idx;++i)
	{
		ret += SubToken(ret, token_type::source)->reg.target.relative ? 2 : 1;
	}

	SubToken(ret, token_type::source);

	return ret;
}

void d912pxy_dxbc9::Reset()
{
	stream = streamBase;
	subTokensLoadedFlags = 0;
	i_current.load(stream[0], token_type::unk);

	sm1 = i_current.ver.major < 2;
}

ID3DXBuffer* d912pxy_dxbc9::getDisassembly()
{
	ID3DXBuffer* dasm;
	if (FAILED(d912pxy_d3dx9_DisassembleShader(streamBase, &dasm)))
		return nullptr;
	
	return dasm;
}

bool d912pxy_dxbc9::token::load(DWORD code, token_type type)
{
	iType = type;
identified:
	switch (iType)
	{
		case token_type::unk:
		{
			if (com.test(code))
				iType = token_type::comment;				
			else if (end.test(code))
				iType = token_type::end;
			else if (ver.test(code))
				iType = token_type::version;
			else 
				iType = token_type::instruction;

			goto identified;
		}
		break;
		case token_type::comment:
			return com.load(code);
		case token_type::destination:
		case token_type::label:
		case token_type::source:
		case token_type::relative:
			return reg.load(code, iType);
		case token_type::end:
			return end.load(code);
		case token_type::instruction:
			return ins.load(code);
		case token_type::raw:
			return raw.load(code);
		case token_type::version:
			return ver.load(code);
		case token_type::dcl:
			return dcl.load(code);
		default:
			return false;
			break;
	}
	return false;
}

static const UINT SM_1_X_SIO_SIZE[] = {
	0, 	2,	3,	0,	4,	3,	2,	2,	3,	3,	3,	3,	3,	3,	2,	2,	0,	0,	4,	2,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,	0,	2,	3,	0,	0,	2,	2,	2,	1,	0,	0,	2,	0,	0,
	0,	2,	2,	0,	5,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
	3,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	2,	0,	0,	5,	0,	0,	0,	0,	0,	0,
	4,	0,	4,	0,	0,	0,	0,	3,	0
};

UINT d912pxy_dxbc9::token::length(bool noOpLen)
{
	switch (iType)
	{
	case token_type::comment:
		return 1 + com.length;
	case token_type::version:
		return 1;
	case token_type::instruction:
		if (noOpLen)
		{			
			//use lookup table instead of bit 31 forward scan
			return 1 + SM_1_X_SIO_SIZE[ins.operation];		
		} else 
			return 1 + ins.length;
	default:
		return 0;
	}
}

DWORD d912pxy_dxbc9::token::bitFieldExtract(DWORD val, const DWORD rshift, const DWORD mask)
{
	return (val & mask) >> rshift;
}

DWORD d912pxy_dxbc9::token_instruction::getOperation(DWORD code)
{
	return (D3DSI_OPCODE_MASK & code);
}

bool d912pxy_dxbc9::token_instruction::load(DWORD code)
{
	operation = getOperation(code);
	control = token::bitFieldExtract(code, D3DSP_OPCODESPECIFICCONTROL_SHIFT, D3DSP_OPCODESPECIFICCONTROL_MASK);
	length = token::bitFieldExtract(code, D3DSI_INSTLENGTH_SHIFT, D3DSI_INSTLENGTH_MASK);
	predication = (code & D3DSHADER_INSTRUCTION_PREDICATED) != 0;
	coissue = (code & D3DSI_COISSUE) != 0;

	return true;
}

UINT64 d912pxy_dxbc9::token_instruction::formatCmpString()
{
	UINT64 retV = 0;

	char* ret = (char*)&retV;

	/*D3DSPC_GT = 1, // 0 0 1
	D3DSPC_EQ = 2, // 0 1 0
	D3DSPC_GE = 3, // 0 1 1
	D3DSPC_LT = 4, // 1 0 0
	D3DSPC_NE = 5, // 1 0 1
	D3DSPC_LE = 6, // 1 1 0*/

	switch (control & 0x7)
	{
	case 1://GT
		strcpy(ret, " > ");
		break;
	case 2://EQ
		strcpy(ret, " == ");
		break;
	case 3://GE
		strcpy(ret, " >= ");
		break;
	case 4://LT
		strcpy(ret, " < ");
		break;
	case 5://NE
		strcpy(ret, " != ");
		break;
	case 6://LE
		strcpy(ret, " <= ");
		break;
	}

	return retV;
}

bool d912pxy_dxbc9::token_comment::test(DWORD code)
{
	return 
		(token_instruction::getOperation(code) == D3DSIO_COMMENT) &&
		((code & (1 << 31)) == 0);
}

bool d912pxy_dxbc9::token_comment::load(DWORD code)
{	
	length = token::bitFieldExtract(code, D3DSI_COMMENTSIZE_SHIFT, D3DSI_COMMENTSIZE_MASK);
	return true;
}

bool d912pxy_dxbc9::token_end::test(DWORD code)
{
	static_assert(D3DVS_END() == D3DPS_END(), "wrong end marker test code, fix it!");
	return code == D3DVS_END();
}

bool d912pxy_dxbc9::token_end::load(DWORD code)
{
	return true;
}

bool d912pxy_dxbc9::token_version::test(DWORD code)
{
	return (code & (0xFFFE << 16)) == 0xFFFE0000;
}

bool d912pxy_dxbc9::token_version::load(DWORD code)
{
	major = D3DSHADER_VERSION_MAJOR(code);
	minor = D3DSHADER_VERSION_MINOR(code);

	isPS = (code & (1 << 16));

	return true;
}

UINT64 d912pxy_dxbc9::token_register::getSwizzleStr(DWORD swizzle, DWORD writeMask)
{
	UINT64 retV;
	char* ret = (char*)&retV;

	char swCh[4] = {
		(char)((swizzle >> 0) & 0x3),//x
		(char)((swizzle >> 2) & 0x3),//y
		(char)((swizzle >> 4) & 0x3),//z
		(char)((swizzle >> 6) & 0x3)//w
	};

	ret[0] = '.';
	int wp = 1;

	for (int i = 0; i != 4; ++i)
	{
		if (!((1 << i) & writeMask))
			continue;

		switch (swCh[i])
		{
		case 0: //x
			swCh[i] = 'x';
			break;
		case 1: //y
			swCh[i] = 'y';
			break;
		case 2: //z
			swCh[i] = 'z';
			break;
		case 3: //w
			swCh[i] = 'w';
			break;
		}

		ret[wp] = swCh[i];
		++wp;
	}
	ret[wp] = 0;

	//megai2: means ".xyzw", so we can skip this safely
	if ((retV & 0x000000777a79782e) == 0x000000777a79782e)
	{
		return 0;
	}

	return retV;
}

bool d912pxy_dxbc9::token_register::load(DWORD code, token_type type)
{
    regNum = code & D3DSP_REGNUM_MASK;
	regType = token::bitFieldExtract(code, D3DSP_REGTYPE_SHIFT, D3DSP_REGTYPE_MASK) | token::bitFieldExtract(code, D3DSP_REGTYPE_SHIFT2, D3DSP_REGTYPE_MASK2);

	switch (type)
	{
	case token_type::destination:
	case token_type::source:
		return target.load(code, type);
	case token_type::relative:
		return address.load(code);
	case token_type::label:	
		return true;
	default:
		return false;
	}

	return true;
}

bool d912pxy_dxbc9::register_target::load(DWORD code, token_type type)
{
	relative = (code & D3DSHADER_ADDRESSMODE_MASK) != 0;

	if (type == token_type::destination)
		return dst.load(code);
	else
		return src.load(code);
}

bool d912pxy_dxbc9::token_raw::load(DWORD code)
{
	ui32 = code;

	return true;
}

UINT64 d912pxy_dxbc9::register_address::getComponent()
{
	return token_register::getSwizzleStr(component, 1);
}

bool d912pxy_dxbc9::register_address::load(DWORD code)
{
	component = token::bitFieldExtract(code, D3DSP_SWIZZLE_SHIFT, D3DSP_SWIZZLE_MASK);

	return true;
}

bool d912pxy_dxbc9::register_target_destination::load(DWORD code)
{
	mask = token::bitFieldExtract(code, 16, D3DSP_WRITEMASK_ALL);
	mod = token::bitFieldExtract(code, 0, D3DSP_DSTMOD_MASK);
	shift = token::bitFieldExtract(code, D3DSP_DSTSHIFT_SHIFT, D3DSP_DSTSHIFT_MASK);
	allowDstModForSrc = true;

	if (!d912pxy_hlsl_generator::allowPP_suffix)
		mod &= ~D3DSPDM_PARTIALPRECISION;

	return true;
}

UINT64 d912pxy_dxbc9::register_target_destination::getWriteMaskStr(DWORD externMask)
{
	UINT64 retV;
	char* ret = (char*)&retV;

	ret[0] = '.';
	int wp = 1;

	DWORD lmask = externMask ? externMask : mask;

	if (lmask == 0xF)
		return 0;

	char maskCh[4] = { 'x', 'y', 'z', 'w' };

	for (int i = 0; i != 4; ++i)
	{
		if ((1 << i) & lmask)
		{
			ret[wp] = maskCh[i];
			++wp;
		}
	}
	ret[wp] = 0;

	return retV;
}

UINT d912pxy_dxbc9::register_target_destination::dstLength(DWORD externMask)
{
	UINT ret = 0;
	DWORD lmask = externMask ? externMask : mask;

	for (int i = 0; i != 4; ++i)
	{
		if (lmask & (1 << i))
		{
			++ret;
		}
	}
	return ret;
}

bool d912pxy_dxbc9::register_target_source::load(DWORD code)
{
	mod = token::bitFieldExtract(code, 0, D3DSP_SRCMOD_MASK);
	swizzle = token::bitFieldExtract(code, D3DSP_SWIZZLE_SHIFT, D3DSP_SWIZZLE_MASK);

	return true;
}

UINT64 d912pxy_dxbc9::register_target_source::getSwizzleStr(DWORD writeMask)
{
	return token_register::getSwizzleStr(swizzle, writeMask);
}

bool d912pxy_dxbc9::token_dcl::load(DWORD code)
{
	samplerTexType = token::bitFieldExtract(code, D3DSP_TEXTURETYPE_SHIFT, D3DSP_TEXTURETYPE_MASK);
	id = token::bitFieldExtract(code, D3DSP_DCL_USAGEINDEX_SHIFT, D3DSP_DCL_USAGEINDEX_MASK);
	usage = token::bitFieldExtract(code, D3DSP_DCL_USAGE_SHIFT, D3DSP_DCL_USAGE_MASK);//  code & 0x1F;

	return true;
}
