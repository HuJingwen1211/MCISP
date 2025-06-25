#ifndef ISP_CONFIG_H
#define ISP_CONFIG_H

#include "isp_pipeline.h"
#include <QString>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

// 配置参数类型定义
enum ISPConfigParamType {
    ISP_PARAM_INT,
    ISP_PARAM_DOUBLE,
    ISP_PARAM_U16,
    ISP_PARAM_INT_ARRAY,
    ISP_PARAM_U16_ARRAY
};

// 配置参数描述结构
struct ISPConfigParam {
    QString name;          // 参数名
    void* ptr;             // 参数指针
    ISPConfigParamType type; // 参数类型
    int size;              // 数组大小（非数组为1）
};

// 配置映射表
typedef QVector<ISPConfigParam> ISPConfigMapping;

class ISPConfig {
public:
    explicit ISPConfig(ISP_Pipeline* pipeline);

    // 配置文件操作
    bool importConfig(const QString& filePath);
    bool exportConfig(const QString& filePath);

    // 验证配置
    bool validateConfig();

    // 参数映射访问
    const ISPConfigMapping& getMapping() const { return m_mapping; }

private:
    void initializeMapping();

    void processParameter(const QString& key, const QString& value);

    ISP_Pipeline* m_pipeline;
    ISPConfigMapping m_mapping;
    QMap<QString, ISPConfigParam> m_paramMap;
};

#endif // ISP_CONFIG_H
