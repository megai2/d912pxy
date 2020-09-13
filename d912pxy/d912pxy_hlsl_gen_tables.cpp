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
	"inp.reg_attrout",
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
	"ret.reg_attrout",
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

const char* d912pxy_hlsl_generator::GetRegTypeStr(DWORD regType, UINT8 proc)
{
	if (proc)
		if (verToken.isPS)
			return d912pxy_hlsl_generator_reg_names_proc_ps[regType];
		else
			return d912pxy_hlsl_generator_reg_names_proc_vs[regType];
	else
		return d912pxy_hlsl_generator_reg_names[regType];
}

const char* d912pxy_hlsl_generator::GetUsageString(UINT usage, UINT type)
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

d912pxy_hlsl_generator_sio_handler d912pxy_hlsl_generator::SIOhandlers[d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_cnt] = { 0 };

void d912pxy_hlsl_generator::FillHandlers()
{

	for (int i = 0; i != d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_cnt; ++i)
		SIOhandlers[i] = &d912pxy_hlsl_generator::ProcSIO_UNK;

	//megai2: sm 1_0, uses same handles as sm 2 and 3 , cuz i'm lazy and hope it will work
#define __SIOtOF (d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_1_x)
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
	SIOhandlers[__SIOtOF + D3DSIO_IF] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IFC;
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
#define __SIOtOF (d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_2_x)
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
	SIOhandlers[__SIOtOF + D3DSIO_IF] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IFC;
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
	SIOhandlers[__SIOtOF + D3DSIO_DSX] = &d912pxy_hlsl_generator::ProcSIO_DSX;
	SIOhandlers[__SIOtOF + D3DSIO_DSY] = &d912pxy_hlsl_generator::ProcSIO_DSY;
#undef __SIOtOF

	//sm 3_0
#define __SIOtOF (d912pxy_hlsl_generator_op_handler_group_size * d912pxy_hlsl_generator_op_handler_3_x)
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
	SIOhandlers[__SIOtOF + D3DSIO_IF] = &d912pxy_hlsl_generator::ProcSIO_IF;
	SIOhandlers[__SIOtOF + D3DSIO_IFC] = &d912pxy_hlsl_generator::ProcSIO_IFC;
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
	SIOhandlers[__SIOtOF + D3DSIO_DSX] = &d912pxy_hlsl_generator::ProcSIO_DSX;
	SIOhandlers[__SIOtOF + D3DSIO_DSY] = &d912pxy_hlsl_generator::ProcSIO_DSY;
#undef __SIOtOF
}
