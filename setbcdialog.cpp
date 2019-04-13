#include "setbcdialog.h"
#include "ui_setbcdialog.h"
#include "picdata.h"
#include "converter.h"
SetBCDialog::SetBCDialog(QWidget *parent, QStringList filePath, int charWidth, int charHeight, int finalWidth, int finalHeight, int style, char* usedChar, int charcount, int *stretch, QList<QColor> *color) :
    QDialog(parent),ui(new Ui::SetBCDialog),mCharWidth(charWidth),mCharHeight(charHeight),mFinalWidth(finalWidth),mFinalHeight(finalHeight),mStyle(style),mUsedChar(usedChar),mCharCount(charcount),mStretch(stretch),mColor(color)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    mIndex=0;
    mFilePath=filePath;
    mCount=filePath.count();
    mScene=new QGraphicsScene;
    ui->graphicsView->setScene(mScene);
    ui->sliderPic->setMinimum(0);
    ui->sliderPic->setMaximum(mCount-1);
    mBrightness=50;
    mContrast=50;
    ui->slider->setValue(mBrightness);
    ui->slider2->setValue(mContrast);
    connect(ui->btnLeft,SIGNAL(clicked()),this,SLOT(onBtnLeftClick()));
    connect(ui->btnRight,SIGNAL(clicked()),this,SLOT(onBtnRightClick()));
    connect(ui->slider,SIGNAL(valueChanged(int)),this,SLOT(onSliderValueChanged()));
    connect(ui->slider2,SIGNAL(valueChanged(int)),this,SLOT(onSlider2ValueChanged()));
    connect(ui->sliderPic,SIGNAL(valueChanged(int)),this,SLOT(onSliderPicValueChanged()));
    drawPic();
}

SetBCDialog::~SetBCDialog()
{
    delete ui;
    delete mScene;
}

void SetBCDialog::onBtnLeftClick()
{
    if(mIndex>0){
        mIndex--;
        drawPic();
    }

}

void SetBCDialog::onBtnRightClick()
{

    if(mIndex<mCount-1){
        mIndex++;
        drawPic();
    }
}

void SetBCDialog::onSliderValueChanged()
{

    mBrightness=ui->slider->value();
    drawPic();
}

void SetBCDialog::onSlider2ValueChanged()
{
    mContrast=ui->slider2->value();
    drawPic();

}

void SetBCDialog::onSliderPicValueChanged()
{

    mIndex=ui->sliderPic->value();
    drawPic();
}

int SetBCDialog::contrast() const
{
    return mContrast;
}

void SetBCDialog::setContrast(int contrast)
{
    mContrast = contrast;
}

int SetBCDialog::brightness() const
{
    return mBrightness;
}

void SetBCDialog::setBrightness(int brightness)
{
    mBrightness = brightness;
}

void SetBCDialog::drawPic()
{
    ui->lbHint->setText(QString("第%1张/共%2张").arg(mIndex+1).arg(mCount));
    QByteArray charValue;
    QImage outImage=Converter::convert(mFilePath,mIndex,mCharWidth,mCharHeight,mFinalWidth,mFinalHeight,mStyle,mBrightness,mContrast,mUsedChar,mCharCount,mStretch,&charValue,mColor);
    mScene->clear();
    mScene->setBackgroundBrush(QBrush(Qt::lightGray));
    mScene->addPixmap(QPixmap::fromImage(outImage));
}
