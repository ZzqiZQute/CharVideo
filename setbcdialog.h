#ifndef SETBCDIALOG_H
#define SETBCDIALOG_H

//B for brightness
//C for contrast

#include <QDialog>
#include <QGraphicsView>

namespace Ui {
class SetBCDialog;
}

class SetBCDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetBCDialog(QWidget *parent,QStringList filePath,int charWidth,int charHeight,int finalWidth,int finalHeight,int style,char* usedChar,int charcount);
    ~SetBCDialog();

    int brightness() const;
    void setBrightness(int brightness);

    int contrast() const;
    void setContrast(int contrast);

public slots:
    void onBtnLeftClick();
    void onBtnRightClick();
    void onSliderValueChanged();
    void onSlider2ValueChanged();
    void onSliderPicValueChanged();

private:
    Ui::SetBCDialog *ui;
    QGraphicsScene *mScene;
    int mBrightness;
    int mContrast;
    QStringList mFilePath;
    int mIndex;
    int mCount;
    int mCharWidth;
    int mCharHeight;
    int mFinalWidth;
    int mFinalHeight;
    int mStyle;
    char* mUsedChar;
    int mCharCount;
    void drawPic();
};

#endif // SETBCDIALOG_H
