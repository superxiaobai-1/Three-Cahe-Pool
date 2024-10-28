#include "PageCache.h"

PageCache* PageCache::page_instance_ = nullptr;
std::once_flag PageCache::init_flag;

// page cache ����һ��span
Span* PageCache::NewSpan(size_t k)
{
	// �ȶ���һ��
	assert(k > 0);

	// ���k > 128ҳ ��ϵͳ����
	if (k > NPAGES - 1)
	{
		void* ptr = SystemAlloc(k);
		//Span* span = new Span;
                Span* span = span_pool_.New();
                span->page_id_ = (PAGE_ID)ptr >> PAGE_SHITF;
                span->n_ = k;

                _idSpanMap[span->page_id_] = span;
                return span;
        }

        // �ȼ���k��Ͱ����û��Span
        if (!span_lists_[k].Empty()) {
          // ����ע���Ѿ����������⣡����
          Span* kSpan = span_lists_[k].PopFront();
          for (PAGE_ID i = 0; i < kSpan->n_; ++i) {
            _idSpanMap[kSpan->page_id_ + i] = kSpan;
          }
          // ͷɾ��һ����������ȥʵ��PopFront���߼�
          return kSpan;
        }

        // ���k��ͰΪ�գ��������Ͱ���ҵ�һ���ǿ�Ͱ��ֱ�����һ��Ͱ
        for (size_t i = k + 1; i < NPAGES; i++)
	{
          if (!span_lists_[i].Empty()) {
            Span* nSpan = span_lists_[i].PopFront();
            // ��
            // Span* kSpan = new Span;
            Span* kSpan = span_pool_.New();

            // �ֳ�npan ��һ�� kҳ���أ�Ȼ��n-kҳ��span�ҵ�n-k��Ͱ��
            // ����ע�⣬ֻ��Ҫ��ҳ�ź�ҳ���Ϳ����ˣ�span�ҵ��ڴ�ĵ�ַ��ҳ��ȡ
            kSpan->page_id_ = nSpan->page_id_;
            kSpan->n_ = k;

            nSpan->n_ -= k;
            nSpan->page_id_ += k;

            span_lists_[nSpan->n_].PushFront(nSpan);
            // �洢nSpan����βҳ��nSpan����ӳ�䣬����page cache�����ڴ�

            _idSpanMap[nSpan->page_id_] = nSpan;
            _idSpanMap[nSpan->page_id_ + nSpan->n_ - 1] = nSpan;

            // ���ص�kSpan��ҲҪ����id��span��ӳ�䣬����central
            // cache����С���ڴ�ʱ�����Ҷ�Ӧ��span

            for (PAGE_ID i = 0; i < kSpan->n_; ++i) {
              _idSpanMap[kSpan->page_id_ + i] = kSpan;
            }
            return kSpan;
          }
        }

        // ������˵�����������λ�ö�û�д�ҳ��span
        // ��ʱ�����Ҫ�������һ��128ҳ��span
        // Span* bigSpan = new Span;
        Span* bigSpan = span_pool_.New();

        void* ptr = SystemAlloc(NPAGES - 1);

        bigSpan->n_ = NPAGES - 1;
        bigSpan->page_id_ = (PAGE_ID)ptr >> PAGE_SHITF;

        span_lists_[bigSpan->n_].PushFront(bigSpan);

        return NewSpan(k);
}



// ��ȡ����span��ӳ��
Span* PageCache::MapObjectToSpan(void* obj)
{
	PAGE_ID id = ((PAGE_ID)obj >> PAGE_SHITF);
	std::unique_lock<std::mutex> lock(_pageMtx);
	if (_idSpanMap.find(id) != _idSpanMap.end())
	{
		return _idSpanMap[id];
	}
	else
	{
		assert(false);
		return nullptr;
	}
}

// �ͷſ���span�ص�PageCache�����ϲ����ڵ�span
void PageCache::ReleaseSpanToPageCache(Span* span)
{
  if (span->n_ > NPAGES - 1) {
    void* ptr = (void*)(span->page_id_ << PAGE_SHITF);
    SystemFree(ptr, span->obj_size_);
    span_pool_.Delete(span);
    return;
  }
  // �ϲ����ڵ�span����ǰ�ϲ��������⣺ǰ���span������þͲ��ܽ��кϲ�
  while (1) {
    PAGE_ID prevId = span->page_id_ - 1;

    auto ret = _idSpanMap.find(prevId);
    // ǰ�ߵ�ҳ��û���˾Ͳ��Ͳ���
    if (ret == _idSpanMap.end()) {
      break;
    }

    // ǰ������ҳ��ʹ�ã����ϲ�
    Span* prevSpan = ret->second;
    if (prevSpan->is_use_ == true) {
      break;
    }

    // ����128ҳ��spanҲ���ϲ�
    if (prevSpan->n_ + span->n_ > NPAGES - 1) {
      break;
    }

    // ����ϲ����߼�

    // span->page_id_ -= prevSpan->n_;
    span->page_id_ = prevSpan->page_id_;
    span->n_ += prevSpan->n_;

    span_lists_[prevSpan->n_].Erase(prevSpan);
    // delete prevSpan;
    span_pool_.Delete(prevSpan);
  }

  while (1) {
    PAGE_ID nextID = span->page_id_ + span->n_;
    auto ret = _idSpanMap.find(nextID);
    if (ret == _idSpanMap.end()) {
      break;
    }

    Span* nextSpan = ret->second;
    if (nextSpan->is_use_ == true) {
      break;
    }

    // ����128ҳ��spanҲ���ϲ�
    if (nextSpan->n_ + span->n_ > NPAGES - 1) {
      break;
    }

    span->n_ += nextSpan->n_;

    span_lists_[nextSpan->n_].Erase(nextSpan);
    // delete nextSpan;

    span_pool_.Delete(nextSpan);
  }

  span_lists_[span->n_].PushFront(span);
  span->is_use_ = false;

  _idSpanMap[span->page_id_] = span;
  _idSpanMap[span->page_id_ + span->n_ - 1] = span;
}