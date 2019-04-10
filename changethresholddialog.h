#ifndef CHANGETHRESHOLDDIALOG_H
#define CHANGETHRESHOLDDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
namespace Ui {
class ChangeThresholdDialog;
}

class ChangeThresholdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeThresholdDialog(QWidget *parent,QStringList filePath);
    ~ChangeThresholdDialog();

    int threshold() const;

public slots:
    void onBtnLeftClick();
    void onBtnRightClick();
    void onSliderValueChanged();
    void onSliderPicValueChanged();

private:
    Ui::ChangeThresholdDialog *ui;
    QGraphicsScene *mScene;
    int mThreshold;
    QStringList mFilePath;
    int mIndex;
    int mCount;
    void drawPic();
};

#endif // CHANGETHRESHOLDDIALOG_H
