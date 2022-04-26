#include "database.h"

#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QPluginLoader>
#include <QSqlDriverPlugin>
#include <QApplication>

#include <QDebug>
#define cout qDebug()

QString CurrentTimeLineId = "CurrentTimeLineId";

database::database(QWidget *parent) : QWidget(parent)
{

}

bool database::init() {
    QPluginLoader driverload(qApp->applicationDirPath()+"/plugins/sqldrivers/sqlitecipher.dll");
    if (!driverload.load()) {
        QMessageBox::critical(this, "数据库设置失败", "加载驱动失败1");
        exit(0);
    }
    QSqlDriverPlugin *plugin=qobject_cast<QSqlDriverPlugin*>(driverload.instance());
    if (!plugin) {
        QMessageBox::critical(this, "数据库设置失败", "加载驱动失败2");
        exit(0);
    }
    QSqlDriver *driver = plugin->create("SQLITECIPHER");

    QString password = "";
    bool newDB = false;

    QFileInfo fileInfo("notes.db");
    if (fileInfo.exists()) {
        (password = QInputDialog::getText(this, "数据库设置", "请输入密码"));
    } else {
        newDB = true;
        QString repassword = "";
        do {
            password = QInputDialog::getText(this, "数据库设置", "请设置密码");
            repassword = QInputDialog::getText(this, "数据库设置", "请再次输入密码");
        } while (password != repassword);
    }

    //添加数据库驱动
    db = QSqlDatabase::addDatabase(driver);
    //设置数据库名称
    db.setDatabaseName("notes.db");
    db.setUserName("roor");
    db.setPassword(password);
    //打开数据库
    bool connectResult = db.open();
    if (!connectResult) {
        QMessageBox::critical(this, "连接数据库失败", db.lastError().text());
    } else if (newDB) {
        return createTable();
    }
    return connectResult;
}

bool database::createTable() {
    return createTable("CREATE TABLE \"history_record\" (\"history_record_id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT,\"front_equal_count\" integer,\"last_equal_count\" integer,\"old_string\" tex,\"new_string\" text,\"create_time\" integer);")
           && createTable("CREATE TABLE \"text_cache\" (\"text_cache_id\" integer NOT NULL PRIMARY KEY AUTOINCREMENT,\"history_record_id\" integer NOT NULL,\"text_cache\" text,\"create_time\" integer)")
            ;
}

bool database::createTable(QString createSQL) {
    QSqlQuery query;
    query.prepare(createSQL);
    if(!query.exec()) {
        QMessageBox::critical(this, "建表错误", query.lastError().text());
        return false;
    }
    return true;
}

bool database::addHistoryRecord(HistoryRecord &historyRecord) {
    historyRecord.createTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QSqlQuery query;
    QString insertSQL = "INSERT INTO \"history_record\" (\"front_equal_count\", \"last_equal_count\", \"old_string\", \"new_string\", \"create_time\") VALUES(?, ?, ?, ?, ?)";
    query.prepare(insertSQL);
    query.addBindValue(historyRecord.frontEqualCount);
    query.addBindValue(historyRecord.lastEqualCount);
    query.addBindValue(historyRecord.oldString);
    query.addBindValue(historyRecord.newString);
    query.addBindValue(historyRecord.createTime);
    if(!query.exec()) {
        QMessageBox::critical(this, "添加历史记录错误1", query.lastError().text());
        return false;
    }
    query.exec("SELECT last_insert_rowid()");
    if(!query.exec()) {
        QMessageBox::critical(this, "添加历史记录错误2", query.lastError().text());
        return false;
    }
    query.next();
    historyRecord.historyRecordId = query.value(0).toInt();
    return true;
}

QString selectFieldSQL = "\"history_record_id\", \"front_equal_count\", \"last_equal_count\", \"old_string\", \"new_string\", \"create_time\"";
void parseHistoryRecord(QSqlQuery &query, QList<HistoryRecord> *historyRecordList) {
    while(query.next()) {
        int historyRecordId  = query.value(0).toInt();
        int frontEqualCount  = query.value(1).toInt();
        int lastEqualCount   = query.value(2).toInt();
        QString oldString    = query.value(3).toString();
        QString newString    = query.value(4).toString();
        long long createTime = query.value(5).toLongLong();
        historyRecordList->append(HistoryRecord(historyRecordId, frontEqualCount, lastEqualCount, oldString, newString, createTime));
    }
}

bool database::getHistoryRecord(QList<HistoryRecord> *historyRecordList, int baseHistoryRecordId, long long maxCreateTime) {
    QSqlQuery query;
    QString selectSQL = "SELECT " + selectFieldSQL + " FROM \"history_record\" WHERE \"history_record_id\" > ? ORDER BY \"history_record_id\" ASC";
    if (maxCreateTime > 0) {
        selectSQL = "SELECT " + selectFieldSQL + " FROM \"history_record\" WHERE \"history_record_id\" > ? AND \"create_time\" <= ? ORDER BY \"history_record_id\" ASC";
    }
    query.prepare(selectSQL);
    query.addBindValue(baseHistoryRecordId);
    if (maxCreateTime > 0) {
        query.addBindValue(maxCreateTime);
    }
    if(!query.exec()) {
        QMessageBox::critical(this, "获取操作记录失败", query.lastError().text());
        return false;
    }
    parseHistoryRecord(query, historyRecordList);
    return true;
}

bool database::getBeforeHistoryRecord(QList<HistoryRecord> *historyRecordList, int count, int baseHistoryRecordId) {
    QSqlQuery query;
    QString selectSQL = "SELECT " + selectFieldSQL + " FROM \"history_record\" ORDER BY \"history_record_id\" DESC LIMIT ?";
    if (baseHistoryRecordId >= 0) {
        selectSQL = "SELECT " + selectFieldSQL + " FROM \"history_record\" WHERE \"history_record_id\" < ? ORDER BY \"history_record_id\" DESC LIMIT ?";
    }
    query.prepare(selectSQL);
    if (baseHistoryRecordId >= 0) {
        query.addBindValue(baseHistoryRecordId);
    }
    query.addBindValue(count);
    if(!query.exec()) {
        QMessageBox::critical(this, "获取历史记录错误1", query.lastError().text());
        return false;
    }
    parseHistoryRecord(query, historyRecordList);
    return true;
}

bool database::getAfterHistoryRecord(QList<HistoryRecord> *historyRecordList, int count, int baseHistoryRecordId) {
    QSqlQuery query;
    QString selectSQL = "SELECT " + selectFieldSQL + " FROM \"history_record\" WHERE \"history_record_id\" > ? ORDER BY \"history_record_id\" ASC LIMIT ?";
    query.prepare(selectSQL);
    query.addBindValue(baseHistoryRecordId);
    query.addBindValue(count);
    if(!query.exec()) {
        QMessageBox::critical(this, "获取历史记录错误2", query.lastError().text());
        return false;
    }
    parseHistoryRecord(query, historyRecordList);
    return true;
}

bool database::addTextCache(TextCache &textCache) {
    QSqlQuery query;
    QString insertSQL = "INSERT INTO \"text_cache\" (\"history_record_id\", \"text_cache\", \"create_time\") VALUES(?, ?, ?)";
    query.prepare(insertSQL);
    query.addBindValue(textCache.historyRecordId);
    query.addBindValue(textCache.textCache);
    query.addBindValue(textCache.createTime);
    if(!query.exec()) {
        QMessageBox::critical(this, "添加缓存错误", query.lastError().text());
        return false;
    }
    return true;
}

bool database::getLastTextCache(TextCache *textCache, long long baseCreateTime) {
    QSqlQuery query;
    QString selectSQL = "SELECT \"text_cache_id\", \"history_record_id\", \"text_cache\", \"create_time\" FROM \"text_cache\" ORDER BY \"text_cache_id\" DESC LIMIT 1";
    if (baseCreateTime > 0) {
        selectSQL = "SELECT \"text_cache_id\", \"history_record_id\", \"text_cache\", \"create_time\" FROM \"text_cache\" WHERE \"create_time\" <= ? ORDER BY \"text_cache_id\" DESC LIMIT 1";
    }
    query.prepare(selectSQL);
    if (baseCreateTime > 0) {
        query.addBindValue(baseCreateTime);
    }
    if (!query.exec()) {
        QMessageBox::critical(this, "获取缓存失败", query.lastError().text());
        return false;
    }
    if (query.next()) {
        textCache->textCacheId     = query.value(0).toInt();
        textCache->historyRecordId = query.value(1).toInt();
        textCache->textCache       = query.value(2).toString();
        textCache->createTime      = query.value(3).toLongLong();
    }
    return true;
}

database::~database() {
    db.close();
}
