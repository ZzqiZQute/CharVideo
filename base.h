#ifndef BASE_H
#define BASE_H

#include <QObject>
#include <QProcess>
#include <QSize>
#include <QTimer>
class Base : public QObject
{
    Q_OBJECT
public:
    explicit Base(QObject *parent = nullptr);

    QProcess *process() const;
    void setProcess(QProcess *process);

    QByteArray buffer() const;


signals:

public slots:
    void onReadyReadStandardError();

private:
    int* mCompleteCnt;
    int* mTotalCnt;
    QProcess* mProcess;
    QSize mSize;
    double mFPS;
    QByteArray mBuffer;


};

#endif // BASE_H
