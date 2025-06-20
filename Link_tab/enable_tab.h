#pragma once

#include <QMainWindow>
#include "link_board.h"


#define TPG_MODULE_EN             (1<<0)
#define DPC_MODULE_EN             (1<<1)
#define BLC_MODULE_EN             (1<<2)
#define LSC_MODULE_EN             (1<<3)
#define NR_RAW_MODULE_EN          (1<<4)
#define AWB_MODULE_EN             (1<<5)
#define WBC_MODULE_EN             (1<<6)
#define STA_MODULE_EN             (1<<7)
#define GB_MODULE_EN              (1<<8)
#define DMS_MODULE_EN             (1<<9)
#define CCM_MODULE_EN             (1<<10)
#define GAMMA_MODULE_EN           (1<<11)
#define CSC_MODULE_EN             (1<<12)
#define NR_YUV_MODULE_EN          (1<<13)


#define REG_MODULE_EN_ADDR  0xA0000000


namespace Ui {
    class ENABLE_tab;
}


class ENABLE_tab : public QMainWindow
{
    Q_OBJECT

private slots:
    void on_cfg_btn_clicked();

public:
    explicit ENABLE_tab(QWidget *parent = 0);
    ~ENABLE_tab();

private:
    Ui::ENABLE_tab *ui;
    link_board* link_tab;
};
