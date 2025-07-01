#include "ccm_tab.h"

CCM_tab::CCM_tab(QWidget *parent)
    : ConfigurableTab(parent)
{
	ui.setupUi(this);
    link_tab=qobject_cast<link_board*>(this->parent());
    ccm_model = new QStandardItemModel(3, 3, this);
    spinBoxes = {
        ui.ccm_a11, ui.ccm_a12, ui.ccm_a13,
        ui.ccm_a21, ui.ccm_a22, ui.ccm_a23,
        ui.ccm_a31, ui.ccm_a32, ui.ccm_a33
    };
    foreach (QSpinBox *spinBox, spinBoxes) {
        spinBox->setDisplayIntegerBase(16);  // 16进制显示
        spinBox->setPrefix("0x");           // 添加0x前缀
        spinBox->setRange(0, 0x1FFF);       // 设置范围(0-65535)
    }
    connect(ccm_model, &QStandardItemModel::dataChanged, this, &CCM_tab::updateUIFromModel);
    ccm_model->setData(ccm_model->index(0, 0),0x04a2);
    ccm_model->setData(ccm_model->index(0, 1),0x1f6e);
    ccm_model->setData(ccm_model->index(0, 2),0x00b4);
    ccm_model->setData(ccm_model->index(1, 0),0x1fe5);
    ccm_model->setData(ccm_model->index(1, 1),0x050f);
    ccm_model->setData(ccm_model->index(1, 2),0x1e00);
    ccm_model->setData(ccm_model->index(2, 0),0x1f7a);
    ccm_model->setData(ccm_model->index(2, 1),0x1f84);
    ccm_model->setData(ccm_model->index(2, 2),0x054b);
}

CCM_tab::~CCM_tab()
{}


QMap<QString, int> CCM_tab::getAllParams() const
{
    QMap<QString, int> params;
    // CCM矩阵的9个参数
    params["ccm_a11"] = ui.ccm_a11->value();
    params["ccm_a12"] = ui.ccm_a12->value();
    params["ccm_a13"] = ui.ccm_a13->value();
    params["ccm_a21"] = ui.ccm_a21->value();
    params["ccm_a22"] = ui.ccm_a22->value();
    params["ccm_a23"] = ui.ccm_a23->value();
    params["ccm_a31"] = ui.ccm_a31->value();
    params["ccm_a32"] = ui.ccm_a32->value();
    params["ccm_a33"] = ui.ccm_a33->value();
    return params;
}

void CCM_tab::setParams(const QMap<QString, int>& params)
{
    if (params.contains("ccm_a11")) ui.ccm_a11->setValue(params["ccm_a11"]);
    if (params.contains("ccm_a12")) ui.ccm_a12->setValue(params["ccm_a12"]);
    if (params.contains("ccm_a13")) ui.ccm_a13->setValue(params["ccm_a13"]);
    if (params.contains("ccm_a21")) ui.ccm_a21->setValue(params["ccm_a21"]);
    if (params.contains("ccm_a22")) ui.ccm_a22->setValue(params["ccm_a22"]);
    if (params.contains("ccm_a23")) ui.ccm_a23->setValue(params["ccm_a23"]);
    if (params.contains("ccm_a31")) ui.ccm_a31->setValue(params["ccm_a31"]);
    if (params.contains("ccm_a32")) ui.ccm_a32->setValue(params["ccm_a32"]);
    if (params.contains("ccm_a33")) ui.ccm_a33->setValue(params["ccm_a33"]);
}

void CCM_tab::updateModelFromUI()
{
    disconnect(ccm_model, &QStandardItemModel::dataChanged, this, &CCM_tab::updateUIFromModel);
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int value = spinBoxes[row*3+col]->value();
            ccm_model->setData(ccm_model->index(row, col), value);
        }
    }
    connect(ccm_model, &QStandardItemModel::dataChanged, this, &CCM_tab::updateUIFromModel);
}

void CCM_tab::on_ccm_write_btn_clicked()
{
    updateModelFromUI();   ///先从界面拿到数据并更新到模型中
    uint32_t ccm_a11 = ccm_model->data(ccm_model->index(0,0)).toUInt();
    uint32_t ccm_a12 = ccm_model->data(ccm_model->index(0,1)).toUInt();
    uint32_t ccm_a13 = ccm_model->data(ccm_model->index(0,2)).toUInt();
    uint32_t ccm_a21 = ccm_model->data(ccm_model->index(1,0)).toUInt();
    uint32_t ccm_a22 = ccm_model->data(ccm_model->index(1,1)).toUInt();
    uint32_t ccm_a23 = ccm_model->data(ccm_model->index(1,2)).toUInt();
    uint32_t ccm_a31 = ccm_model->data(ccm_model->index(2,0)).toUInt();
    uint32_t ccm_a32 = ccm_model->data(ccm_model->index(2,1)).toUInt();
    uint32_t ccm_a33 = ccm_model->data(ccm_model->index(2,2)).toUInt();

    uint32_t CCM_REG1 = ((ccm_a12<<13) &  0x3FFFE000) | (ccm_a11 & 0x1FFF);
    uint32_t CCM_REG2 = ((ccm_a21<<13) &  0x3FFFE000) | (ccm_a13 & 0x1FFF);
    uint32_t CCM_REG3 = ((ccm_a23<<13) &  0x3FFFE000) | (ccm_a22 & 0x1FFF);
    uint32_t CCM_REG4 = ((ccm_a32<<13) &  0x3FFFE000) | (ccm_a31 & 0x1FFF);
    uint32_t CCM_REG5 = (ccm_a33 & 0x1FFF);

    uint32_t CCM_ADDR1 = REG_CCM_COEFF1_ADDR;
    uint32_t CCM_ADDR2 = REG_CCM_COEFF2_ADDR;
    uint32_t CCM_ADDR3 = REG_CCM_COEFF3_ADDR;
    uint32_t CCM_ADDR4 = REG_CCM_COEFF4_ADDR;
    uint32_t CCM_ADDR5 = REG_CCM_COEFF5_ADDR;

    qDebug()<<"write addr:"<<Qt::uppercasedigits << Qt::hex<<CCM_ADDR1<<" "<<CCM_ADDR2<<" "<<CCM_ADDR3<<" "<<CCM_ADDR4<<" "<<CCM_ADDR5;
    qDebug()<<"write value:"<<Qt::uppercasedigits << Qt::hex<<CCM_REG1<<" "<<CCM_REG2<<" "<<CCM_REG3<<" "<<CCM_REG4<<" "<<CCM_REG5;


    uint32_t CCM_DATA[10]={CCM_ADDR1,CCM_REG1,CCM_ADDR2,CCM_REG2,CCM_ADDR3,CCM_REG3,CCM_ADDR4,CCM_REG4,CCM_ADDR5,CCM_REG5};

    uint8_t databuf[41];
    databuf[0] = CCM_MODULE;    //模块标识
    for(int i=0;i<10;i++){
        memcpy(databuf+1+i*4,&CCM_DATA[i],4);
    }
    link_tab->send_cmd_data(WRITE_REG_CMD,databuf,41);
}


void CCM_tab::updateUIFromModel()
{
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int value = ccm_model->data(ccm_model->index(row, col)).toInt();
            spinBoxes[row*3+col]->blockSignals(true); // 防止触发valueChanged信号
            spinBoxes[row*3+col]->setValue(value);
            spinBoxes[row*3+col]->blockSignals(false);
        }
    }
}





