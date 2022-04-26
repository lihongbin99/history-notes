#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>

#define cout qDebug()

QString parseHistoryRecord(QString &Text, int frontEqualCount, int lastEqualCount, QString &replaceString) {
    int stringLen = Text.length() - frontEqualCount - lastEqualCount;
    return Text.mid(0, frontEqualCount) +
           replaceString +
           Text.mid(frontEqualCount + stringLen, lastEqualCount);
}

QString dateTimeFormat = "yyyy-MM-dd HH:mm:ss.zzz";
QString parseDataTime(long long MSecsSinceEpoch) {
    QDateTime dataTime;
    dataTime = dataTime.fromMSecsSinceEpoch(MSecsSinceEpoch);
    return dataTime.toString(dateTimeFormat);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(title);

    // 设置宽高
    resize(300, 270);

    // 窗口置顶
    Qt::WindowFlags m_flags = windowFlags();
    setWindowFlags(m_flags | Qt::WindowStaysOnTopHint);

    // 设置图标
    if (QFileInfo("icon.jpg").exists()) {
        setWindowIcon(QIcon("icon.jpg"));
    } else {
        setWindowIcon(QIcon(":/static/icon.jpg"));
    }

    // 设置字体
    ui->notes->setFontFamily(fontFamily);
    ui->notes->setFontPointSize(fontPointSize);

    // 初始化文本
    db = new database(this);
    if (!db->init()) {
        exit(1);
    }
    TextCache textCache;
    if (!db->getLastTextCache(&textCache, 0)) {
        exit(1);
    }
    oldText = textCache.textCache;
    QList<HistoryRecord> historyRecordList = QList<HistoryRecord>();
    if (!db->getHistoryRecord(&historyRecordList, textCache.historyRecordId, -1)) {
        exit(1);
    }
    foreach (HistoryRecord historyRecord, historyRecordList) {
        oldText = parseHistoryRecord(oldText, historyRecord.frontEqualCount, historyRecord.lastEqualCount, historyRecord.newString);
    }
    ui->notes->setPlainText(oldText);
    if (historyRecordList.size() > 0) {
        HistoryRecord historyRecord = historyRecordList.at(historyRecordList.size() - 1);
        ui->last_time->setText(parseDataTime(historyRecord.createTime));
    }

    // 设置时间轴
    ui->time_line->setRange(0, timeLineRange);
    ui->time_line->setValue(timeLineRange / 2);

    connect(ui->notes, &QTextEdit::	currentCharFormatChanged, this, &MainWindow::currentCharFormatChanged);
    connect(ui->notes, &QTextEdit::textChanged, this, &MainWindow::textChanged);

    connect(ui->time_line, &QSlider::sliderPressed, this, &MainWindow::timeLinePressedd);
    connect(ui->time_line, &QSlider::sliderReleased, this, &MainWindow::timeLineReleased);
    connect(ui->time_line, &QSlider::valueChanged, this, &MainWindow::timeLineValueChanged);

    connect(ui->last_time, &QLineEdit::returnPressed, this, &MainWindow::lastTimeReturnPressed);
}

void MainWindow::currentCharFormatChanged (const QTextCharFormat &f) {
    if (f.fontFamily() != fontFamily) {
        ui->notes->setFontFamily(fontFamily);
    }
    if (f.fontPointSize() != fontPointSize) {
        ui->notes->setFontPointSize(fontPointSize);
    }
}

void MainWindow::textChanged() {
    if (isTimeLineing) {
        return;// 拖动时间轴的时候不记录操作
    }
    QString oldText;        // 旧文本
    QString newText;        // 新文本
    int oldLen;             // 旧文本长度
    int newLen;             // 新文本长度
    int frontEqualCount;    // 前面相同的字符个数
    int lastEqualCount;     // 后面相同的字符个数
    QString oldString;      // 旧文本需要删除的字符
    QString newString;      // 新文本需要添加的字符

    oldText = this->oldText;
    newText = ui->notes->toPlainText();
    this->oldText = newText;

    oldLen = oldText.length();
    newLen = newText.length();

    frontEqualCount = 0;
    for (; frontEqualCount < oldLen && frontEqualCount < newLen; frontEqualCount++) {
        if (oldText.at(frontEqualCount) != newText.at(frontEqualCount)) {
            break;
        }
    }
    if (frontEqualCount == oldLen && frontEqualCount == newLen) {
        return;// no change
    }
    lastEqualCount = 0;
    for (; lastEqualCount < oldLen - frontEqualCount && lastEqualCount < newLen - frontEqualCount; lastEqualCount++) {
        if (oldText.at(oldLen - 1 - lastEqualCount) != newText.at(newLen - 1 - lastEqualCount)) {
            break;
        }
    }

    oldString = oldText.mid(frontEqualCount, oldLen - frontEqualCount - lastEqualCount);
    newString = newText.mid(frontEqualCount, newLen - frontEqualCount - lastEqualCount);

    // Test
    QString testHTML = parseHistoryRecord(oldText, frontEqualCount, lastEqualCount, newString);
    if (testHTML != newText) {
        QMessageBox::critical(this, "错误", "测试错误1");
        exit(1);
    }
    testHTML = parseHistoryRecord(newText, frontEqualCount, lastEqualCount, oldString);
    if (testHTML != oldText) {
        QMessageBox::critical(this, "错误", "测试错误2");
        exit(1);
    }
//    else { qDebug() << "true"; }

    // Save
    HistoryRecord historyRecord(frontEqualCount, lastEqualCount, oldString, newString);
    if (!db->addHistoryRecord(historyRecord)) {
        exit(1);
    }
    ui->last_time->setText(parseDataTime(historyRecord.createTime));
    timeLineId   = -1;
    timeLineMode = 0;
    // 保存缓存
    if (historyRecord.historyRecordId % 10 == 0) {// TODO
        TextCache textCache(historyRecord.historyRecordId, newText, historyRecord.createTime);
        if (!db->addTextCache(textCache)) {
            exit(1);
        }
    }
}

void MainWindow::timeLinePressedd() {
    beforeList.clear();
    afterList.clear();
    timeLineTempId = timeLineId;
    tempText = ui->notes->toPlainText();
    if (db->getBeforeHistoryRecord(&beforeList, 50, timeLineMode > 0 ? timeLineId + 1 : timeLineId) &&
            (timeLineId < 0 ||
                db->getAfterHistoryRecord(&afterList, 50, timeLineMode < 0 ? timeLineId - 1 : timeLineId))
    ) {
        isTimeLineing = true;
        isTimeLinePressedd = true;
    }
}

void MainWindow::timeLineReleased() {
    isTimeLinePressedd = false;
    ui->time_line->setValue(50);
    isTimeLineing = false;
}

void MainWindow::timeLineValueChanged(int value) {
    QString timeLineText = tempText;
    if (value < 50) {
        for (int i = 0; i < 50 - value && i < beforeList.size(); i++) {
            HistoryRecord historyRecord = beforeList.at(i);
            timeLineText = parseHistoryRecord(timeLineText, historyRecord.frontEqualCount, historyRecord.lastEqualCount, historyRecord.oldString);
            timeLineId = historyRecord.historyRecordId;
            timeLineMode = -1;
            ui->last_time->setText(parseDataTime(historyRecord.createTime));
        }
    } else if (value > 50) {
        for (int i = 0; i < value - 50 && i < afterList.size(); i++) {
            HistoryRecord historyRecord = afterList.at(i);
            timeLineText = parseHistoryRecord(timeLineText, historyRecord.frontEqualCount, historyRecord.lastEqualCount, historyRecord.newString);
            timeLineId = historyRecord.historyRecordId;
            timeLineMode = 1;
            ui->last_time->setText(parseDataTime(historyRecord.createTime));
        }
    } else if (isTimeLinePressedd) {
        timeLineId = timeLineTempId;
        timeLineMode = 0;
    }
    if (isTimeLinePressedd) {
        ui->notes->setPlainText(timeLineText);
    }
}

void MainWindow::lastTimeReturnPressed() {
    isTimeLineing = true;
    QDateTime dateTime = QDateTime::fromString(ui->last_time->text(), dateTimeFormat);
    long long MSecsSinceEpoch = dateTime.toMSecsSinceEpoch();
    if (MSecsSinceEpoch < 0) {
        QMessageBox::critical(this, "格式异常", dateTimeFormat);
        return;
    }
    TextCache textCache;
    if (!db->getLastTextCache(&textCache, MSecsSinceEpoch)) {
        exit(1);
    }
    tempText = textCache.textCache;
    if (textCache.createTime > 0) {
        ui->last_time->setText(parseDataTime(textCache.createTime));
    }
    timeLineId = textCache.historyRecordId;

    QList<HistoryRecord> historyRecordList = QList<HistoryRecord>();
    if (!db->getHistoryRecord(&historyRecordList, textCache.historyRecordId, MSecsSinceEpoch)) {
        exit(1);
    }
    foreach (HistoryRecord historyRecord, historyRecordList) {
        tempText = parseHistoryRecord(tempText, historyRecord.frontEqualCount, historyRecord.lastEqualCount, historyRecord.newString);
    }
    ui->notes->setPlainText(tempText);
    if (historyRecordList.size() > 0) {
        HistoryRecord historyRecord = historyRecordList.at(historyRecordList.size() - 1);
        ui->last_time->setText(parseDataTime(historyRecord.createTime));
        timeLineId = historyRecord.historyRecordId;
    }

    timeLineMode = 0;
    isTimeLineing = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}
