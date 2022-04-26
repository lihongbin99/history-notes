#include "historyrecord.h"

HistoryRecord::HistoryRecord():
    HistoryRecord(0, 0, 0, "", "", 0)
{

}

HistoryRecord::HistoryRecord(int historyRecordId, int frontEqualCount, int lastEqualCount, QString oldString, QString newString, long long createTime):
    HistoryRecord(frontEqualCount, lastEqualCount, oldString, newString)
//  , historyRecordId(historyRecordId)
//  , createTime(createTime)
{
    this->historyRecordId = historyRecordId;
    this->createTime = createTime;
}

HistoryRecord::HistoryRecord(int frontEqualCount, int lastEqualCount, QString oldString, QString newString):
    frontEqualCount(frontEqualCount)
  , lastEqualCount(lastEqualCount)
  , oldString(oldString)
  , newString(newString)
{

}
