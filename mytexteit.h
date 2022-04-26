#ifndef MYTEXTEIT_H
#define MYTEXTEIT_H

#include <QTextEdit>
#include <QMimeData>
#include <QFileInfo>
#include <QImageReader>

class mytexteit : public QTextEdit
{
    Q_OBJECT
public:
    explicit mytexteit(QWidget *parent = nullptr);

    // bool canInsertFromMimeData(const QMimeData* source) const;
    // void insertFromMimeData(const QMimeData* source);

private:
    // void dropImage(const QUrl& url, const QImage& image);
    // void dropTextFile(const QUrl& url);

signals:

};

#endif // MYTEXTEIT_H
