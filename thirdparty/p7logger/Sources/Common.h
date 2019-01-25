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
#ifndef COMMON_H
#define COMMON_H

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include 
// WinSDKVer.h and set the _WIN32_WINNT macro to the platform you wish to support 
// before including SDKDDKVer.h.
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>


#include "GTypes.h"

#include "Length.h"

#include "PAtomic.h"
#include "PLock.h"
#include "PString.h"
#include "PTime.h"
#include "Ticks.h"
#include "IMEvent.h"
#include "PMEvent.h"
#include "PThreadShell.h"
#include "RBTree.h"
#include "AList.h"
#include "PShared.h"

#include "UTF.h"
#include "PString.h"
#include "WString.h"
#include "PFileSystem.h"

#include "PProcess.h"
#include "IJournal.h"
#include "PJournal.h"

#endif //COMMON_H
