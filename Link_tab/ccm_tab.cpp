#include "ccm_tab.h"

CCM_tab::CCM_tab(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
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

void CCM_tab::updateModelFromUI()
{
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            int value = spinBoxes[row*3+col]->value();
            ccm_model->setData(ccm_model->index(row, col), value);
        }
    }
}

void CCM_tab::on_ccm_write_btn_clicked()
{
    updateModelFromUI();   ///先从界面拿到数据并更新到模型中

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



