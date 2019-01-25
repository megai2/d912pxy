////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2017 (c) Baical                                                        /
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
#pragma once

///////////////////////////////////////////////////////////////////////////////
typedef  CRITICAL_SECTION          tLOCK;


#define  LOCK_CREATE(i_Lock)       InitializeCriticalSection(&i_Lock)
#define  LOCK_DESTROY(i_Lock)      DeleteCriticalSection(&i_Lock)

#define  LOCK_ENTER(i_Lock)        EnterCriticalSection(&i_Lock)
#define  LOCK_EXIT(i_Lock)         LeaveCriticalSection(&i_Lock)
#define  LOCK_TRY(i_Lock)          TryEnterCriticalSection(&i_Lock)
