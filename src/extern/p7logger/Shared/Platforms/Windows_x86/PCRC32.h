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

static tUINT32 Get_CRC32(tUINT8 *i_pData, size_t i_szCount)
{
    tUINT32 l_dwResult = 0xFFFFFFFF;

    if (    (NULL == i_pData)
         || (0  >= i_szCount) 
       )
    {
        return l_dwResult;
    }


#if defined(GTX64) || defined(__linux__) || !defined(_MSC_VER)

    for (size_t l_szI = 0;  l_szI < i_szCount;  l_szI ++)
    {
        l_dwResult = g_pCRC32_Table[(l_dwResult  ^ i_pData[l_szI]) & 0xFF ] ^ (l_dwResult >> 8);
    }

     //Suppress warning that this is not used
    (void)g_ppCRC32_Table;

#else

    // Register use:
    //  eax - CRC32 value
    //  ebx - a lot of things
    //  ecx - CRC32 value
    //  edx - address of end of buffer
    //  esi - address of start of buffer
    //  edi - CRC32 table
    __asm
    {
            // Save the esi and edi registers
            push esi
            push edi

            mov ecx, l_dwResult                 // load dwCrc32

            mov edi, g_ppCRC32_Table            // Load the CRC32 table

            mov esi, i_pData                    // Load buffer
            mov ebx, i_szCount                  // Load dwBytesRead
            lea edx, [esi + ebx]                // Calculate the end of the buffer

    crc32loop:
            xor eax, eax                        // Clear the eax register
            mov bl, byte ptr [esi]              // Load the current source byte

            mov al, cl                          // Copy crc value into eax
            inc esi                             // Advance the source pointer

            xor al, bl                          // Create the index into the CRC32 table
            shr ecx, 8

            mov ebx, [edi + eax * 4]            // Get the value out of the table
            xor ecx, ebx                        // xor with the current byte

            cmp edx, esi                        // Have we reached the end of the buffer?
            jne crc32loop

            // Restore the edi and esi registers
            pop edi
            pop esi

            lea eax, l_dwResult                 // Load the pointer to dwCrc32
            mov [eax], ecx                      // Write the result
    }

#endif

    return l_dwResult;
}


// #if  defined(_WIN32) || defined(_WIN64)
// tUINT32 Get_CRC32_SSE4(tUINT8 *i_pData, tUINT32 i_szCount)
// {
//     tUINT32 l_dwCount  = i_szCount >> 2; //count of DWORDs
//     tUINT32 l_dwResult = 0x0;
//     
//     while (l_dwCount--)
//     {
//         l_dwResult = _mm_crc32_u32(l_dwResult, *(DWORD*)i_pData);
//         i_pData += 4;
//     }
// 
//     l_dwCount = i_szCount & 0x3; //bytes count at the tail
// 
//     while (l_dwCount--)
//     {
//         l_dwResult = _mm_crc32_u8(l_dwResult, *i_pData);
//         i_pData ++;
//     }
// 
//     return l_dwResult;
// } 
// #endif 
