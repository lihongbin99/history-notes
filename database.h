#ifndef DATABASE_H
#define DATABASE_H

#include <QWidget>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "historyrecord.h"
#include "textcache.h"

class database : public QWidget
{
    Q_OBJECT
public:
    explicit database(QWidget *parent = nullptr);
    ~database();

    bool init();


    bool addHistoryRecord(HistoryRecord &historyRecord);
    bool getHistoryRecord(QList<HistoryRecord> *historyRecordList, int baseHistoryRecordId, long long maxCreateTime);
    bool getBeforeHistoryRecord(QList<HistoryRecord> *historyRecordList, int count, int baseHistoryRecordId);
    bool getAfterHistoryRecord (QList<HistoryRecord> *historyRecordList, int count, int baseHistoryRecordId);

    bool addTextCache(TextCache &textCache);
    bool getLastTextCache(TextCache *textCache, long long baseCreateTime);

signals:

private:
    QSqlDatabase db;

    bool createTable();
    bool createTable(QString createSQL);

};

#endif // DATABASE_H
