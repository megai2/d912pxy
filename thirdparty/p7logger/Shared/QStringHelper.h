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
#ifndef QSTRING_HELPER_H
#define QSTRING_HELPER_H

#if defined(UTF8_ENCODING)
    #define XCHAR_TO_QSTRING(X) QString::fromUtf8(X)
#else
    #define XCHAR_TO_QSTRING(X) QString::fromWCharArray(X)
#endif

#if defined(UTF8_ENCODING)
    #define QSTRING_TO_XCHAR_TRANSLATE(QbAUniq, QStr, XChar)\
        QByteArray QbAUniq = QStr.toUtf8();\
        XChar = (const tXCHAR*)QbAUniq.constData()
    #define QSTRING_TO_XCHAR(QStr, XChar) QSTRING_TO_XCHAR_TRANSLATE(MAKE_UNIQUE(QbA), QStr, XChar)
#else
    #define QSTRING_TO_XCHAR(QStr, XChar) XChar = (const tXCHAR*)QStr.utf16();
#endif


#endif //QSTRING_HELPER_H
