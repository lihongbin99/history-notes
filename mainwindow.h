#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCharFormat>

#include "database.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

private:
    Ui::MainWindow *ui;

    QString oldText;    // 旧文本
    database *db;       // 数据库对象

    int timeLineRange       = 100;      // 时间轴长度
    bool isTimeLinePressedd = false;    // 当前是否按下时间轴
    bool isTimeLineing      = false;    // 当前是否在拖动时间轴
    int timeLineId          = -1;       // 时间轴操作序号
    int timeLineTempId;                 // 按下时间轴临时保存操作序号
    int timeLineMode        = 0;       // 时间轴操作模式
    QString tempText;                   // 按下时间轴时临时保存文本
    QList<HistoryRecord> beforeList;    // 时间轴之前的数据
    QList<HistoryRecord> afterList;     // 时间轴之后的数据

    QString title      = "记事本";    // 标题
    QString fontFamily = "Menlo";    // 字体
    int fontPointSize  = 13;         // 字号

public slots:
    void currentCharFormatChanged (const QTextCharFormat &f);
    void textChanged();
    void timeLinePressedd();
    void timeLineReleased();
    void timeLineValueChanged(int value);
    void lastTimeReturnPressed();
};
#endif // MAINWINDOW_H
