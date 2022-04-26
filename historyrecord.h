#ifndef HISTORYRECORD_H
#define HISTORYRECORD_H

#include <QString>

class HistoryRecord
{
public:
    HistoryRecord();
    HistoryRecord(int frontEqualCount, int lastEqualCount, QString oldString, QString newString);
    HistoryRecord(int historyRecordId, int frontEqualCount, int lastEqualCount, QString oldString, QString newString, long long createTime);

    int historyRecordId;    // 历史记录序号
    int frontEqualCount;    // 前面相同的字符个数
    int lastEqualCount;     // 后面相同的字符个数
    QString oldString;      // 旧文本需要删除的字符
    QString newString;      // 新文本需要添加的字符
    long long createTime;   // 操作时间
};

#endif // HISTORYRECORD_H
