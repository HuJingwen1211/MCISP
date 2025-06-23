#ifndef TUNNING_TAB_H
#define TUNNING_TAB_H

#include <QObject>
#include <QMainWindow>
#include "isp_pipeline.h"
#include "my_graphicsview.h"
#include <QListWidget>
#include <QComboBox>
#include <QMessageBox>
#include <QThread>

#include "Dialog/dgain_dialog.h"
#include "Dialog/nlm_nr_dialog.h"
#include "Dialog/ccm_dialog.h"
#include "Dialog/sharpen_dialog.h"
#include "Dialog/dpc_dialog.h"
#include "Dialog/blc_dialog.h"
#include "Dialog/lsc_dialog.h"
#include "Dialog/nryuv_dialog.h"
#include "Dialog/gamma_dialog.h"
#include "Dialog/awb_manual_dialog.h"
#include "Dialog/csc_dialog.h"

typedef void (*FunctionPtr)(); ////函数指针

namespace Ui {
class Tunning_Tab;
}

class Tunning_Tab : public QMainWindow     ////public QMainWindow
{
    Q_OBJECT

public:
    explicit Tunning_Tab(QWidget *parent =  nullptr);
    ~Tunning_Tab();
    void init_modules_reg();
    void init_raw_image();
    void raw2clor(u16 *BAYER_DAT);
    void rgb2view(u16 *RGB_DATA[]);
    void yuv2view(u16 *YUV_DATA[]);

public:////////Module Clicked
    void DGain_Double_Clicked();
    void NR_RAW_Double_Clicked();
    void AWB_Double_Clicked(QComboBox *AWB_Method);
    void CCM_Double_Clicked();
    void CSC_Double_Clicked();
    void EE_Double_Clicked();
    void DPC_Double_Clicked();
    void BLC_Double_Clicked();
    void LSC_Double_Clicked();
    void NRYUV_Double_Clicked();
    void Gamma_Double_Clicked();
    void Item_check2reg();   ////运行前将用户的选择写入寄存器
    void pipeline_check_reset();
    void Run_Pipeline();
    int  open_with_click_init(QString filepath,int width,int height,int sensorbits,int bayerpattern);

private slots:
    void on_btn_raw_open_clicked();  ////打开raw图片

    void on_btn_update_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_btn_isp_run_clicked();

    void on_size_combox_currentIndexChanged(int index);

    void on_btn_imsave_clicked();

signals:
    void finished();

private:
    Ui::Tunning_Tab *ui;   ///布局对象
    ISP_Pipeline *isp;     /////isp对象
    My_GraphicsView *view; ///图像显示对象
    //QThread* workerThread;
};

#endif // TUNNING_TAB_H
