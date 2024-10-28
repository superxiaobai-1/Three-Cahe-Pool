#pragma once

#include "Common.h"
#include "ObjectPool.h"
#include "span.h"
#include "free_list.h"
class PageCache
{
public:
	
	static PageCache* GetInstance()
	{
          std::call_once(init_flag, []() { page_instance_ = new PageCache(); });
          return page_instance_;
        }

        Span* NewSpan(size_t k);

        // ��ȡ����span��ӳ��
	Span* MapObjectToSpan(void* obj);

	// �ͷſ���span�ص�PageCache�����ϲ����ڵ�span
	void ReleaseSpanToPageCache(Span* span);

public:
	std::mutex _pageMtx;

       private:
        PageCache() {};
        PageCache(const PageCache&) = delete;

        // std::unordered_map<PAGE_ID, Span*> _idSpanMap;
        std::map<PAGE_ID, Span*> _idSpanMap;

        ObjectPool<Span> span_pool_;

        SpanList span_lists_[NPAGES];

        // static PageCache _sInst;
        static PageCache* page_instance_;
        static std::once_flag init_flag;
};