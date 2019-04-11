#ifndef CONVERTER_H
#define CONVERTER_H
#include <QImage>
#include <QStringList>

class Converter
{
public:
    Converter();
    static QImage convert(QStringList filePath, int index, int charWidth, int charHeight, int finalWidth, int finalHeight, int style, int brightness, int contrast, char*usedChar, int charcount);
};

#endif // CONVERTER_H
