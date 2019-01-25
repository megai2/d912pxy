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
#ifndef LENGTH_H
#define LENGTH_H

template <typename Type, size_t i_szLength>
char (&GetCharArray(Type (&i_pArray)[i_szLength]))[i_szLength];
#define LENGTH(i_pArray) (sizeof(GetCharArray(i_pArray)))

#define MEMBER_SIZE(Structure, Memeber)         sizeof(((Structure*)0)->Memeber)


#endif //LENGTH_H
