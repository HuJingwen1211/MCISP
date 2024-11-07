#include "lsc_dialog.h"
#include "ui_lsc_dialog.h"

lsc_dialog::lsc_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::lsc_dialog)
{
    ui->setupUi(this);
}

lsc_dialog::~lsc_dialog()
{
    delete ui;
}

int *lsc_dialog::get_R_Gain_Mat()
{
    int *R_Gain=new int[13*17];
    QString inputText = ui->R_Gain_Mat->toPlainText();
    QStringList inputList = inputText.split(",", Qt::SkipEmptyParts);
    for(int i=0;i<qMin(13*17,inputList.size());i++){
        bool conversionOK;
        int value = inputList.at(i).trimmed().toInt(&conversionOK);
        if (conversionOK) {
            R_Gain[i]=value;
        } else {
            R_Gain[i]=0;
        }
    }
    return R_Gain;
}

int *lsc_dialog::get_Gr_Gain_Mat()
{
    int *Gr_Gain=new int[13*17];
    QString inputText = ui->Gr_Gain_Mat->toPlainText();
    QStringList inputList = inputText.split(",", Qt::SkipEmptyParts);
    for(int i=0;i<qMin(13*17,inputList.size());i++){
        bool conversionOK;
        int value = inputList.at(i).trimmed().toInt(&conversionOK);
        if (conversionOK) {
            Gr_Gain[i]=value;
        } else {
            Gr_Gain[i]=0;
        }
    }
    return Gr_Gain;
}

int *lsc_dialog::get_Gb_Gain_Mat()
{
    int *Gb_Gain=new int[13*17];
    QString inputText = ui->Gb_Gain_Mat->toPlainText();
    QStringList inputList = inputText.split(",", Qt::SkipEmptyParts);
    for(int i=0;i<qMin(13*17,inputList.size());i++){
        bool conversionOK;
        int value = inputList.at(i).trimmed().toInt(&conversionOK);
        if (conversionOK) {
            Gb_Gain[i]=value;
        } else {
            Gb_Gain[i]=0;
        }
    }
    return Gb_Gain;
}

int *lsc_dialog::get_B_Gain_Mat()
{
    int *B_Gain=new int[13*17];
    QString inputText = ui->B_Gain_Mat->toPlainText();
    QStringList inputList = inputText.split(",", Qt::SkipEmptyParts);
    for(int i=0;i<qMin(13*17,inputList.size());i++){
        bool conversionOK;
        int value = inputList.at(i).trimmed().toInt(&conversionOK);
        if (conversionOK) {
            B_Gain[i]=value;
        } else {
            B_Gain[i]=0;
        }
    }
    return B_Gain;
}

void lsc_dialog::set_R_Gain_Mat(const int value[])
{
    QStringList arrayElements;
    for (int i = 0; i <13*17; i++) {
        arrayElements.append(QString::number(value[i]));
    }
    QString arrayText = arrayElements.join(", ");
    ui->R_Gain_Mat->setPlainText(arrayText);
}

void lsc_dialog::set_Gr_Gain_Mat(const int value[])
{
    QStringList arrayElements;
    for (int i = 0; i <13*17; i++) {
        arrayElements.append(QString::number(value[i]));
    }
    QString arrayText = arrayElements.join(", ");
    ui->Gr_Gain_Mat->setPlainText(arrayText);
}

void lsc_dialog::set_Gb_Gain_Mat(const int value[])
{
    QStringList arrayElements;
    for (int i = 0; i <13*17; i++) {
        arrayElements.append(QString::number(value[i]));
    }
    QString arrayText = arrayElements.join(", ");
    ui->Gb_Gain_Mat->setPlainText(arrayText);
}

void lsc_dialog::set_B_Gain_Mat(const int value[])
{
    QStringList arrayElements;
    for (int i = 0; i <13*17; i++) {
        arrayElements.append(QString::number(value[i]));
    }
    QString arrayText = arrayElements.join(", ");
    ui->B_Gain_Mat->setPlainText(arrayText);
}

