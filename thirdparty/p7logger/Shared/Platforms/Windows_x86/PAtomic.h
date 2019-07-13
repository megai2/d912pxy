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
#pragma once

///////////////////////////////////////////////////////////////////////////////
#define  ATOMIC_ADD(io_Val, i_Add)  InterlockedExchangeAdd((LONG volatile *)io_Val, i_Add)
#define  ATOMIC_SUB(io_Val, i_Sub)  InterlockedExchangeAdd((LONG volatile *)io_Val, -((tINT32)i_Sub))
#define  ATOMIC_SET(io_Var, i_Val)  InterlockedExchange((LONG volatile *)io_Var, (LONG)i_Val)

#define  ATOMIC_INC(io_Val)         InterlockedIncrement((LONG volatile *)io_Val)
#define  ATOMIC_DEC(io_Val)         InterlockedDecrement((LONG volatile *)io_Val)
