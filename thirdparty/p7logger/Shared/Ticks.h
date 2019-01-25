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
#ifndef TICKS_H
#define TICKS_H


////////////////////////////////////////////////////////////////////////////////
class CTicks
{
public:
    static tUINT32 inline Difference(tUINT32 i_dwCur, tUINT32 i_dwPrev)
    {
        tUINT32 l_dwReturn = 0;

        if (i_dwCur >= i_dwPrev)
        {
            l_dwReturn = i_dwCur - i_dwPrev;
        }
        else
        {
            l_dwReturn = (0xFFFFFFFF - i_dwPrev) + i_dwCur;
        }

        return l_dwReturn;
    }
};

#endif //TICKS_H
