#ifndef XML_MODULE_TAB_H
#define XML_MODULE_TAB_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <QString>
#include <QByteArray>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QSerialPortInfo>
#include "xml_config.h"
#include "Comm/comm_manager.h"
// enum class AddressType {
//     Physical_Addr = 0,
//     Virtual_Addr = 1
// };
namespace Ui {
class XMLModuleTab;
}

class XMLModuleTab : public QMainWindow
{
    Q_OBJECT

public:
    explicit XMLModuleTab(QWidget *parent = nullptr);
    ~XMLModuleTab();

private slots:
    void setEditMode();
    void refreshToDefault();
    void importXML();
    void exportXml();
    void importConfig();
    void exportConfig();
    void allRead();
    void allWrite();
    void onModuleTreeDoubleClicked(QTreeWidgetItem *item, int column);
    void onLinkBtnClicked();
    void addNewModule();
    
    // 网络连接成功/失败回调
    void onNetworkConnected();
    void onNetworkDisconnected();

private:
    // UI 初始化
    void initializeUI();
    
    // XML 操作
    bool parseXML(const QByteArray &data);
    QByteArray serializeXML() const;
    void printXMLConfig();
    
    // UI 生成与管理
    void clearUI();
    void generateUI();
    void generateModuleTree();
    void generateModuleGroup(const Module &module);
    
    // 模块读写
    void readModule(QGroupBox *group);
    void writeModule(QGroupBox *group);
    
    // 配置文件操作
    bool applyConfigToUI(const QByteArray &data);
    QByteArray collectConfigFromUI();
    QLabel* findLabelForSpinBox(QGroupBox* group, QSpinBox* spinBox);
    bool parseConfigFile(const QByteArray &data);
    
    // 日志
    void printLog(const QString &message);
    
    // 连接处理
    void handleSerialConnect();
    void handleNetworkConnect();
    void handleDisconnect();

private:
    Ui::XMLModuleTab *ui;

    CommManager *m_commMgr = nullptr;
    bool m_connected = false;
    XMLConfig m_xmlConfig;
    bool m_isEditMode = false;
};

#endif // XML_MODULE_TAB_H
