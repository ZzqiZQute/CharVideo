#include "converter.h"
#include "picdata.h"
#include "common.h"
#include <QPainter>
#include <QFont>
#include <QPixmap>
Converter::Converter()
{

}

QImage Converter::convert(QStringList filePath, int index, int charWidth, int charHeight, int finalWidth, int finalHeight, int style, int brightness, int contrast, char *usedChar,int charcount,int* stretch,QByteArray* value)
{
    QPixmap pixmap(filePath[index]);
    QImage image=pixmap.toImage();
    int width=image.width();
    int height=image.height();
//    //图像增强
    QImage enhancedImage(image.width(),image.height(),QImage::Format_ARGB32);
    for(int i=1;i<height-1;i++){
        unsigned char* data=image.scanLine(i-1);
        unsigned char* data1=image.scanLine(i);
        unsigned char* data11=image.scanLine(i+1);
        unsigned char* data2=enhancedImage.scanLine(i);
        for(int j=1;j<width-1;j++){
            int red=5**(data1+4*j);
            red-=*(data+4*j);
            red-=*(data11+4*j);
            red-=*(data1+4*(j-1));
            red-=*(data1+4*(j+1));
            if(red>255)red=255;
            else if(red<0)red=0;
            int green=5**(data1+4*j+1);
            green-=*(data+4*j+1);
            green-=*(data11+4*j+1);
            green-=*(data1+4*(j-1)+1);
            green-=*(data1+4*(j+1)+1);
            if(green>255)green=255;
            else if(green<0)green=0;
            int blue=5**(data1+4*j+2);
            blue-=*(data+4*j+2);
            blue-=*(data11+4*j+2);
            blue-=*(data1+4*(j-1)+2);
            blue-=*(data1+4*(j+1)+2);
            if(blue>255)blue=255;
            else if(blue<0)blue=0;
            *(data2+4*j)=static_cast<unsigned char>(red);
            *(data2+4*j+1)=static_cast<unsigned char>(green);
            *(data2+4*j+2)=static_cast<unsigned char>(blue);
        }
    }


    QImage grayImage(image.width(),image.height(),QImage::Format_Grayscale8);
    for(int i=0;i<height;i++){
        unsigned char* data=enhancedImage.scanLine(i);
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
    //HE
//    int prob[256]={0};
//    int total=0;
//    for(int i=0;i<height;i++){
//        unsigned char* line=grayImage.scanLine(i);
//        for(int j=0;j<width;j++){
//            prob[line[j]]++;
//            total++;
//        }
//    }
//    double prob2[256]={0};
//    prob2[0]=static_cast<double>(prob[0])/total;
//    for(int i=1;i<256;i++){
//        prob2[i]=prob2[i-1]+static_cast<double>(prob[i])/total;
//    }
//    for(int i=0;i<height;i++){
//        unsigned char* line=grayImage.scanLine(i);
//        for(int j=0;j<width;j++){
//            line[j]=static_cast<unsigned char>(0.5+255*prob2[line[j]]);
//        }
//    }
//    return grayImage;
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
            int tmp=static_cast<int>(grayavg*getValue(0.9,1.1,brightness)+getValue(0.5,1.5,contrast)*(oridata[i*width+j]-grayavg));
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

    //匹配
    value->clear();
    for(int i=0;i<data.length();i++){
        PicData* d=data.at(i);
        int t=0;
        for(int j=0;j<charWidth*charHeight;j++){
            t+=d->getData(j/charHeight,j%charHeight);
        }
        t=t/(charWidth*charHeight);
        char s=usedChar[t*charcount/256];
        value->append(s);
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
            char c=value->at(k);
            font.setStretch(stretch[static_cast<int>(c)]);
            painter.setFont(font);
            painter.drawText(QRect(j*charWidth,i*charHeight,charWidth,charHeight),Qt::AlignCenter,QString(c));
            k++;
        }
    return outImage;
}
