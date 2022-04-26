#include "textcache.h"

TextCache::TextCache() :
    TextCache(0, 0, "", 0)
{

}

TextCache::TextCache(int historyRecordId, QString textCache, long long createTime) :
    historyRecordId(historyRecordId),
    textCache(textCache),
    createTime(createTime)
{

}
TextCache::TextCache(int textCacheId, int historyRecordId, QString textCache, long long createTime) :
    TextCache(historyRecordId, textCache, createTime)
{
    this->textCacheId = textCacheId;
}
