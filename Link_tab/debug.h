#ifndef DEBUG_H
#define DEBUG_H

#include <QWidget>
#include "link_board.h"
#include <QButtonGroup>
// #include <qDebug>

namespace Ui {
class Debug;
}

class Debug : public QWidget
{
    Q_OBJECT

public:
    explicit Debug(QWidget *parent = nullptr);
    ~Debug();

private slots:
    void on_sensor_btn_clicked();

    void on_tpg_btn_clicked();

    void on_isp_en_btn_clicked();

    void on_sd_save_btn_clicked();

    void on_tpg_back_btn_clicked();

    void on_tpg_cfg_btn_clicked();

    void on_isp_back_btn_clicked();

    void on_isp_cfg_btn_clicked();

    void on_DMS_MODULE_EN_check_stateChanged(int arg1);

    void on_TPG_MODULE_EN_check_stateChanged(int arg1);

    void on_BLC_MODULE_EN_check_stateChanged(int arg1);

    void on_LSC_MODULE_EN_check_stateChanged(int arg1);

    void on_NR_RAW_MODULE_EN_check_stateChanged(int arg1);

    void on_AWB_MODULE_EN_check_stateChanged(int arg1);

    void on_WBC_MODULE_EN_check_stateChanged(int arg1);


    void on_GB_MODULE_EN_check_stateChanged(int arg1);

    void on_CCM_MODULE_EN_check_stateChanged(int arg1);

    void on_GAMMA_MODULE_EN_check_stateChanged(int arg1);

    void on_CSC_MODULE_EN_check_stateChanged(int arg1);

    void on_NR_YUV_MODULE_EN_check_stateChanged(int arg1);

    void on_STA_MODULE_EN_check_stateChanged(int arg1);

    void on_DPC_MODULE_EN_check_stateChanged(int arg1);

private:
    link_board *link_tab;
    QButtonGroup *radioGroup;
    QButtonGroup *checkboxGroup;
    Ui::Debug *ui;
};

#endif // DEBUG_H
