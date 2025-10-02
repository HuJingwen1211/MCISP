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
    void showConnectDialog();
    void setEditMode();
    void refreshToDefault();
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
    QByteArray serializeXML() const;
    void printXMLConfig();
    void clearUI();
    void generateUI();
    void generateModuleTree();
    void generateModuleGroup(const Module &module);
    void readModule(QGroupBox *group);
    void writeModule(QGroupBox *group);
    bool applyConfigToUI(const QByteArray &data);
    QByteArray collectConfigFromUI();
    QLabel* findLabelForSpinBox(QGroupBox* group, QSpinBox* spinBox);
    bool parseConfigFile(const QByteArray &data);
    void printLog(const QString &message);

private:
    Ui::XMLModuleTab *ui;

    bool m_connected = false;
    XMLConfig m_xmlConfig;
    bool m_isEditMode = false;
};

#endif // XML_MODULE_TAB_H
