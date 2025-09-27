#ifndef XML_MODULE_TAB_H
#define XML_MODULE_TAB_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTreeWidgetItem>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QGroupBox>

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
    // void refresh();
    void importXML();
    // void exportXml();
    void importConfig();
    void exportConfig();
    void allRead();
    void allWrite();
    void onModuleTreeDoubleClicked(QTreeWidgetItem *item, int column);


private:
// structs
struct Param {
    QString paramName;
    int min = 0;
    int max = 0;
    int defaultVal = 0;
    int address = 0;
    // AddressType addrType = AddressType::Virtual_Addr;
};
struct Module {
    QString moduleName;
    QVector<Param> params;
};
struct XMLConfig {
    QStringList moduleOrder;
    QVector<Module> modules;
};

private:
    bool parseXML(const QByteArray &data);
    void printXMLConfig();
    void clearUI();
    void generateUI();
    void generateModuleTree();
    void generateModuleGroup(const Module &module);
    void readModule(QGroupBox *group);
    void writeModule(QGroupBox *group);

private:
    Ui::XMLModuleTab *ui;

    bool m_connected = false;
    QVBoxLayout *m_moduleLayout;
    XMLConfig m_xmlConfig;
};

#endif // XML_MODULE_TAB_H
