#include "reader.h"
#include <QDebug>
#include <iostream>
using namespace std;
Reader::Reader(QObject *parent) : QObject(parent)
{


}

QProcess *Reader::process() const
{
    return mProcess;
}

void Reader::setProcess(QProcess *process)
{
    mProcess = process;
}

void Reader::onReadyReadStandardError()
{
    mBuffer+=mProcess->readAllStandardError();
}

QByteArray Reader::buffer() const
{
    return mBuffer;
}
