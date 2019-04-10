#include "picdata.h"
#include <iostream>
using namespace std;
PicData::PicData()
{
   mData=new unsigned char[0];
}

PicData::PicData(int width, int height)
    :mWidth(width),mHeight(height)
{
    mData=new unsigned char[width*height];
}

PicData::~PicData()
{
    delete mData;

}

void PicData::setData(int i, int j, unsigned char d)
{
    mData[j*mWidth+i]=d;
}

unsigned char PicData::getData(int i, int j)
{
    return mData[j*mWidth+i];
}

void PicData::print()
{
   for(int i=0;i<mHeight;i++){
       for(int j=0;j<mWidth;j++){
           cout<<((mData[j*mWidth+i]==0)?'0':'1')<<" ";
       }
   cout<<endl;
   }
}

