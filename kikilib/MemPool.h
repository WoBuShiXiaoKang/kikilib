//@Author Liu Yukang 
#pragma once
#include "Parameter.h"
#include "utils.h"

namespace kikilib
{
	struct MemBlockNode
	{
		union 
		{
			MemBlockNode* next;
			char data;
		};
	};

	//ÿ�ο��Դ��ڴ���л�ȡobjSize��С���ڴ��
	template<size_t objSize>
	class MemPool
	{
	public:
		MemPool()
			:_freeListHead(nullptr), _mallocListHead(nullptr), _mallocTimes(0)
		{ };

		~MemPool();

		DISALLOW_COPY_MOVE_AND_ASSIGN(MemPool);

		void* AllocAMemBlock();
		void FreeAMemBlock(void* block);

	private:
		//��������
		MemBlockNode* _freeListHead;
		//malloc�Ĵ��ڴ������
		MemBlockNode* _mallocListHead;
		//ʵ��malloc�Ĵ���
		size_t _mallocTimes;
	};

	template<size_t objSize>
	MemPool<objSize>::~MemPool()
	{
		while (_mallocListHead)
		{
			MemBlockNode* mallocNode = _mallocListHead;
			_mallocListHead = mallocNode->next;
			free(static_cast<void*>(mallocNode));
		}
	}

	template<size_t objSize>
	void* MemPool<objSize>::AllocAMemBlock()
	{
		void* ret;
		if (nullptr == _freeListHead)
		{
			size_t mallocCnt = Parameter::memPoolMallocObjCnt + _mallocTimes;
			void* newMallocBlk = malloc(mallocCnt * objSize + sizeof(MemBlockNode));
			MemBlockNode* mallocNode = static_cast<MemBlockNode*>(newMallocBlk);
			mallocNode->next = _mallocListHead;
			_mallocListHead = mallocNode;
			newMallocBlk = static_cast<char*>(newMallocBlk) + sizeof(MemBlockNode);
			for (size_t i = 0; i < mallocCnt; ++i)
			{
				MemBlockNode* newNode = static_cast<MemBlockNode*>(newMallocBlk);
				newNode->next = _freeListHead;
				_freeListHead = newNode;
				newMallocBlk = static_cast<char*>(newMallocBlk) + objSize;
			}
			++_mallocTimes;
		}
		ret = &(_freeListHead->data);
		_freeListHead = _freeListHead->next;
		return ret;
	}

	template<size_t objSize>
	void MemPool<objSize>::FreeAMemBlock(void* block)
	{
		if (nullptr == block)
		{
			return;
		}
		MemBlockNode* newNode = static_cast<MemBlockNode*>(block);
		newNode->next = _freeListHead;
		_freeListHead = newNode;
	}
}