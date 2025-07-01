#ifndef CONFIGURABLE_TAB_H
#define CONFIGURABLE_TAB_H

#include <QWidget>
#include <QMap>
#include <QString>

class ConfigurableTab : public QWidget
{
    Q_OBJECT
public:
    //构造函数
    explicit ConfigurableTab(QWidget *parent = nullptr) : QWidget(parent) {}

    virtual QMap<QString, int> getAllParams() const = 0;
    virtual QString getModuleName() const = 0;
    virtual void setParams(const QMap<QString, int>& params) = 0;
    virtual ~ConfigurableTab() = default;

    static const QMap<QString, QMap<QString, int>> defaultParams;
};


#endif // CONFIGURABLE_TAB_H
