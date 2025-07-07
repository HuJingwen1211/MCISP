#ifndef LINK_BOARD_H
#define LINK_BOARD_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTreeWidget>
#include <QTextCharFormat>
#include <QTcpSocket>

#define BUFFER_SIZE   256    ///发送接收的最大缓存
#define STR_CMD       0x01
#define DEBUG_CMD     0x02
#define WRITE_REG_CMD 0x03
#define READ_REG_CMD  0x04
#define CAPTURE_CMD   0x05
#define TEST_RW_CMD   0x06


////各个模块的标识号
#define DPC_MODULE     0x01
#define BLC_MODULE     0x02
#define LSC_MODULE     0x03
#define NR_RAW_MODULE  0x04
#define AWBC_MODULE    0x05
#define GB_MODULE      0x06
#define DMS_MODULE     0x07
#define CCM_MODULE     0x08
#define GAMMA_MODULE   0x09
#define CSC_MODULE     0x0A
#define NR_YUV_MODULE  0x0B


namespace Ui {
class link_board;
}

class link_board : public QMainWindow
{
    Q_OBJECT

signals:
    void frameReceived(uint8_t cmd, const QByteArray &data);  // 完整帧接收信号
    void imageReceived(const QByteArray &imageData);
    ////各个模块的signal
    void test_rw_signal(const QByteArray &regData);
    void awbc_read_done(const QByteArray &regData);


private:
    int Send(const uint8_t *data,uint16_t len);   //串口发送
    uint16_t CRC16_Check(const uint8_t *data,uint8_t len);  //CRC校验
    void Receive(uint8_t byteData);  // 帧解析状态机
    //状态机变量
    struct {
        uint8_t step = 0;
        uint8_t cnt = 0;
        uint8_t Buf[300];
        uint8_t len = 0;
        uint8_t cmd = 0;
        uint8_t *data_ptr = nullptr;
        uint16_t crc16 = 0;
    } frameState;
    struct ImageReception {
        bool active = false;
        uint32_t totalFrames = 0;
        uint32_t receivedFrames = 0;
        uint32_t frameDataSize = 0;
        QVector<QByteArray> frameData;
    };
    void process_recv_image(const QByteArray &data);
    void resetReception();
    void startNewReception(uint32_t totalFrames, uint32_t frameDataSize);
    ImageReception currentReception;
    QByteArray partialFrame;

    ///用于串口颜色判断：
    QTextCharFormat m_currentFormat;
    QTextCharFormat m_defaultFormat;
    bool m_inEscapeSequence = false;
    QString m_escapeSequence;
    void processColorByte(uint8_t byte);
    void resetColorFormat();
    void applyAnsiColorFormat(const QString &ansiCode);
    void appendChar(char c);


    void exportAllConfig();
    void importAllConfig();
    bool setParamsToTab(const QString& moduleName, const QMap<QString, int>& params);
    void openAllModuleTabs();


public:
    explicit link_board(QWidget *parent = 0);
    ~link_board();
    void send_cmd_data(uint8_t cmd,const uint8_t *datas,uint16_t len);   //封装命令数据并发送
    void read_reg_process(const QByteArray &data);

private slots:
    void handle_redy_read();
    void process_cmd_data(uint8_t cmd, const QByteArray &data);  //处理命令数据
    void on_clear_btn_clicked();
    void on_link_btn_clicked();
    void on_module_list_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_link_tab_tabCloseRequested(int index);
    void save_image(const QByteArray &imageData);

    void on_import_cfg_btn_clicked();

    void on_export_cfg_btn_clicked();

    void on_boot_cfg_btn_clicked();

    void printTimeStamp();

    void on_serial_radio_toggled(bool checked);

    void on_network_radio_toggled(bool checked);

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
    void Debug_DoubleClicked();
    void Capture_DoubleClicked();
    QSerialPort* serial = nullptr;   ////串口对象

private:
    Ui::link_board *ui;
    QTcpSocket* tcpSocket = nullptr;
    bool isNetworkMode = false; // 可选，用于区分当前模式
};



#endif // LINK_BOARD_H
