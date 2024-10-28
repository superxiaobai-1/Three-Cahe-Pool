#pragma once
#include "Common.h"
#include "thread_cache.h"
#include "PageCache.h"
#include "size_map.h"

static void* ConcurrentAlloc(size_t size)
{
	if (size > MAX_BYTES)
	{

		size_t alignSize = SizeClass::RoundUp(size);
		
		// Ȼ������Ҫ��ҳ
		size_t kpage = alignSize >> PAGE_SHITF;
		PageCache::GetInstance()->_pageMtx.lock();
		Span* span = PageCache::GetInstance()->NewSpan(alignSize);
                span->obj_size_ = alignSize;
                PageCache::GetInstance()->_pageMtx.unlock();

                return (void*)(span->page_id_ << PAGE_SHITF);
        }
	else
	{
   
		return  ThreadCache::GetThreadCache()->Allocate(size);
	}
}


static void ConcurrentFree(void* ptr)
{
	Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
        size_t size = span->obj_size_;

        if (size > MAX_BYTES) {
          PageCache::GetInstance()->_pageMtx.lock();
          PageCache::GetInstance()->ReleaseSpanToPageCache(span);
          PageCache::GetInstance()->_pageMtx.unlock();

        } else {
          ThreadCache::GetThreadCache()->Deallocate(ptr, size);
        }
}