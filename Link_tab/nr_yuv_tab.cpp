#include "nr_yuv_tab.h"

NR_YUV_tab::NR_YUV_tab(QWidget *parent)
    : ConfigurableTab(parent)
{
	ui.setupUi(this);
    link_tab=qobject_cast<link_board*>(this->parent());
    nryuv_model = new QStandardItemModel(1, 3, this);
    spinBoxes ={
        ui.sigma_R,ui.sigma_S_1,ui.sigma_S_4
    };
    connect(nryuv_model, &QStandardItemModel::dataChanged, this, &NR_YUV_tab::updateUIFromModel);
    foreach (QSpinBox *spinBox, spinBoxes) {
        // spinBox->setDisplayIntegerBase(16);  // 16进制显示
        // spinBox->setPrefix("0x");           // 添加0x前缀
        spinBox->setRange(0, 65535);       // 设置范围(0-65535)
    }
    nryuv_model->setData(nryuv_model->index(0, 0),7);
    nryuv_model->setData(nryuv_model->index(0, 1),904);
    nryuv_model->setData(nryuv_model->index(0, 2),797);

}

NR_YUV_tab::~NR_YUV_tab()
{

}


QMap<QString, int> NR_YUV_tab::getAllParams() const
{
    QMap<QString, int> params;
    params["sigma_R"] = ui.sigma_R->value();
    params["sigma_S_1"] = ui.sigma_S_1->value();
    params["sigma_S_4"] = ui.sigma_S_4->value();
    return params;
}

void NR_YUV_tab::setParams(const QMap<QString, int>& params)
{
    if (params.contains("sigma_R")) ui.sigma_R->setValue(params["sigma_R"]);
    if (params.contains("sigma_S_1")) ui.sigma_S_1->setValue(params["sigma_S_1"]);
    if (params.contains("sigma_S_4")) ui.sigma_S_4->setValue(params["sigma_S_4"]);
}


void NR_YUV_tab::updateModelFromUI()
{
    disconnect(nryuv_model, &QStandardItemModel::dataChanged, this, &NR_YUV_tab::updateUIFromModel);
    for(int i=0;i<3;i++){
        int value = spinBoxes[i]->value();
        nryuv_model->setData(nryuv_model->index(0, i), value);
    }
    connect(nryuv_model, &QStandardItemModel::dataChanged, this, &NR_YUV_tab::updateUIFromModel);
}

void NR_YUV_tab::on_nryuv_write_btn_clicked()
{
    updateModelFromUI();    //先从界面更新数据
    uint32_t sigma_r  = nryuv_model->data(nryuv_model->index(0, 0)).toUInt();
    uint32_t sigma_s1 = nryuv_model->data(nryuv_model->index(0, 1)).toUInt();
    uint32_t sigma_s4 = nryuv_model->data(nryuv_model->index(0, 2)).toUInt();
    uint32_t NRYUV_REG1 = sigma_r & 0x0000FFFF;
    uint32_t NRYUV_REG2 = ((sigma_s1<<10) & 0xFFC00) | (sigma_s4 & 0x3FF);
    uint32_t NRYUV_ADDR1 = REG_NR_YUV_0_ADDR;
    uint32_t NRYUV_ADDR2 = REG_NR_YUV_1_ADDR;

    uint8_t databuf[17];
    databuf[0] = NR_YUV_MODULE;      ///NRYUV模块标识
    memcpy(databuf+1,&NRYUV_ADDR1,4);
    memcpy(databuf+5,&NRYUV_REG1,4);
    memcpy(databuf+9,&NRYUV_ADDR2,4);
    memcpy(databuf+13,&NRYUV_REG2,4);

    qDebug()<<"write addr:"<<Qt::uppercasedigits << Qt::hex<<NRYUV_ADDR1<<" "<<NRYUV_ADDR2;
    qDebug()<<"write value:"<<Qt::uppercasedigits << Qt::hex<<NRYUV_REG1<<" "<<NRYUV_REG2;

    link_tab->send_cmd_data(WRITE_REG_CMD,databuf,17);
}



void NR_YUV_tab::updateUIFromModel()
{
    for(int i=0;i<3;i++){
        int value = nryuv_model->data(nryuv_model->index(0, i)).toUInt();
        spinBoxes[i]->blockSignals(true); // 防止触发valueChanged信号
        spinBoxes[i]->setValue(value);
        spinBoxes[i]->blockSignals(false);
    }

}

