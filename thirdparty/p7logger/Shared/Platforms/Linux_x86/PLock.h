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
#ifndef PLOCK_H
#define PLOCK_H

#include <pthread.h>

///////////////////////////////////////////////////////////////////////////////
typedef pthread_mutex_t           tLOCK;


#define  LOCK_CREATE(i_Lock)       {\
                                       pthread_mutexattr_t l_sAttr;\
                                       memset(&l_sAttr, 0, sizeof(pthread_mutexattr_t));\
                                       pthread_mutexattr_init(&l_sAttr);\
                                       pthread_mutexattr_settype(&l_sAttr,\
                                                                 PTHREAD_MUTEX_RECURSIVE);\
                                       pthread_mutex_init(&i_Lock, &l_sAttr);\
                                       pthread_mutexattr_destroy(&l_sAttr);\
                                   }\


#define  LOCK_DESTROY(i_Lock)      pthread_mutex_destroy(&i_Lock)

#define  LOCK_ENTER(i_Lock)        pthread_mutex_lock(&i_Lock)
#define  LOCK_EXIT(i_Lock)         pthread_mutex_unlock(&i_Lock)
#define  LOCK_TRY(i_Lock)          ((0 == pthread_mutex_trylock(&i_Lock)) ? TRUE : FALSE)

#endif //PLOCK_H
