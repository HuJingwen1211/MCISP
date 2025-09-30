#ifndef XML_MODULE_TAB_H
#define XML_MODULE_TAB_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <QString>
#include <QByteArray>
#include <QGroupBox>
#include "xml_config.h"
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
    void connectToBoard();
    void disconnectFromBoard();
    void setEditMode();
    void refresh();
    void importXML();
    void exportXml();
    void importConfig();
    void exportConfig();
    void allRead();
    void allWrite();
    void onModuleTreeDoubleClicked(QTreeWidgetItem *item, int column);

    void addNewModule();

private:
    bool parseXML(const QByteArray &data);
    void printXMLConfig();
    void clearUI();
    void generateUI();
    void generateModuleTree();
    void generateModuleGroup(const Module &module);
    void readModule(QGroupBox *group);
    void writeModule(QGroupBox *group);

    void printLog(const QString &message);

private:
    Ui::XMLModuleTab *ui;

    bool m_connected = false;
    XMLConfig m_xmlConfig;
    bool m_isEditMode = false;
};

#endif // XML_MODULE_TAB_H
