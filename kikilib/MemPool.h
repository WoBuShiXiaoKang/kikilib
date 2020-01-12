//@Author Liu Yukang 
#pragma once
#include "List.h"
#include "Parameter.h"
#include "utils.h"

namespace kikilib
{
	//ÿ�ο��Դ��ڴ���л�ȡobjSize��С���ڴ��
	template<size_t objSize>
	class MemPool
	{
	public:
		MemPool()
			: _mallocTimes(0)
		{ };

		~MemPool();

		DISALLOW_COPY_MOVE_AND_ASSIGN(MemPool);

		void* AllocAMemBlock();
		inline void FreeAMemBlock(void* block) { _freeList.push_back(block); };

	private:
		//��������
		List<void*> _freeList;
		//ÿ��ʵ��malloc�����Ĵ���ڴ�����
		List<void*> _mallocList;
		//ʵ��malloc�Ĵ���
		size_t _mallocTimes;
	};

	template<size_t objSize>
	MemPool<objSize>::~MemPool()
	{
		for (auto it = _mallocList.begin(); it != _mallocList.end(); ++it)
		{
			free(*it);
		}
	}

	template<size_t objSize>
	void* MemPool<objSize>::AllocAMemBlock()
	{
		void* ret;
		if (_freeList.empty())
		{
			size_t mallocCnt = Parameter::memPoolMallocObjCnt + _mallocTimes;
			void* newMallocBlk = malloc(mallocCnt * objSize);
			_mallocList.push_back(newMallocBlk);
			for (size_t i = 0; i < mallocCnt; ++i)
			{
				_freeList.push_back(newMallocBlk);
				newMallocBlk = static_cast<char*>(newMallocBlk) + objSize;
			}
			++_mallocTimes;
		}
		ret = _freeList.front();
		_freeList.pop_front();
		return ret;
	}
}

