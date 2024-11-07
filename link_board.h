#ifndef LINK_BOARD_H
#define LINK_BOARD_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTreeWidget>
#include "Link_tab/test_tab.h"
#include "Link_tab/dpc_tab.h"
#include "Link_tab/enable_tab.h"
#include "Link_tab/blc_tab.h"
#include "Link_tab/lsc_tab.h"
#include "Link_tab/nr_raw_tab.h"
#include "Link_tab/awb_tab.h"
#include "Link_tab/dms_tab.h"
#include "Link_tab/ccm_tab.h"
#include "Link_tab/ee_tab.h"
#include "Link_tab/tm_tab.h"
#include "Link_tab/gamma_tab.h"
#include "Link_tab/csc_tab.h"
#include "Link_tab/nr_yuv_tab.h"
#include "Link_tab/gb_tab.h"
#include "Link_tab/scale_tab.h"
#include "Link_tab/crop_tab.h"
#include "Link_tab/yfc_tab.h"

namespace Ui {
class link_board;
}

class link_board : public QMainWindow
{
    Q_OBJECT

public:
    explicit link_board(QWidget *parent = 0);
    ~link_board();
    void Read_Data();
    int  Write_Data(QByteArray transdata);

private slots:
    void on_clear_btn_clicked();

    void on_link_btn_clicked();

    void on_module_list_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_link_tab_tabCloseRequested(int index);

public:
    void set_echo_text(QString str);
    void TEST_DoubleClicked();
    void DPC_DoubleClicked();
    void ENABLE_DoubleClicked();
    void BLC_DoubleClicked();
    void LSC_DoubleClicked();
    void NR_RAW_DoubleClicked();
    void AWB_DoubleClicked();
    void GB_DoubleClicked();
    void DMS_DoubleClicked();
    void CCM_DoubleClicked();
    void EE_DoubleClicked();
    void TM_DoubleClicked();
    void GAMMA_DoubleClicked();
    void CSC_DoubleClicked();
    void NR_YUV_DoubleClicked();
    void SCALE_DoubleClicked();
    void CROP_DoubleClicked();
    void YFC_DoubleClicked();
    QSerialPort* serial;   ////串口对象
private:
    Ui::link_board *ui;
    
};

#endif // LINK_BOARD_H
