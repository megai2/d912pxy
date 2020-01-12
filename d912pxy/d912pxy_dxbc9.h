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
#pragma once
#include "stdafx.h"

class d912pxy_dxbc9
{
public:
	enum class token_type : UINT {
		comment,
		destination,
		end,
		instruction,
		label,
		source,
		version,
		raw,
		relative,
		dcl,
		unk
	};

	struct token_comment {
		DWORD length;
		
		static bool test(DWORD code);

		bool load(DWORD code);
	};

	struct token_end {
		DWORD indicator;

		static bool test(DWORD code);

		bool load(DWORD code);
	};

	struct token_instruction {
		DWORD operation;
		DWORD control;
		DWORD length;
		DWORD predication;
		DWORD coissue;

		static DWORD getOperation(DWORD code);

		bool load(DWORD code);

		UINT64 formatCmpString();
	};

	struct register_label {
		DWORD zero;
	};

	struct register_target_destination {
		DWORD mask;
		DWORD mod;
		DWORD shift;
		bool allowDstModForSrc;

		bool load(DWORD code);
				
		UINT64 getWriteMaskStr(DWORD externMask);
		UINT dstLength(DWORD externMask);
	};

	struct register_target_source {
		DWORD swizzle;
		DWORD mod;

		bool load(DWORD code);

		UINT64 getSwizzleStr(DWORD writeMask);
	};

	struct register_address {
		DWORD unused1;
		DWORD component;
		DWORD unused2;

		UINT64 getComponent();

		bool load(DWORD code);
	};

	struct register_target {
		DWORD relative;
		union {
			register_target_destination dst;
			register_target_source src;
		};

		bool load(DWORD code, token_type type);
	};

	struct token_register {
		DWORD regNum;
		DWORD regType;
		union {
			register_address address;
			register_label label;
			register_target target;
		};

		static UINT64 getSwizzleStr(DWORD swizzle, DWORD writeMask);

		bool load(DWORD code, token_type type);
	};

	struct token_version {
		DWORD minor;
		DWORD major;
		bool isPS;

		static bool test(DWORD code);

		bool load(DWORD code);
	};

	union token_raw {
		DWORD ui32;
		INT32 i32;
		float f;

		bool load(DWORD code);
	};

	struct token_dcl {
		DWORD samplerTexType;
		UINT id;
		UINT usage;

		bool load(DWORD code);
	};

	struct token {
		union {
			token_version ver;
			token_register reg;
			token_instruction ins;
			token_end end;
			token_comment com;
			token_raw raw;
			token_dcl dcl;
		};

		token_type iType;

		bool load(DWORD code, token_type type);

		UINT length(bool noOpLen);

		static DWORD bitFieldExtract(DWORD val, const DWORD rshift, const DWORD mask);
	};

public:
	d912pxy_dxbc9(DWORD* code);
	~d912pxy_dxbc9();

	token* Current();
	bool Next();
	 
	token* SubToken(UINT offset, token_type asType);
	token* SubToken(UINT offset);

	token* FindDstRegToken();

	UINT FindDstRegTokenIdx();
	UINT FindSrcRegTokenIdx(bool haveDst, UINT idx);

	void Reset();

	ID3DXBuffer* getDisassembly();

private:
	constexpr UINT DstRegSubTokenIdx() { return 1; };
	constexpr UINT SrcRegSubTokenIdxBase() { return 1; };

	static const UINT maxSubtokensForOp = 10;

	DWORD* streamBase;
	DWORD* stream;
	token i_current;

	token subTokens[maxSubtokensForOp];
	UINT64 subTokensLoadedFlags;
	

	bool sm1;
};

