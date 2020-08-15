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

		union PreparedKey
		{
			Hash val;
			StepType tip;

			const Hash::Data& data()
			{
				return val.value;
			}

			PreparedKey(const Key& obj)
			{
				val.from(obj);
			}
		};
	
	private:
		static const int nodeChilds = (1 << (8 * sizeof(StepType))) - 1;
		static const int treeDepth = sizeof(Hash) / sizeof(StepType);

		struct Node
		{
			IndexType childs[nodeChilds];
		};

		struct Leaf
		{
			Value val;
			PreparedKey origHash;
		};

		Value* insertNew(const PreparedKey& hash, IndexType depth, IndexType lastNode)
		{
			IndexType nextNode;

			for (; depth > 1; --depth)
			{
				nextNode = nodes.next();
				nodes[lastNode].childs[hash.val[depth]] = nextNode;
				lastNode = nextNode;
			}

			nextNode = leafs.next();
			nodes[lastNode].childs[hash.tip] = nextNode;
			
			leafs[nextNode].origHash = hash;
			return &leafs[nextNode].val;
		}

		Value* searchInner(const PreparedKey& hash, IndexType& depth, IndexType& lastNode)
		{
			lastNode = baseNode;
			Node* cnode = &nodes[lastNode];
			for (depth = treeDepth; depth > 1; --depth)
			{			
				IndexType childNode = cnode->childs[hash.val[depth]];
				if (!childNode)
					return nullptr;

				cnode = &nodes[childNode];
				lastNode = childNode;
			}

			IndexType leafNode = cnode->childs[hash.tip];
			if (!leafNode)
				return nullptr;

			Leaf& retLeaf = leafs[leafNode];
			error::check(retLeaf.origHash.val == hash.val, L"hash collision in Memtree");

			return &retLeaf.val;
		}

	public:
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

		Value& find(const Key& key) { return findPrepared(transformKey(key)); }
		Value* contains(const Key& key) { return containsPrepared(transformKey(key)); }
		Value& operator[](const Key& key) { return find(key); }

	private:
		IndexType baseNode;

		Trivial::PushBuffer<Node> nodes;
		Trivial::PushBuffer<Leaf> leafs;
	};
	
}


