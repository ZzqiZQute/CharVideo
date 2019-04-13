#include <QApplication>
#include <iostream>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include "reader.h"
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
#include "setbcdialog.h"
#include "converter.h"
#include <QSettings>
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
static int status=0;
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
        if(status==0)
            cout<<"\033[u\033[2K"<<*(reinterpret_cast<int*>(arg))<<".正在生成视频";
        else if(status==1)
            cout<<"\033[u\033[2K"<<*(reinterpret_cast<int*>(arg))<<".正在提取视频数据";
        else if(status==2)
            cout<<"\033[u\033[2K"<<*(reinterpret_cast<int*>(arg))<<".正在提取音频数据";
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
    Reader reader;
    int videoWidth;
    int videoHeight;
    int method;
    double videoFPS;
    int step=0;
    int finalWidth = 0;
    int finalHeight = 0;
    bool hasSound=true;
    bool colorVideo=false;
    bool colorTerminal=false;
    int completeCnt=0;
    char input[20];
    int count;
    int brightness;
    int contrast;
    QStringList arguments;
    QProcess ffmpeg;
    system("reset");
    cout<<endl;
    cout<<"\033[34m";
    cout<<"   ________              _    ___     __"<<endl;
    cout<<"  / ____/ /_  ____ _____| |  / (_)___/ /__  ____"<<endl;
    cout<<" / /   / __ \\/ __ `/ ___/ | / / / __  / _ \\/ __ \\"<<endl;
    cout<<"/ /___/ / / / /_/ / /   | |/ / / /_/ /  __/ /_/ /"<<endl;
    cout<<"\\____/_/ /_/\\__,_/_/    |___/_/\\__,_/\\___/\\____/"<<endl;
    cout<<"\033[0m";
    cout<<endl;
    cout<<"-------------Powered By \033[3m\033[92mQt\033[0m & \033[3m\033[91mFFMpeg\033[0m--------------"<<endl;
    cout<<endl;
    QFile ffmpegExe1("./ffmpeg");
    QFile ffmpegExe2("/usr/bin/ffmpeg");
    QFile ffmpegExe3("/usr/local/bin/ffmpeg");
    if(!ffmpegExe1.exists()&&!ffmpegExe2.exists()&&!ffmpegExe3.exists()){
        errorCode=ErrorCode::FFMPEGNOTFOUND;
        goto ERROR;
    }

    if(argc>=2&&argc<=3){
        QString outputFile="";
        if(argc==3){
            outputFile=QString(argv[2]);
        }
        QFile video(argv[1]);
        QDir currentDir;
        if(video.exists()){
            cout<<"输入文件:"<<video.fileName().toStdString()<<endl;
            if(outputFile!="")
                cout<<"输出文件:"<<outputFile.toStdString()<<endl;
            else
                cout<<"输出文件:"<<video.fileName().mid(0,video.fileName().lastIndexOf('.')).toStdString()+"_charvideo.mp4"<<endl;
            QDir tempDir(currentDir.path()+"/"+video.fileName()+"_tempdir");
            if(!tempDir.exists()){
                currentDir.mkdir(tempDir.path());
            }else{
                tempDir.removeRecursively();
                currentDir.mkdir(tempDir.path());
            }
            QDir txtDir(outputFile==""?video.fileName().mid(0,video.fileName().lastIndexOf('.'))+"_txt":outputFile.mid(0,outputFile.lastIndexOf('.'))+"_char");
            cout<<"输出txt及音频目录(灰度方法):"<<txtDir.absolutePath().toStdString()<<endl<<endl;
            if(!txtDir.exists()){
                currentDir.mkdir(txtDir.path());
            }else{
                txtDir.removeRecursively();
                currentDir.mkdir(txtDir.path());
            }
            cout<<++step<<".设置方法（0:灰度方法，1:二值化方法，默认0）:";
            fgets(input,20,stdin);
            size_t size=strlen(input)-1;
            QString temp=QString::fromLatin1(input,static_cast<int>(size));
            method=temp.toInt();if(method!=1)method=0;
            cout<<endl;
            if(1==method){
                currentPrintDot=0;
                status=1;
                cout<<"\033[s";
                pthread_t thread;
                pthread_create(&thread,nullptr,printThread,&++step);
                QProcess ffmpeg;
                reader.setProcess(&ffmpeg);
                reader.connect(&ffmpeg,SIGNAL(readyReadStandardError()),&reader,SLOT(onReadyReadStandardError()));
                arguments<<"-i";
                arguments<<currentDir.absoluteFilePath(video.fileName());
                arguments<<tempDir.absolutePath()+"/%09d.jpg";
                arguments<<"-y";
                ffmpeg.start("ffmpeg",arguments);
                ffmpeg.waitForFinished(-1);
                pthread_cancel(thread);
                QStringList filters;
                filters<<"*.jpg";
                QStringList jpgFile=tempDir.entryList(filters);
                count=jpgFile.count();
                QString msg(reader.buffer());
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
                reader.disconnect();
                currentPrintDot=0;
                status=2;
                cout<<"\033[s";
                pthread_create(&thread,nullptr,printThread,&++step);
                arguments.clear();
                arguments<<"-i";
                arguments<<currentDir.absoluteFilePath(video.fileName());
                arguments<<tempDir.absolutePath()+"/sound.aac";
                arguments<<"-y";
                ffmpeg.start("ffmpeg",arguments);
                ffmpeg.waitForFinished(-1);
                pthread_cancel(thread);
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
                cout<<++step<<".设置阈值(按回车键继续):";
                char newline[2];
                fgets(newline,2,stdin);
                ChangeThresholdDialog dialog(nullptr,jpgFilePath);
                int result=dialog.exec();
                if(result==QDialog::Rejected){
                    tempDir.removeRecursively();
                    errorCode=ErrorCode::USERCANCEL;
                    cout<<endl;
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
                    if(style==0)
                        painter.fillRect(outImage.rect(),Qt::white);
                    else {
                        painter.fillRect(outImage.rect(),Qt::black);
                    }
                    QFont font;
                    font.setFamily("Monospace");
                    font.setPointSize(charHeight-1);
                    painter.setFont(font);
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
                    outImage.save(QString("%1/char_%2.jpg").arg(tempDir.absolutePath()).arg(n,9,10,QChar('0')));
                    completeCnt++;
#pragma omp critical
                    cout<<"\033[u\033[2K"<<step<<".正在并行处理 已处理"<<completeCnt<<"帧/共"<<count<<"帧"<<endl;
                }
                cout<<endl;
                cout<<"\033[s";
                currentPrintDot=0;
                status=0;
                pthread_create(&thread,nullptr,printThread,&++step);
                arguments.clear();
                arguments<<QString("-s");
                arguments<<QString("%1x%2").arg(finalWidth).arg(finalHeight);
                arguments<<QString("-r");
                arguments<<QString("%1").arg(videoFPS);
                arguments<<"-i";
                arguments<<tempDir.absolutePath()+"/char_%9d.jpg";
                if(hasSound){
                    arguments<<"-i";
                    arguments<<tempDir.absolutePath()+"/sound.aac";
                }
                arguments<<"-pix_fmt";
                arguments<<"yuv420p";
                arguments<<currentDir.absolutePath()+"/"+video.fileName().mid(0,video.fileName().lastIndexOf('.'))+"_charvideo.mp4";
                arguments<<"-y";
                ffmpeg.start("ffmpeg",arguments);
                ffmpeg.waitForFinished(-1);
                pthread_cancel(thread);
                tempDir.removeRecursively();
                cout<<endl<<"完成,共计用时"<<time.msecsTo(QDateTime::currentDateTime())/1000.0<<"秒"<<endl<<endl;
                exit(0);
            }

            else if(method==0){
                reader.setProcess(&ffmpeg);
                reader.connect(&ffmpeg,SIGNAL(readyReadStandardError()),&reader,SLOT(onReadyReadStandardError()));
                arguments.clear();
                arguments<<"-i";
                arguments<<currentDir.absoluteFilePath(video.fileName());
                ffmpeg.start("ffmpeg",arguments);
                ffmpeg.waitForFinished(-1);
                QString msg(reader.buffer());
                QRegExp sizeReg("\\d{2,4}x\\d{2,4}");
                QRegExp fpsReg("\\d{1,4} fps(?=,)");
                QRegExp durationReg("\\d{2}:\\d{2}:\\d{2}.\\d{2}");
                int sizeIdx=sizeReg.indexIn(msg);
                int sizeLength=sizeReg.matchedLength();
                int fpsIdx=fpsReg.indexIn(msg);
                int fpsLength=fpsReg.matchedLength();
                int durationIdx=durationReg.indexIn(msg);
                int durationLength=durationReg.matchedLength();
                reader.disconnect();
                if(sizeIdx!=-1&&fpsIdx!=-1&&durationIdx!=-1){
                    QString temp=msg.mid(sizeIdx,sizeLength);
                    QStringList t=temp.split("x");
                    videoWidth=t.at(0).toInt();
                    videoHeight=t.at(1).toInt();
                    temp=msg.mid(fpsIdx,fpsLength);
                    t=temp.split(" ");
                    videoFPS=t.at(0).toDouble();
                    temp=msg.mid(durationIdx,durationLength);
                    t=temp.split(":");
                    int hour=t.at(0).toInt();
                    int minute=t.at(1).toInt();
                    QStringList t2= t.at(2).split(".");
                    int second=t2.at(0).toInt();
                    int milli=t2.at(1).toInt();
                    double totaltime=hour*3600+minute*60+second+milli/100.0;
                    cout<<"视频大小:"<<videoWidth<<"x"<<videoHeight<<" 帧率:"<<videoFPS<<"fps 视频长度:"<<totaltime<<"秒"<<endl<<endl;;
                    cout<<++step<<".输入字符高度(偶数，默认16):";
                    fgets(input,20,stdin);
                    size=strlen(input)-1;
                    temp=QString::fromLatin1(input,static_cast<int>(size));
                    int charHeight=temp.toInt();
                    if(charHeight==0)charHeight=16;
                    charHeight=charHeight/2*2;
                    int charWidth=charHeight/2;
                    finalWidth=videoWidth/charHeight*charHeight;
                    finalHeight=videoHeight/charHeight*charHeight;
                    int ww=finalWidth/charWidth;
                    int hh=finalHeight/charHeight;
                    cout<<"字符宽度:"<<charWidth<<" 字符高度:"<<charHeight<<" 输出宽度:"<<finalWidth<<" 输出高度:"<<finalHeight<<" 终端字符横向数量:"<<ww<<" 终端字符纵向数量:"<<hh<<endl<<endl;;
                    QFile playpy(":/script/playpy");
                    QFile playpyfile(txtDir.absolutePath()+"/play.py");
                    playpy.open(QFile::ReadOnly);
                    playpyfile.open(QFile::WriteOnly);
                    playpyfile.write(playpy.readAll());
                    playpy.close();
                    playpyfile.close();
                    system(std::string("chmod +x "+txtDir.absolutePath().toStdString()+"/play.py").c_str());
                    QList<QByteArray> charList;
                    cout<<++step<<".输入样式(0:白底，1:黑底，默认0):";
                    fgets(input,20,stdin);
                    size=strlen(input)-1;
                    temp=QString::fromLatin1(input,static_cast<int>(size));
                    int style=temp.toInt();
                    if(style!=1)style=0;
                    cout<<endl;
                    cout<<++step<<".输入使用字符个数(最小为2,最大为"<<sizeof(characters2)-1<<",默认15):";
                    fgets(input,20,stdin);
                    size=strlen(input)-1;
                    temp=QString::fromLatin1(input,static_cast<int>(size));
                    int charcount=temp.toInt();
                    if(charcount==0)charcount=15;
                    else if(charcount<2)charcount=2;
                    else if(charcount>static_cast<int>(sizeof(characters2)-1))charcount=static_cast<int>(sizeof(characters2)-1);
                    char *usedChar=new char[charcount];
                    for(size_t i=0;i<static_cast<size_t>(charcount);i++){
                        size_t t=(i+1)*sizeof(characters2)/(static_cast<size_t>(charcount+1));
                        if(t>=sizeof(characters2))t=sizeof(characters2)-1;
                        usedChar[i]=characters2[t];
                    }
                    QFont font;
                    font.setPixelSize(static_cast<int>(charHeight*0.85));
                    QFontMetrics fm(font);
                    int * stretch=new int[128];
                    for(int i=0;i<128;i++){
                        char c=static_cast<char>(i);
                        int width=fm.boundingRect(QChar(c)).width();
                        if(width>charWidth)
                            stretch[i]=charWidth*100/width;
                        else stretch[i]=100;
                    }
                    cout<<endl;
                    currentPrintDot=0;
                    status=1;
                    cout<<"\033[s";
                    pthread_t thread;
                    pthread_create(&thread,nullptr,printThread,&++step);
                    reader.setProcess(&ffmpeg);
                    reader.connect(&ffmpeg,SIGNAL(readyReadStandardError()),&reader,SLOT(onReadyReadStandardError()));
                    arguments.clear();
                    arguments<<"-i";
                    arguments<<currentDir.absoluteFilePath(video.fileName());
                    arguments<<tempDir.absolutePath()+"/%09d.jpg";
                    arguments<<"-y";
                    ffmpeg.start("ffmpeg",arguments);
                    ffmpeg.waitForFinished(-1);
                    QStringList filters;
                    filters<<"*.jpg";
                    QStringList jpgFile=tempDir.entryList(filters);
                    int count=jpgFile.count();
                    int ffmpegRes=ffmpeg.exitCode();
                    if(ffmpegRes!=0){
                        errorCode=ErrorCode::FILECORRUPT;
                        goto ERROR;
                    }
                    reader.disconnect();
                    pthread_cancel(thread);
                    cout<<endl;
                    currentPrintDot=0;
                    status=2;
                    cout<<"\033[s";
                    pthread_create(&thread,nullptr,printThread,&++step);
                    arguments.clear();
                    arguments<<"-i";
                    arguments<<currentDir.absoluteFilePath(video.fileName());
                    arguments<<tempDir.absolutePath()+"/sound.aac";
                    arguments<<"-y";
                    ffmpeg.start("ffmpeg",arguments);
                    ffmpeg.waitForFinished(-1);
                    ffmpegRes=ffmpeg.exitCode();
                    if(ffmpegRes!=0){
                        cout<<"\033[31m未正确提取出音频，将生成无音频字符视频\033[0m"<<endl<<endl;
                        hasSound=false;
                    }else{
                        QFile sound(tempDir.absolutePath()+"/sound.aac");
                        sound.copy(txtDir.absolutePath()+"/sound.aac");
                        cout<<endl;
                    }
                    pthread_cancel(thread);
                    QStringList jpgFilePath;
                    for(QString file:jpgFile){
                        jpgFilePath<<tempDir.absolutePath()+"/"+file;
                    }
                    QSettings settings(txtDir.absolutePath()+"/settings.conf",QSettings::NativeFormat);
                    settings.setValue("fps",videoFPS);
                    settings.setValue("width",ww);
                    settings.setValue("height",hh);
                    settings.sync();
                    cout<<++step<<".是否生成彩色视频(y/N):";
                    fgets(input,20,stdin);
                    size=strlen(input)-1;
                    temp=QString::fromLatin1(input,static_cast<int>(size));
                    if(temp=="y"||temp=="Y"){
                        colorVideo=true;
                    }
                    cout<<endl;
                    cout<<++step<<".设置亮度与对比度(按回车键继续):";
                    char newline[2];
                    fgets(newline,2,stdin);
                    QList<QColor> colors;
                    SetBCDialog *dialog;
                    if(colorVideo)
                        dialog=new SetBCDialog(nullptr,jpgFilePath,charWidth,charHeight,finalWidth,finalHeight,style,usedChar,charcount,stretch,&colors);
                    else
                        dialog=new SetBCDialog(nullptr,jpgFilePath,charWidth,charHeight,finalWidth,finalHeight,style,usedChar,charcount,stretch,nullptr);
                    int result=dialog->exec();
                    if(result==QDialog::Rejected){
                        tempDir.removeRecursively();
                        errorCode=ErrorCode::USERCANCEL;
                        goto ERROR;
                    }
                    brightness=dialog->brightness();
                    contrast=dialog->contrast();
                    delete dialog;
                    cout<<endl;

                    if(colorVideo){
                    cout<<++step<<".是否生成纯彩色控制台文本(y/N):";
                    fgets(input,20,stdin);
                    size=strlen(input)-1;
                    temp=QString::fromLatin1(input,static_cast<int>(size));
                    if(temp=="y"||temp=="Y"){
                        colorTerminal=true;
                    }
                    cout<<endl;
                    }
                    time=QDateTime::currentDateTime();
                    cout<<"\033[s";
                    ++step;
#pragma omp parallel for
                    for(int n=0;n<count;n++){
                        QByteArray charValue;
                        QList<QColor> color;
                        QImage outImage;
                        if(colorVideo)
                             outImage=Converter::convert(jpgFilePath,n,charWidth,charHeight,finalWidth,finalHeight,style,brightness,contrast,usedChar,charcount,stretch,&charValue,&color);
                        else
                             outImage=Converter::convert(jpgFilePath,n,charWidth,charHeight,finalWidth,finalHeight,style,brightness,contrast,usedChar,charcount,stretch,&charValue,nullptr);
                        outImage.save(QString("%1/char_%2.jpg").arg(tempDir.absolutePath()).arg(n,9,10,QChar('0')));
                        QString txtPath=QString("%1/char_%2.txt").arg(txtDir.absolutePath()).arg(n,9,10,QChar('0'));
                        QFile txtFile(txtPath);
                        if(txtFile.exists())txtFile.remove();
                        txtFile.open(QFile::WriteOnly);
                        QTextStream stream(&txtFile);
                        if(style==0)stream<<"\033[107m";
                        else if(style==1)stream<<"\033[40m";
                        if(!colorVideo)
                        for(int i=0;i<hh;i++){
                            stream<<charValue.mid(i*ww,ww);
                            stream<<endl;
                        }else{
                            int k=0;
                            for(int i=0;i<hh;i++){
                                for(int j=0;j<ww;j++){
                                    QColor c=color.at(k);
                                    if(colorTerminal)
                                    stream<<QString("\033[48;2;%1;%2;%3m%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(' ');
                                    else
                                    stream<<QString("\033[38;2;%1;%2;%3m%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(charValue.at(i*ww+j));
                                    k++;
                                }
                                stream<<endl;
                            }
                        }
                        stream<<"\033[0m";
                        txtFile.close();

                        completeCnt++;
#pragma omp critical
                        cout<<"\033[u\033[2K"<<step<<".正在并行处理 已处理"<<completeCnt<<"帧/共"<<count<<"帧"<<endl;
                    }
                    delete[] stretch;

                    cout<<"\033[u\033[2K"<<step<<".正在并行处理 已处理"<<count<<"帧/共"<<count<<"帧"<<endl;
                    cout<<endl;
                    delete[] usedChar;
                    cout<<"\033[s";
                    currentPrintDot=0;
                    status=0;
                    pthread_create(&thread,nullptr,printThread,&++step);
                    arguments.clear();
                    arguments<<QString("-s");
                    arguments<<QString("%1x%2").arg(finalWidth).arg(finalHeight);
                    arguments<<QString("-r");
                    arguments<<QString("%1").arg(videoFPS);
                    arguments<<"-i";
                    arguments<<tempDir.absolutePath()+"/char_%9d.jpg";
                    if(hasSound){
                        arguments<<"-i";
                        arguments<<tempDir.absolutePath()+"/sound.aac";
                    }
                    arguments<<"-pix_fmt";
                    arguments<<"yuv420p";
                    if(outputFile!="")
                        arguments<<currentDir.absolutePath()+"/"+outputFile;
                    else
                        arguments<<currentDir.absolutePath()+"/"+video.fileName().mid(0,video.fileName().lastIndexOf('.'))+"_charvideo.mp4";
                    arguments<<"-y";
                    ffmpeg.start("ffmpeg",arguments);
                    ffmpeg.waitForFinished(-1);
                    pthread_cancel(thread);
                }
                tempDir.removeRecursively();
                cout<<endl<<"完成,共计用时"<<time.msecsTo(QDateTime::currentDateTime())/1000.0<<"秒"<<endl<<endl;
                exit(0);
            }
        }else{
            errorCode=ErrorCode::FILENOTEXIST;
            goto ERROR;
        }}
    else{
        cout<<"用法: ./CharVideo <输入文件名> [输出文件名]"<<endl;
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
