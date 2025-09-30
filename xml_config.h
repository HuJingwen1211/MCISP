#ifndef XML_CONFIG_H
#define XML_CONFIG_H
#include <QString>
#include <QVector>
#include <QStringList>

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
    QVector<Param> params{};
};
struct XMLConfig {
    QStringList moduleOrder;
    QVector<Module> modules;
};
#endif // XML_CONFIG_H
