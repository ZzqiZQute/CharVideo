#include "converter.h"
#include "picdata.h"
#include "common.h"
#include <QPainter>
#include <QFont>
#include <QPixmap>
Converter::Converter()
{

}

QImage Converter::convert(QStringList filePath, int index, int charWidth, int charHeight, int finalWidth, int finalHeight, int style, int brightness, int contrast, char *usedChar,int charcount)
{
    QPixmap pixmap(filePath[index]);
    QImage image=pixmap.toImage();
    QImage grayImage(image.width(),image.height(),QImage::Format_Grayscale8);
    int width=image.width();
    int height=image.height();
    for(int i=0;i<height;i++){
        unsigned char* data=image.scanLine(i);
        unsigned char* data2=grayImage.scanLine(i);
        for(int j=0;j<width;j++){
            int red=*(data+4*j);
            int green=*(data+4*j+1);
            int blue=*(data+4*j+2);
            int gray= (red*30 +green*59 +blue*11 + 50) / 100;
            *(data2+j)=static_cast<unsigned char>(gray);
        }
    }


    unsigned char* oridata=grayImage.bits();
    unsigned long graysum=0;
    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++)
        {
            graysum+=oridata[i*width+j];
        }
    }
    int grayavg=static_cast<int>(graysum/static_cast<unsigned long>(height*width));

    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++)
        {
            int tmp=static_cast<int>(grayavg*getValue(0.5,1.5,brightness)+getValue(0.5,1.5,contrast)*(oridata[i*width+j]-grayavg));
            if(tmp>255)tmp=255;
            else if(tmp<0)tmp=0;
            oridata[i*width+j]=static_cast<unsigned char>(tmp);

        }
    }

    //按字符大小裁剪窗口
    int ww=finalWidth/charWidth;
    int hh=finalHeight/charHeight;
    QList<PicData*> data;
    for(int j=0;j<hh;j++)
        for(int i=0;i<ww;i++)
        {
            PicData* pic=new PicData(charWidth,charHeight);
            for(int a=0;a<charHeight;a++)
            {
                unsigned char* picdata=grayImage.scanLine(j*charHeight+a);
                for(int b=0;b<charWidth;b++)
                {
                    int pass=i*charWidth+b;

                    pic->setData(b,a,*(picdata+pass));
                }
            }
            data<<pic;
        }

    QByteArray list;
    //匹配
    for(int i=0;i<data.length();i++){
        PicData* d=data.at(i);
        int t=0;
        for(int j=0;j<charWidth*charHeight;j++){
            t+=d->getData(j/charHeight,j%charHeight);
        }
        t=t/(charWidth*charHeight);
        char s=usedChar[t*charcount/256];
        list.append(s);
    }
    QImage outImage(finalWidth,finalHeight,QImage::Format_Grayscale8);
    QPainter painter(&outImage);
    if(style==0)
        painter.fillRect(outImage.rect(),Qt::white);
    else {
        painter.fillRect(outImage.rect(),Qt::black);
    }
    QFont font;
    font.setPixelSize(static_cast<int>(charHeight*0.85));
    QFontMetrics fm(font);
    painter.setFont(font);
    if(style!=0){
        painter.setPen(QPen(Qt::white));
    }
    int k=0;
    for(int i=0;i<hh;i++)
        for(int j=0;j<ww;j++){
            char c=list.at(k);
            int width=fm.boundingRect(QChar(c)).width();
            if(width>charWidth)
                font.setStretch(charWidth*100/width);
            else font.setStretch(100);
            painter.setFont(font);
            painter.drawText(QRect(j*charWidth,i*charHeight,charWidth,charHeight),Qt::AlignCenter,QString(c));
            k++;
        }
    return outImage;
}
