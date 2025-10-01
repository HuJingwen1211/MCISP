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
    uint32_t address = 0;
};
struct Module {
    QString moduleName;
    QVector<Param> params{};
};
struct XMLConfig {
    QVector<Module> modules;
};
#endif // XML_CONFIG_H
