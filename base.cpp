#include "base.h"
#include <QDebug>
#include <iostream>
using namespace std;
Base::Base(QObject *parent) : QObject(parent)
{


}

QProcess *Base::process() const
{
    return mProcess;
}

void Base::setProcess(QProcess *process)
{
    mProcess = process;
}

void Base::onReadyReadStandardError()
{
    mBuffer+=mProcess->readAllStandardError();
}

QByteArray Base::buffer() const
{
    return mBuffer;
}
