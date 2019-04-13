#ifndef PICDATA_H
#define PICDATA_H

#include <QColor>
class PicData
{
public:
    PicData();
    PicData(int width,int height);
    ~PicData();
    void setData(int i,int j,unsigned char d);
    unsigned char getData(int i,int j);
    void print();

    QColor getColor() const;
    void setColor(const QColor &color);

private:
    int mWidth;
    int mHeight;
    unsigned char* mData;
    QColor mColor;

};

#endif // PICDATA_H
