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
#ifndef IPSHARED_LIB_H
#define IPSHARED_LIB_H

#include "GTypes.h"

#include "Length.h"
#include "PString.h"
#include "WString.h"
#include "SharedLib.h"
#include "PAtomic.h"

class CPSharedLib
    : public CSharedLib
{
protected:
    HMODULE m_pDll;

public:
    CPSharedLib(const tXCHAR *i_pPath)
        : CSharedLib(i_pPath)
        , m_pDll(NULL)
    {
        m_pDll = LoadLibraryW(i_pPath);

        if (m_pDll)
        {
            m_bState = TRUE;
        }
    }

    void *GetFunction(const tACHAR *i_pName)
    {
        return (void*)(GetProcAddress(m_pDll, i_pName));
    }

    virtual ~CPSharedLib()
    {
        if (m_pDll)
        {
            FreeLibrary(m_pDll);
            m_pDll = NULL;
        }
    }
};

#endif //IPSHARED_LIB_H
