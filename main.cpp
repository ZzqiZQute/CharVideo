#include <QApplication>
#include <iostream>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include "base.h"
#include "changethresholddialog.h"
#include "picdata.h"
#include "common.h"
#include <omp.h>
#include <QImage>
#include <QPainter>
#include <stdarg.h>
#include <QDateTime>
#include <pthread.h>
#include <unistd.h>
using namespace std;
static enum ErrorCode{
    OK,
    FFMPEGNOTFOUND,
    FILENOTEXIST,
    FILECORRUPT,
    FILENOTSELECT,
    USERCANCEL

}errorCode;
static int currentPrintDot=0;
unsigned char average(int cnt,...){
    int ret=0;
    va_list ap;
    va_start(ap,cnt);
    for(int i=0;i<cnt;i++){
        unsigned char t=static_cast<unsigned char>(va_arg(ap,int));
        ret+=t;
    }
    va_end(ap);
    return static_cast<unsigned char>(ret/cnt);


}
void *printThread(void* arg){
    while(true){
        cout<<"\033[u\033[2K"<<*(reinterpret_cast<int*>(arg))<<".正在生成视频";
        for(int i=0;i<currentPrintDot;i++)cout<<".";
        cout<<endl;
        currentPrintDot++;
        if(currentPrintDot>6)currentPrintDot=0;
        sleep(1);
    }

}
unsigned char minValue(int cnt,...){
    unsigned char ret=255;
    va_list ap;
    va_start(ap,cnt);
    unsigned char t=static_cast<unsigned char>(va_arg(ap,int));
    ret=t;
    for(int i=1;i<cnt;i++){
        unsigned char t=static_cast<unsigned char>(va_arg(ap,int));
        if(t>ret)ret=t;
    }
    va_end(ap);
    return ret;


}
unsigned char maxValue(int cnt,...){
    unsigned char ret=0;
    va_list ap;
    va_start(ap,cnt);
    unsigned char t=static_cast<unsigned char>(va_arg(ap,int));
    ret=t;
    for(int i=1;i<cnt;i++){
        unsigned char t=static_cast<unsigned char>(va_arg(ap,int));
        if(t<ret)ret=t;
    }
    va_end(ap);
    return ret;
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDateTime time;
    Base base;
    int videoWidth;
    int videoHeight;
    int method;
    double videoFPS;
    int step=0;
    int finalWidth = 0;
    int finalHeight = 0;
    bool hasSound=true;
    int completeCnt=0;
    char input[20];
    system("reset");
    cout<<endl;
    cout<<"   ________              _    ___     __"<<endl;
    cout<<"  / ____/ /_  ____ _____| |  / (_)___/ /__  ____"<<endl;
    cout<<" / /   / __ \\/ __ `/ ___/ | / / / __  / _ \\/ __ \\"<<endl;
    cout<<"/ /___/ / / / /_/ / /   | |/ / / /_/ /  __/ /_/ /"<<endl;
    cout<<"\\____/_/ /_/\\__,_/_/    |___/_/\\__,_/\\___/\\____/"<<endl;
    cout<<endl;
    cout<<"----------------Using \033[3m\033[92mQt\033[0m & \033[3m\033[91mFFMpeg\033[0m-----------------"<<endl;
    cout<<endl;
    cout<<"\033[33m注意:生成的视频为当前目录的output.mp4\n请检查当前目录是否存在output.mp4文件,如果有请移至别的文件夹\033[0m"<<endl;
    cout<<endl;
    QFile ffmpegExe1("./ffmpeg");
    QFile ffmpegExe2("/usr/bin/ffmpeg");
    QFile ffmpegExe3("/usr/local/bin/ffmpeg");
    if(!ffmpegExe1.exists()&&!ffmpegExe2.exists()&&!ffmpegExe3.exists()){
       errorCode=ErrorCode::FFMPEGNOTFOUND;
       goto ERROR;
    }
    if(argc==2){
        QFile video(argv[1]);
        QDir currentDir;
        QDir tempDir(currentDir.path()+"/.charvideotempdir");
        if(!tempDir.exists()){
            currentDir.mkdir(tempDir.path());
        }else{
            tempDir.removeRecursively();
            currentDir.mkdir(tempDir.path());
        }
        if(video.exists()){
            cout<<++step<<".正在提取视频数据"<<endl;
            QProcess ffmpeg;
            QStringList arguments;
            base.setProcess(&ffmpeg);
            base.connect(&ffmpeg,SIGNAL(readyReadStandardError()),&base,SLOT(onReadyReadStandardError()));
            arguments<<"-i";
            arguments<<currentDir.absoluteFilePath(video.fileName());
            arguments<<tempDir.absolutePath()+"/%06d.jpg";
            arguments<<"-y";
            ffmpeg.start("ffmpeg",arguments);
            ffmpeg.waitForFinished();
            QStringList filters;
            filters<<"*.jpg";
            QStringList jpgFile=tempDir.entryList(filters);
            int count=jpgFile.count();
            QString msg(base.buffer());
            QRegExp sizeReg("\\d{2,4}x\\d{2,4}");
            QRegExp fpsReg("\\d{1,4} fps(?=,)");
            int sizeIdx=sizeReg.indexIn(msg);
            int sizeLength=sizeReg.matchedLength();
            int fpsIdx=fpsReg.indexIn(msg);
            int fpsLength=fpsReg.matchedLength();
            if(sizeIdx!=-1&&fpsIdx!=-1){
                QString temp=msg.mid(sizeIdx,sizeLength);
                QStringList t=temp.split("x");
                videoWidth=t.at(0).toInt();
                videoHeight=t.at(1).toInt();
                temp=msg.mid(fpsIdx,fpsLength);
                t=temp.split(" ");
                videoFPS=t.at(0).toDouble();
                cout<<"视频大小:"<<videoWidth<<"x"<<videoHeight<<" 帧数:"<<count<<"帧 帧率:"<<videoFPS<<"fps 视频长度:"<<count/videoFPS<<"秒"<<endl<<endl;;
            }else{
                errorCode=ErrorCode::FILECORRUPT;
                goto ERROR;
            }
            int ffmpegRes=ffmpeg.exitCode();
            if(ffmpegRes!=0){
                errorCode=ErrorCode::FILECORRUPT;
                goto ERROR;
            }
            base.disconnect();
            cout<<++step<<".正在提取音频数据"<<endl;
            arguments.clear();
            arguments<<"-i";
            arguments<<currentDir.absoluteFilePath(video.fileName());
            arguments<<tempDir.absolutePath()+"/sound.wav";
            arguments<<"-y";
            ffmpeg.start("ffmpeg",arguments);
            ffmpeg.waitForFinished();
            ffmpegRes=ffmpeg.exitCode();
            if(ffmpegRes!=0){
                cout<<"\033[31m未正确提取出音频，将生成无音频字符视频\033[0m"<<endl<<endl;;
                hasSound=false;
            }else{
                cout<<endl;
            }
            QStringList jpgFilePath;
            for(QString file:jpgFile){
                jpgFilePath<<tempDir.absolutePath()+"/"+file;
            }

            cout<<++step<<".设置方法（0:灰度方法，1:二值化方法，默认0）:";
            fgets(input,20,stdin);
            size_t size=strlen(input)-1;
            QString temp=QString::fromLatin1(input,static_cast<int>(size));
            method=temp.toInt();if(method!=1)method=0;
            cout<<endl;
            if(1==method){
                cout<<++step<<".设置阈值(按回车键继续):";
                char newline[2];
                fgets(newline,2,stdin);
                ChangeThresholdDialog dialog(nullptr,jpgFilePath);
                int result=dialog.exec();
                if(result==QDialog::Rejected){
                    errorCode=ErrorCode::USERCANCEL;
                    goto ERROR;
                }
                int threshold=dialog.threshold();
                cout<<"阈值设置为:"<<threshold<<endl<<endl;;
                cout<<++step<<".输入字符高度(4的倍数,最小16，默认16):";
                fgets(input,20,stdin);
                size=strlen(input)-1;
                temp=QString::fromLatin1(input,static_cast<int>(size));
                int charHeight=temp.toInt();
                if(charHeight<16)charHeight=16;
                else charHeight=charHeight/4*4;
                int charWidth=charHeight/2;
                finalWidth=videoWidth/charHeight*charHeight;
                finalHeight=videoHeight/charHeight*charHeight;
                cout<<"字符宽度:"<<charWidth<<" 字符高度:"<<charHeight<<" 输出宽度:"<<finalWidth<<" 输出高度:"<<finalHeight<<endl<<endl;;
                QList<QByteArray> charList;
                cout<<++step<<".输入样式(0:白底黑字，1:黑底白字，默认0):";
                fgets(input,20,stdin);
                size=strlen(input)-1;
                temp=QString::fromLatin1(input,static_cast<int>(size));
                int style=temp.toInt();
                if(style!=1)style=0;
                time=QDateTime::currentDateTime();
                cout<<endl<<"\033[s";
                ++step;
#pragma omp parallel for
                for(int n=0;n<count;n++){
                    //提取二值化图片
                    QPixmap pixmap(jpgFilePath[n]);
                    QImage image=pixmap.toImage();
                    QImage twoValueImage(image.width(),image.height(),QImage::Format_Mono);
                    int width=image.width();
                    int height=image.height();
                    for(int i=0;i<height;i++){
                        int k=0;
                        int m=0;
                        unsigned char* data=image.scanLine(i);
                        unsigned char* data2=twoValueImage.scanLine(i);
                        for(int j=0;j<width;j++){
                            int red=*(data+4*j);
                            int green=*(data+4*j+1);
                            int blue=*(data+4*j+2);
                            int gray= (red*30 +green*59 +blue*11 + 50) / 100;
                            if(gray>threshold)
                                *(data2+k)|=1<<(7-m);
                            else {
                                *(data2+k)&=~(1<<(7-m));
                            }
                            m++;
                            if(m>7){
                                m=0;
                                k++;
                            }
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
                                unsigned char* picdata=twoValueImage.scanLine(j*charHeight+a);
                                for(int b=0;b<charWidth;b++)
                                {
                                    int pass=i*charWidth+b;
                                    int offset=pass/8;
                                    int bit=pass%8;
                                    pic->setData(b,a,((*(picdata+offset))&(1<<(7-bit)))==0?0:1);
                                }
                            }
                            data<<pic;
                        }

                    //插值
                    QList<PicData*> interpolate;
                    if(charHeight!=16){
                        while(data.count()>0){
                            PicData* d=data.takeFirst();
                            PicData* d2=new PicData(8,16);
                            for(int i=0;i<16;i++){
                                for(int j=0;j<8;j++){
                                    double x=charWidth*j/8.0;
                                    double y=charHeight*i/16.0;
                                    int xx=static_cast<int>(x);
                                    int yy=static_cast<int>(y);
                                    if(xx+1<charWidth&&yy+1<charHeight){
                                        unsigned char dd1=d->getData(xx,yy);
                                        unsigned char dd2=d->getData(xx+1,yy);
                                        unsigned char dd3=d->getData(xx,yy+1);
                                        unsigned char dd4=d->getData(xx+1,yy+1);
                                        unsigned char v=static_cast<unsigned char>(dd1*(x-xx)*(y-yy)+dd2*(xx+1-x)*(y-yy)+dd3*(x-xx)*(yy+1-y)+dd4*(xx+1-x)*(yy+1-y));
                                        d2->setData(j,i,v);
                                    }
                                }
                            }
                            interpolate<<d2;
                            delete d;
                        }
                        data=interpolate;
                    }
                    QByteArray list;
                    //匹配
                    for(int i=0;i<data.length();i++){
                        PicData* d=data.at(i);
                        int max=-1;
                        int max_idx=-1;
                        for(int r=0;r<95;r++){
                            int sum=0;
                            const unsigned char *l=code[r];
                            for(int j=0;j<16;j++){
                                for(int k=0;k<8;k++){
                                    if(((l[j]&(1<<k))>0&&d->getData(k,j)==0)||((l[j]&(1<<k))==0&&d->getData(k,j)>0)){
                                        sum++;
                                    }
                                }
                            }
                            if(sum>max){
                                max=sum;
                                max_idx=r;
                            }

                        }
                        list.append(characters[max_idx]);
                    }
                    QImage outImage(finalWidth,finalHeight,QImage::Format_Grayscale8);
                    QPainter painter(&outImage);
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    if(style==0)
                        painter.fillRect(outImage.rect(),Qt::white);
                    else {
                        painter.fillRect(outImage.rect(),Qt::black);
                    }
                    QFont font;
                    font.setPointSize(charWidth);
                    painter.setFont(font);
                    if(style!=0){
                        painter.setPen(QPen(Qt::white));
                    }
                    int k=0;
                    for(int i=0;i<hh;i++)
                        for(int j=0;j<ww;j++){
                            painter.drawText(j*charWidth,(i+1)*charHeight,QString(list.at(k)));
                            k++;
                        }
                    outImage.save(QString("%1/char_%2.jpg").arg(tempDir.absolutePath()).arg(n,6,10,QChar('0')));
                    completeCnt++;
#pragma omp critical
                    cout<<"\033[u\033[2K"<<step<<".正在并行处理 已处理"<<completeCnt<<"帧/共"<<count<<"帧"<<endl;
                }
            }else if(method==0){

                cout<<++step<<".输入字符高度(4的倍数，默认16):";
                fgets(input,20,stdin);
                size=strlen(input)-1;
                temp=QString::fromLatin1(input,static_cast<int>(size));
                int charHeight=temp.toInt();
                if(charHeight==0)charHeight=16;
                else if(charHeight<4)charHeight=4;
                else charHeight=charHeight/4*4;
                int charWidth=charHeight/2;
                finalWidth=videoWidth/charHeight*charHeight;
                finalHeight=videoHeight/charHeight*charHeight;
                cout<<"字符宽度:"<<charWidth<<" 字符高度:"<<charHeight<<" 输出宽度:"<<finalWidth<<" 输出高度:"<<finalHeight<<endl<<endl;;
                QList<QByteArray> charList;
                cout<<++step<<".输入样式(0:白底黑字，1:黑底白字，默认0):";
                fgets(input,20,stdin);
                size=strlen(input)-1;
                temp=QString::fromLatin1(input,static_cast<int>(size));
                int style=temp.toInt();
                if(style!=1)style=0;
                time=QDateTime::currentDateTime();
                cout<<endl<<"\033[s";
                ++step;
#pragma omp parallel for
                for(int n=0;n<count;n++){
                    //提取灰度图片
                    QPixmap pixmap(jpgFilePath[n]);
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

                    //膨胀处理
                    for(int k=0;k<FILTER_COUNT;k++){
                        unsigned char* oridata=grayImage.bits();
                        unsigned char* tempdata=new unsigned char[width*height];
                        int bpl=grayImage.bytesPerLine();
                        for(int i=1;i<height-1;i++)
                            for(int j=1;j<width-1;j++){
                                unsigned char t=maxValue(9,
                                                         oridata[(i-1)*bpl+j-1],
                                                oridata[(i)*bpl+j-1],
                                                oridata[(i+1)*bpl+j-1],
                                                oridata[(i-1)*bpl+j],
                                                oridata[(i)*bpl+j],
                                                oridata[(i+1)*bpl+j],
                                                oridata[(i-1)*bpl+j+1],
                                                oridata[(i)*bpl+j+1],
                                                oridata[(i+1)*bpl+j+1]
                                                );
                                tempdata[i*bpl+j]=t;
                            }
                        //memset(oridata,0,static_cast<size_t>(width*height));
                        memcpy(oridata,tempdata,static_cast<size_t>(width*height));
                        delete[] tempdata;
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
                        int s=static_cast<int>(sizeof(characters3))*t/256;

                        list.append(static_cast<char>(characters3[s]));
                    }
                    QImage outImage(finalWidth,finalHeight,QImage::Format_Grayscale8);
                    QPainter painter(&outImage);
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    if(style==0)
                        painter.fillRect(outImage.rect(),Qt::white);
                    else {
                        painter.fillRect(outImage.rect(),Qt::black);
                    }
                    QFont font;
                    font.setPointSize(charWidth);
                    painter.setFont(font);
                    if(style!=0){
                        painter.setPen(QPen(Qt::white));
                    }
                    int k=0;
                    for(int i=0;i<hh;i++)
                        for(int j=0;j<ww;j++){
                            painter.drawText(j*charWidth,(i+1)*charHeight,QString(list.at(k)));
                            k++;
                        }
                    outImage.save(QString("%1/char_%2.jpg").arg(tempDir.absolutePath()).arg(n,6,10,QChar('0')));
                    completeCnt++;
#pragma omp critical
                    cout<<"\033[u\033[2K"<<step<<".正在并行处理 已处理"<<completeCnt<<"帧/共"<<count<<"帧"<<endl;
                }
            }
            cout<<"\033[u\033[2K"<<step<<".正在并行处理 已处理"<<count<<"帧/共"<<count<<"帧"<<endl;
            cout<<endl;
            cout<<"\033[s";
            pthread_t thread;
            pthread_create(&thread,nullptr,printThread,&++step);
            arguments.clear();
            arguments<<QString("-s");
            arguments<<QString("%1x%2").arg(finalWidth).arg(finalHeight);
            arguments<<QString("-r");
            arguments<<QString("%1").arg(videoFPS);
            arguments<<"-i";
            arguments<<tempDir.absolutePath()+"/char_%6d.jpg";
            if(hasSound){
                arguments<<"-i";
                arguments<<tempDir.absolutePath()+"/sound.wav";
            }
            arguments<<"-pix_fmt";
            arguments<<"yuv420p";
            arguments<<currentDir.absolutePath()+"/output.mp4";
            arguments<<"-y";
            ffmpeg.start("ffmpeg",arguments);
            ffmpeg.waitForFinished();
            pthread_cancel(thread);
        }else{
            errorCode=ErrorCode::FILENOTEXIST;
            goto ERROR;
        }
        tempDir.removeRecursively();
        cout<<endl<<"完成,共计用时"<<time.msecsTo(QDateTime::currentDateTime())/1000.0<<"秒"<<endl<<endl;
        exit(0);
    }else{
        cout<<"用法: ./CharVideo <文件名>"<<endl;
        cout<<endl;
        errorCode=FILENOTSELECT;
    }

ERROR:
    cerr<<"\033[31m操作失败:";
    switch(errorCode){
    case ErrorCode::FFMPEGNOTFOUND:
        cerr<<"未找到FFMpeg可执行文件，请安装FFMpeg或将可执行文件目录放入环境变量"<<endl;
        break;
    case ErrorCode::FILENOTEXIST:
        cerr<<"文件不存在"<<endl;
        break;
    case ErrorCode::FILECORRUPT:
        cerr<<"文件已损坏"<<endl;
        break;
    case ErrorCode::USERCANCEL:
        cerr<<"用户取消了操作"<<endl;
        break;
    case ErrorCode::FILENOTSELECT:
        cerr<<"用户未选择文件"<<endl;
        break;
    default:
        break;

    }
    cerr<<"\033[0m"<<endl;
    exit(1);
}
