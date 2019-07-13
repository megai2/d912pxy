////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2019 (c) Baical                                                        /
//                                                                             /
// This library is free software; you can redistribute it and/or               /
// modify it under the terms of the GNU Lesser General Public                  /
// License as published by the Free Software Foundation; either                /
// version 3.0 of the License, or (at your option) any later version.          /
//                                                                             /
// This library is distributed in the hope that it will be useful,             /
// but WITHOUT ANY WARRANTY; without even the implied warranty of              /
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           /
// Lesser General Public License for more details.                             /
//                                                                             /
// You should have received a copy of the GNU Lesser General Public            /
// License along with this library.                                            /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////
#ifndef ISIGNAL_H
#define ISIGNAL_H

enum eCrashCode
{
    eCrashException,
    eCrashPureCall,
    eCrashMemAlloc,
    eCrashInvalidParameter,
    eCrashSignal
};

typedef void (__cdecl *fnCrashHandler)(eCrashCode i_eCode, const void *i_pCrashContext, void *i_pUserContext);

struct stChContext
{
    volatile int      iInstalled;
    volatile int      iProcessed;
    void             *pUserContext;
    fnCrashHandler    pUserHandler;
};

#endif //ISIGNAL_H