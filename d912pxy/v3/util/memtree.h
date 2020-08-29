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

	template<typename Key, typename Value, typename Hash>
	class Memtree : public BaseObject, public mt::containter::Provider<mt::sync::Lock, Value, Key>
	{
	public:
		typedef uint8_t StepType;
		typedef uint32_t IndexType;	

		struct PreparedKey
		{
			Hash val;
			
			StepType tip() const { return val[0]; }
			const typename Hash::Data& data() { return val.value; }

			PreparedKey() {}
			PreparedKey(const Key& obj) { val.from(obj); }

			static PreparedKey fromRawData(typename Hash::Data& rawData)
			{
				PreparedKey ret;
				ret.val.value = rawData;
				return ret;
			}
		};
	
	private:
		static const int nodeChilds = (1 << (8 * sizeof(StepType)));
		static const int treeDepth = sizeof(Hash) / sizeof(StepType);

		struct Node
		{
			IndexType childs[nodeChilds];
		};

		struct Leaf
		{
			Value val;
			//TODO: add wrap to this class with hash collision check
		};

		Value* insertNew(const PreparedKey& hash, IndexType depth, IndexType lastNode)
		{
			IndexType nextNode;

			for (; depth >= 1; --depth)
			{
				nextNode = nodes.next();
				nodes[lastNode].childs[hash.val[depth]] = nextNode;
				lastNode = nextNode;
			}

			nextNode = leafs.next();
			nodes[lastNode].childs[hash.tip()] = nextNode;
			
			return &leafs[nextNode].val;
		}

		Value* searchInner(const PreparedKey& hash, IndexType& depth, IndexType& lastNode)
		{
			lastNode = baseNode;
			Node* cnode = &nodes[lastNode];
			for (depth = treeDepth-1; depth >= 1; --depth)
			{			
				IndexType childNode = cnode->childs[hash.val[depth]];
				if (!childNode)
					return nullptr;

				cnode = &nodes[childNode];
				lastNode = childNode;
			}

			IndexType leafNode = cnode->childs[hash.tip()];
			if (!leafNode)
				return nullptr;

			Leaf& retLeaf = leafs[leafNode];
			return &retLeaf.val;
		}

	public:
		class Iterator
		{
			Leaf* current;
		public:
			Iterator(Leaf* base) : current(base) { }
			bool operator<(const Iterator& b) { return current < b.current; }							   
			Iterator& operator++() { ++current;	return *this; }
			Value& value() { return current->val; }
		};

		Memtree()
		{
			baseNode = nodes.next();
		}

		PreparedKey prepareKey(const Key& key)
		{
			return PreparedKey(key);
		}
		
		Value* containsPrepared(const PreparedKey& hash)
		{
			IndexType depth;
			IndexType lastChildNode;
			return searchInner(hash, depth, lastChildNode);
		}

		Value& findPrepared(const PreparedKey& hash)
		{
			IndexType depth;
			IndexType lastNode;
			Value* ret = searchInner(hash, depth, lastNode);

			if (!ret)
				ret = insertNew(hash, depth, lastNode);

			return *ret;
		}

		Value& find(const Key& key) { return findPrepared(prepareKey(key)); }
		Value* contains(const Key& key) { return containsPrepared(prepareKey(key)); }
		Value& operator[](const Key& key) { return find(key); }

		Iterator begin() { return Iterator(&leafs[1]); }
		Iterator end() { return Iterator((&leafs.head()) + 1); }

	private:
		IndexType baseNode;

		Trivial::PushBuffer<Node, IndexType> nodes;
		Trivial::PushBuffer<Leaf, IndexType> leafs;
	};
	
}


