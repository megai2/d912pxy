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
#include <dlfcn.h>

class CPSharedLib
    : public CSharedLib
{
protected:
    void *m_pSo;

public:
    CPSharedLib(const tXCHAR *i_pPath)
        : CSharedLib(i_pPath)
        , m_pSo(NULL)
    {
        m_pSo = dlopen(i_pPath, RTLD_LAZY);

        if (m_pSo)
        {
            m_bState = TRUE;
        }
    }

    void *GetFunction(const tACHAR *i_pName)
    {
        dlerror(); /* Clear any existing error */

        void* l_pReturn = (void*) (dlsym(m_pSo, i_pName));
        
        if (NULL != dlerror())
        {
            return NULL;
        }

        return l_pReturn;
    }

    virtual ~CPSharedLib()
    {
        if (m_pSo)
        {
            dlclose(m_pSo);
            m_pSo = NULL;
        }
    }
};


#endif //IPSHARED_LIB_H
