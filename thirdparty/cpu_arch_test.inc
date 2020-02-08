#include <intrin.h>

struct cpu_arch {
	bool SSE;
	bool AVX;
	bool AVX2;
};

cpu_arch GetCPUArch()
{
	cpu_arch ret = {0};
	
	int info[4];
	__cpuid(info, 0);
	int nIds = info[0];
	
	__cpuid(info, 0x80000000);
	unsigned nExIds = info[0];

	if (nIds >= 0x00000001)
	{
		__cpuid(info,0x00000001);
		ret.SSE    = (info[3] & ((int)1 << 25)) != 0;
		ret.AVX    = (info[2] & ((int)1 << 28)) != 0;
	}
	
	if (nIds >= 0x00000007)
	{
		__cpuid(info,0x00000007);
		ret.AVX2   = (info[1] & ((int)1 <<  5)) != 0;	
	}	

	return ret;
}

