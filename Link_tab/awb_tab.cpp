#include "awb_tab.h"

AWB_tab::AWB_tab(QWidget *parent)
    : ConfigurableTab(parent)
{
	ui.setupUi(this);
    link_tab=qobject_cast<link_board*>(this->parent());
    awbc_model = new QStandardItemModel(1, 3, this);
    spinBoxes = {
        ui.R_Gain,ui.G_Gain,ui.B_Gain
    };
    foreach (QSpinBox *spinBox, spinBoxes) {
        spinBox->setDisplayIntegerBase(16);  // 16进制显示
        spinBox->setPrefix("0x");           // 添加0x前缀
        spinBox->setRange(0, 0x3FFF);       // 设置范围(0-65535)
    }
    connect(awbc_model, &QStandardItemModel::dataChanged, this, &AWB_tab::updateUIFromModel);
    awbc_model->setData(awbc_model->index(0, 0),4095);
    awbc_model->setData(awbc_model->index(0, 1),4095);
    awbc_model->setData(awbc_model->index(0, 2),4095);
}


AWB_tab::~AWB_tab()
{}



QMap<QString, int> AWB_tab::getAllParams() const
{
    QMap<QString, int> params;
    params["R_Gain"] = ui.R_Gain->value();
    params["G_Gain"] = ui.G_Gain->value();
    params["B_Gain"] = ui.B_Gain->value();
    return params;
}

void AWB_tab::setParams(const QMap<QString, int>& params)
{
    if (params.contains("R_Gain")) ui.R_Gain->setValue(params["R_Gain"]);
    if (params.contains("G_Gain")) ui.G_Gain->setValue(params["G_Gain"]);
    if (params.contains("B_Gain")) ui.B_Gain->setValue(params["B_Gain"]);
}


void AWB_tab::updateModelFromUI()
{
    disconnect(awbc_model, &QStandardItemModel::dataChanged, this, &AWB_tab::updateUIFromModel);
    for(int i=0;i<3;i++){
        int value = spinBoxes[i]->value();
        awbc_model->setData(awbc_model->index(0, i), value);
    }
    connect(awbc_model, &QStandardItemModel::dataChanged, this, &AWB_tab::updateUIFromModel);
}

void AWB_tab::on_awb_read_btn_clicked()
{
    uint32_t ADDR1 = REG_AWB_GAIN_1_ADDR;
    uint32_t ADDR2 = REG_AWB_GAIN_2_ADDR;
    uint8_t  databuf[9];
    databuf[0] = AWBC_MODULE;
    memcpy(databuf+1,&ADDR1,4);
    memcpy(databuf+5,&ADDR2,4);
    link_tab->send_cmd_data(READ_REG_CMD,databuf,9);
}

void AWB_tab::updateUIFromModel()
{
    for(int i=0;i<3;i++){
        int value = awbc_model->data(awbc_model->index(0, i)).toInt();
        spinBoxes[i]->blockSignals(true); // 防止触发valueChanged信号
        spinBoxes[i]->setValue(value);
        spinBoxes[i]->blockSignals(false);
    }
}


void AWB_tab::on_awb_write_btn_clicked()
{
    updateModelFromUI();    //先从界面更新数据
    uint32_t WBC_RGain = awbc_model->data(awbc_model->index(0, 0)).toUInt();
    uint32_t WBC_GGain = awbc_model->data(awbc_model->index(0, 1)).toUInt();
    uint32_t WBC_BGain = awbc_model->data(awbc_model->index(0, 2)).toUInt();
    uint8_t  databuf[17];
    databuf[0] = AWBC_MODULE;
    uint32_t ADDR1 = REG_WBC_GAIN_1_ADDR;
    uint32_t ADDR2 = REG_WBC_GAIN_2_ADDR;
    uint32_t VALUE1 = ((WBC_GGain<<16) & 0xFFFF0000 ) | ( WBC_RGain & 0x0000FFFF);
    uint32_t VALUE2 = WBC_BGain & 0x0000FFFF;

    memcpy(databuf+1,&ADDR1,4);
    memcpy(databuf+5,&VALUE1,4);
    memcpy(databuf+9,&ADDR2,4);
    memcpy(databuf+13,&VALUE2,4);

    link_tab->send_cmd_data(WRITE_REG_CMD,databuf,17);

    // qDebug()<<"write addr:"<<Qt::uppercasedigits << Qt::hex<<ADDR1<<" "<<Qt::uppercasedigits << Qt::hex<<ADDR2;
    // qDebug()<<"write value:"<<Qt::uppercasedigits << Qt::hex<<VALUE1<<" "<<Qt::uppercasedigits << Qt::hex<<VALUE2;
}

void AWB_tab::read_reg_process(const QByteArray &regData)
{
    const uchar *ptr = reinterpret_cast<const uchar*>(regData.constData());
    uint32_t VALUE1,VALUE2;
    memcpy(&VALUE1,ptr+1,4);
    memcpy(&VALUE2,ptr+5,4);
    uint32_t G_Gain = (VALUE1>>16) & 0x0000FFFF;
    uint32_t R_Gain = VALUE1 & 0x0000FFFF;
    uint32_t B_Gain = VALUE2;
    awbc_model->setData(awbc_model->index(0, 0),R_Gain);    ///改变数据模型后，界面自动变化
    awbc_model->setData(awbc_model->index(0, 1),G_Gain);
    awbc_model->setData(awbc_model->index(0, 2),B_Gain);
    // qDebug()<<"Read Value"<<R_Gain<<" "<<G_Gain<<" "<<B_Gain;
}





