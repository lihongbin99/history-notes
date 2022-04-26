#ifndef TEXTCACHE_H
#define TEXTCACHE_H

#include <QString>

class TextCache
{
public:
    TextCache();
    TextCache(int historyRecordId, QString textCache, long long createTime);
    TextCache(int textCacheId, int historyRecordId, QString textCache, long long createTime);

    int textCacheId;        // 缓存序号
    int historyRecordId;    // 历史记录序号
    QString textCache;      // 缓存文本
    long long createTime;   // 操作时间
};

#endif // TEXTCACHE_H
