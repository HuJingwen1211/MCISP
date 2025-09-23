#ifndef XML_MODULE_TAB_H
#define XML_MODULE_TAB_H

#include <QMainWindow>
#include <QVBoxLayout>

namespace Ui {
class XMLModuleTab;
}

class XMLModuleTab : public QMainWindow
{
    Q_OBJECT

public:
    explicit XMLModuleTab(QWidget *parent = nullptr);
    ~XMLModuleTab();


    void createTestModule();

private slots:
    // void onConnectClicked();
    // void onRefreshClicked();
    // void onAllReadClicked();
    // void onAllWriteClicked();
    // void onImportXmlClicked();
    // void onExportXmlClicked();
    // void onImportConfigClicked();
    // void onExportConfigClicked();

private:
    Ui::XMLModuleTab *ui;
    // 连接状态
    bool m_connected = false;
    // 布局管理器
    QVBoxLayout *m_moduleLayout;
};

#endif // XML_MODULE_TAB_H
