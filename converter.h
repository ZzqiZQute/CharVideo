#ifndef CONVERTER_H
#define CONVERTER_H
#include <QImage>
#include <QStringList>
#include <QDir>
class Converter
{
public:
    Converter();
    static QImage convert(QStringList filePath, int index, int charWidth, int charHeight, int finalWidth, int finalHeight, int style, int brightness, int contrast, char*usedChar, int charcount,int* stretch,QByteArray* value);
};

#endif // CONVERTER_H
