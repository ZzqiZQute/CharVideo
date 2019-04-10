#include "changethresholddialog.h"
#include "ui_changethresholddialog.h"
#include <QDebug>
ChangeThresholdDialog::ChangeThresholdDialog(QWidget *parent, QStringList filePath) :
    QDialog(parent),
    ui(new Ui::ChangeThresholdDialog)
{
    ui->setupUi(this);
    mIndex=0;
    mFilePath=filePath;
    mCount=filePath.count();
    mScene=new QGraphicsScene;
    ui->graphicsView->setScene(mScene);
    mThreshold=100;
    ui->slider->setValue(mThreshold);
    ui->sliderPic->setMinimum(0);
    ui->sliderPic->setMaximum(mCount-1);
    connect(ui->btnLeft,SIGNAL(clicked()),this,SLOT(onBtnLeftClick()));
    connect(ui->btnRight,SIGNAL(clicked()),this,SLOT(onBtnRightClick()));
    connect(ui->slider,SIGNAL(valueChanged(int)),this,SLOT(onSliderValueChanged()));
    connect(ui->sliderPic,SIGNAL(valueChanged(int)),this,SLOT(onSliderPicValueChanged()));
    drawPic();

}

ChangeThresholdDialog::~ChangeThresholdDialog()
{
    delete ui;
    delete mScene;
}

void ChangeThresholdDialog::onBtnLeftClick()
{
    if(mIndex>0){
        mIndex--;
        drawPic();
    }
}

void ChangeThresholdDialog::onBtnRightClick()
{
    if(mIndex<mCount-1){
        mIndex++;
        drawPic();
    }
}

void ChangeThresholdDialog::onSliderValueChanged()
{
    mThreshold=ui->slider->value();
    drawPic();

}

void ChangeThresholdDialog::onSliderPicValueChanged()
{
    mIndex=ui->sliderPic->value();
    drawPic();

}

void ChangeThresholdDialog::drawPic()
{
    ui->lbHint->setText(QString("第%1张/共%2张").arg(mIndex+1).arg(mCount));
    QPixmap pixmap(mFilePath[mIndex]);
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
            if(gray>mThreshold)
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
    mScene->clear();
    mScene->setBackgroundBrush(QBrush(Qt::lightGray));
    mScene->addPixmap(QPixmap::fromImage(twoValueImage));

}

int ChangeThresholdDialog::threshold() const
{
    return mThreshold;
}
