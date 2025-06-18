#include "debug.h"
#include "ui_debug.h"

Debug::Debug(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Debug)
{
    ui->setupUi(this);
    link_tab=qobject_cast<link_board*>(this->parent());   ////父对象指针,new的时候传入,拿取以后便于对象之间的通信，无需对其进行清除
    ui->tabWidget->tabBar()->setEnabled(false);
    ui->tabWidget->setTabText(0,"Main");
    ui->tabWidget->setTabText(1,"TPG");
    ui->tabWidget->setTabText(2,"ISP");
    ui->tabWidget->setCurrentIndex(0);
    // qDebug()<<"Hello bro";
    //TPG configure button
    radioGroup = new QButtonGroup(this);
    radioGroup->addButton(ui->radioButton_1,1);
    radioGroup->addButton(ui->radioButton_2,2);
    radioGroup->addButton(ui->radioButton_3,3);
    radioGroup->addButton(ui->radioButton_4,4);
    radioGroup->addButton(ui->radioButton_5,5);
    radioGroup->addButton(ui->radioButton_6,6);
    radioGroup->addButton(ui->radioButton_7,7);
    radioGroup->addButton(ui->radioButton_8,8);
    radioGroup->addButton(ui->radioButton_9,9);
    radioGroup->addButton(ui->radioButton_10,10);
    radioGroup->addButton(ui->radioButton_11,11);
    radioGroup->addButton(ui->radioButton_12,12);
    radioGroup->addButton(ui->radioButton_13,13);
    radioGroup->addButton(ui->radioButton_14,14);
    radioGroup->addButton(ui->radioButton_15,15);
    radioGroup->addButton(ui->radioButton_16,16);
    radioGroup->addButton(ui->radioButton_17,17);
    radioGroup->addButton(ui->radioButton_18,18);
    radioGroup->addButton(ui->radioButton_19,19);
    radioGroup->addButton(ui->radioButton_20,20);
    ui->radioButton_1->setChecked(true);
    //////ISP  Checkbox configure
    checkboxGroup = new QButtonGroup(this);
    checkboxGroup->setExclusive(false); // 允许多选
    checkboxGroup->addButton(ui->TPG_MODULE_EN_check,0);
    checkboxGroup->addButton(ui->DPC_MODULE_EN_check,1);
    checkboxGroup->addButton(ui->BLC_MODULE_EN_check,2);
    checkboxGroup->addButton(ui->LSC_MODULE_EN_check,3);
    checkboxGroup->addButton(ui->NR_RAW_MODULE_EN_check,4);
    checkboxGroup->addButton(ui->AWB_MODULE_EN_check,5);
    checkboxGroup->addButton(ui->WBC_MODULE_EN_check,6);
    checkboxGroup->addButton(ui->STA_MODULE_EN_check,7);
    checkboxGroup->addButton(ui->GB_MODULE_EN_check,8);
    checkboxGroup->addButton(ui->DMS_MODULE_EN_check,9);
    checkboxGroup->addButton(ui->CCM_MODULE_EN_check,10);
    checkboxGroup->addButton(ui->GAMMA_MODULE_EN_check,11);
    checkboxGroup->addButton(ui->CSC_MODULE_EN_check,12);
    checkboxGroup->addButton(ui->NR_YUV_MODULE_EN_check,13);
}

Debug::~Debug()
{
    delete ui;
}

void Debug::on_sensor_btn_clicked()
{
    // QByteArray transdata;
    // QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    // stream <<'S';
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[1]={CAMERA_SENSOR};
    // databuf[0]=CAMERA_SENSOR;
    link_tab->send_cmd_data(DEBUG_CMD,databuf,1);
}


void Debug::on_tpg_btn_clicked()
{
    // QByteArray transdata;
    // QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    // stream <<'T';
    // link_tab->Write_Data(transdata);        /////发送组合数据

    uint8_t databuf[1]={TPG_MENU};
    link_tab->send_cmd_data(DEBUG_CMD,databuf,1);
    ui->tabWidget->setCurrentIndex(1);
}


void Debug::on_isp_en_btn_clicked()
{
    // QByteArray transdata;
    // QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    // stream <<'I';
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[1]={ISP_EN};
    link_tab->send_cmd_data(DEBUG_CMD,databuf,1);
    ui->tabWidget->setCurrentIndex(2);
}


void Debug::on_sd_save_btn_clicked()
{
    // QByteArray transdata;
    // QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    // stream <<'D';
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[1]={SD_SAVE};
    link_tab->send_cmd_data(DEBUG_CMD,databuf,1);
}



void Debug::on_tpg_back_btn_clicked()
{
    // QByteArray transdata;
    // QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    // stream <<"99\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[]={"99\n"};
    link_tab->send_cmd_data(DEBUG_CMD,databuf,3);
    ui->tabWidget->setCurrentIndex(0);
}


void Debug::on_tpg_cfg_btn_clicked()
{
    int selectedId = radioGroup->checkedId();
    // qDebug()<<selectedId;
    QByteArray numstr = QByteArray::number(selectedId);
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<numstr<<'\n';
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
    // link_tab->Write_Data(transdata);
}


void Debug::on_isp_back_btn_clicked()
{
    // QByteArray transdata;
    // QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    // stream <<"99\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[]={"99\n"};
    link_tab->send_cmd_data(DEBUG_CMD,databuf,3);
    ui->tabWidget->setCurrentIndex(0);
}


void Debug::on_isp_cfg_btn_clicked()
{
    // QList<QAbstractButton*> checkedButtons = checkboxGroup->buttons();
    // for (QAbstractButton *button : checkedButtons) {
    // //     if (button->isChecked()) {
    // //         int id = checkboxGroup->id(button);
    // //         switch(id) {
    // //         case 1:
    // //             // 执行操作1
    // //             break;
    // //         case 2:
    // //             // 执行操作2
    // //             break;
    // //         case 3:
    // //             // 执行操作3
    // //             break;
    // //         }
    // //     }
    // }
}


void Debug::on_TPG_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"0\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}

void Debug::on_DPC_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"1\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}


void Debug::on_BLC_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"2\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));

}


void Debug::on_LSC_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"3\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}


void Debug::on_NR_RAW_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"4\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}


void Debug::on_AWB_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"5\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}


void Debug::on_WBC_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"6\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));

}

void Debug::on_STA_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"7\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));

}

void Debug::on_GB_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"8\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}

void Debug::on_DMS_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"9\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}


void Debug::on_CCM_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"10\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}


void Debug::on_GAMMA_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"11\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));

}


void Debug::on_CSC_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"12\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));

}


void Debug::on_NR_YUV_MODULE_EN_check_stateChanged(int arg1)
{
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream <<"13\n";
    // link_tab->Write_Data(transdata);        /////发送组合数据
    uint8_t databuf[10];
    memcpy(databuf,transdata.constData(),transdata.size());
    link_tab->send_cmd_data(DEBUG_CMD,databuf,static_cast<uint16_t>(transdata.size()));
}







