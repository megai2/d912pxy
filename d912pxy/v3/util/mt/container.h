/*
MIT License

Copyright(c) 2020 megai2

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#pragma once
#include "stdafx.h"

namespace d912pxy
{
	namespace mt
	{
		namespace containter
		{
			template<typename Container, typename ValueType>
			class RefBase
			{
				typename Container::Provider::Lock& lockObj;
			public:
				typename Container::PreparedKey key;
				ValueType val;

				RefBase(Container& container, const typename Container::Provider::Key& rawKey)
					: lockObj(container.getLock())
					, key(container.prepareKey(rawKey))
				{
					lockObj.hold();
				}

				~RefBase() { lockObj.release(); }
			};

			template<typename Container>
			class Ref : public RefBase<Container, typename Container::Provider::Value&>
			{
			public:
				Ref(Container& container, const typename Container::Provider::Key& rawKey)
					: RefBase(container, rawKey)
				{
					RefBase::val = container.findPrepared(RefBase::key);
				}
			};

			template<typename Container>
			class OptRef : public RefBase<Container, typename Container::Provider::Value*>
			{
				typedef typename RefBase<Container, typename Container::Provider::Value*> RefBaseType;

				Container& containerRef;
			public:	
				OptRef(Container& container, const typename Container::Provider::Key& rawKey)
					: RefBaseType(container, rawKey)
					, containerRef(container)
				{				
					RefBaseType::val = container.containsPrepared(RefBaseType::key);
				}

				typename Container::Provider::Value& add()
				{
					//TODO: can be optimized by skipping search sequenece
					return containerRef.findPrepared(RefBaseType::key);
				}
			};

			template<typename Container, typename ValueType>
			class RefBasePrepared
			{
				typename Container::Provider::Lock& lockObj;
			public:
				typename Container::PreparedKey key;
				ValueType val;

				RefBasePrepared(Container& container, const typename Container::PreparedKey& preparedKey)
					: lockObj(container.getLock())
					, key(preparedKey)
				{
					lockObj.hold();
				}

				~RefBasePrepared() { lockObj.release(); }
			};

			template<typename Container>
			class RefPrepared : public RefBasePrepared<Container, typename Container::Provider::Value&>
			{
			public:
				RefPrepared(Container& container, const typename Container::PreparedKey& preparedKey)
					: RefBase(container, preparedKey)
				{
					RefBase::val = container.findPrepared(RefBase::key);
				}
			};

			template<typename Container>
			class OptRefPrepared : public RefBasePrepared<Container, typename Container::Provider::Value*>
			{
				typedef typename RefBasePrepared<Container, typename Container::Provider::Value*> RefBaseType;

				Container& containerRef;
			public:
				OptRefPrepared(Container& container, const typename Container::PreparedKey& preparedKey)
					: RefBaseType(container, preparedKey)
					, containerRef(container)
				{
					RefBaseType::val = container.containsPrepared(RefBaseType::key);
				}

				typename Container::Provider::Value& add()
				{
					//TODO: can be optimized by skipping search sequenece
					return containerRef.findPrepared(RefBaseType::key);
				}
			};

			template <typename LockType, typename ValueType, typename KeyType>
			class Provider
			{
				LockType lockObj;

			public:
				typedef LockType Lock;
				typedef ValueType Value;
				typedef KeyType Key;

				Lock& getLock() { return lockObj; }
				
			};

		}
	}
}
