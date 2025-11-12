#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <QObject>
#include <QIODevice>
#include <QSerialPort>
#include <QTcpSocket>
#include <QByteArray>
#include <QVector>
#include <QTextCharFormat>

// 协议常量定义
#define BUFFER_SIZE   256    ///发送接收的最大缓存
#define STR_CMD       0x01
#define DEBUG_CMD     0x02
#define WRITE_REG_CMD 0x03
#define READ_REG_CMD  0x04
#define CAPTURE_CMD   0x05
#define TEST_RW_CMD   0x06

// 各个模块的标识号
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

class CommManager : public QObject
{
    Q_OBJECT

public:
    explicit CommManager(QObject *parent = nullptr);
    ~CommManager();

    // 连接管理
    bool openSerial(const QString& portName, int baudRate);
    bool openNetwork(const QString& ip, int port);
    void close();
    bool isOpen() const;

    // 数据发送
    bool sendCmd(uint8_t cmd, const uint8_t *datas, uint16_t len);
    int writeRaw(const QByteArray& bytes);

signals:
    // 协议层信号
    void frameReceived(uint8_t cmd, const QByteArray &data);
    void imageReceived(const QByteArray &imageData);
    
    // 模块专用信号
    void test_rw_signal(const QByteArray &regData);
    void awbc_read_done(const QByteArray &regData);
    void moduleReadReply(quint8 moduleId, const QByteArray &payload);
    
    // 网络专用信号（只用于异步网络连接）
    void networkConnected();
    void networkDisconnected();
    
    // 日志信号（替代UI直接操作）
    void logMessage(const QString& message);

private slots:
    void onReadyRead();
    void onNetworkConnected();
    void onNetworkDisconnected();

private:
    // 通信设备（统一抽象）
    QIODevice* device = nullptr;
    QSerialPort* serial = nullptr;
    QTcpSocket* tcpSocket = nullptr;

    // 协议处理
    uint16_t CRC16_Check(const uint8_t *data, uint8_t len);
    void Receive(uint8_t byteData);
    void process_cmd_data(uint8_t cmd, const QByteArray &data);
    void read_reg_process(const QByteArray &data);

    // 图像接收处理
    void process_recv_image(const QByteArray &data);
    void resetReception();
    void startNewReception(uint32_t totalFrames, uint32_t frameDataSize);

    // ANSI颜色处理（可选）
    void processColorByte(uint8_t byte);
    void resetColorFormat();
    void applyAnsiColorFormat(const QString &ansiCode);

    // 帧状态机
    struct {
        uint8_t step = 0;
        uint8_t cnt = 0;
        uint8_t Buf[300];
        uint8_t len = 0;
        uint8_t cmd = 0;
        uint8_t *data_ptr = nullptr;
        uint16_t crc16 = 0;
    } frameState;

    // 图像接收状态
    struct ImageReception {
        bool active = false;
        uint32_t totalFrames = 0;
        uint32_t receivedFrames = 0;
        uint32_t frameDataSize = 0;
        QVector<QByteArray> frameData;
    } currentReception;

    // ANSI颜色状态
    QTextCharFormat m_currentFormat;
    QTextCharFormat m_defaultFormat;
    bool m_inEscapeSequence = false;
    QString m_escapeSequence;

    // 内部辅助函数
    void cleanupSerial();
    void cleanupNetwork();
};

#endif // COMM_MANAGER_H
