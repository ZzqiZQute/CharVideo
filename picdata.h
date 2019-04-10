#ifndef PICDATA_H
#define PICDATA_H


class PicData
{
public:
    PicData();
    PicData(int width,int height);
    ~PicData();
    void setData(int i,int j,unsigned char d);
    unsigned char getData(int i,int j);
    void print();
private:
    int mWidth;
    int mHeight;
    unsigned char* mData;
};

#endif // PICDATA_H
