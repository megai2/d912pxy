#include "stdafx.h"
#include "../thirdparty/StackWalker/StackWalker.h"

class d912pxy_StackWalker : public StackWalker, public d912pxy_noncom
{
public:
	d912pxy_StackWalker();
	~d912pxy_StackWalker();

	void OnOutput(LPCSTR szText);
};