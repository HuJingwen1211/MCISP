#include "ispconfig.h"

ISPConfig::ISPConfig(ISP_Pipeline* pipeline)
    : m_pipeline(pipeline) {
    initializeMapping();
}

void ISPConfig::initializeMapping() {
    // 清除现有映射
    m_mapping.clear();
    m_paramMap.clear();

    // // 基础图像参数
    // m_mapping.append({
    //     {"width", &m_pipeline->isp_cfg_reg->input_width, ISP_PARAM_INT, 1},
    //     {"height", &m_pipeline->isp_cfg_reg->input_height, ISP_PARAM_INT, 1},
    //     {"sensor_bits", &m_pipeline->isp_cfg_reg->sensor_bits, ISP_PARAM_INT, 1},
    //     {"bayer_pattern", &m_pipeline->isp_cfg_reg->bayer_pattern, ISP_PARAM_INT, 1}
    // });

    // DGain 参数
    m_mapping.append({
        {"dgain.enable", &m_pipeline->dgain_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"dgain.r_gain", &m_pipeline->dgain_cfg_reg->Gain_R, ISP_PARAM_DOUBLE, 1},
        {"dgain.gr_gain", &m_pipeline->dgain_cfg_reg->Gain_Gr, ISP_PARAM_DOUBLE, 1},
        {"dgain.gb_gain", &m_pipeline->dgain_cfg_reg->Gain_Gb, ISP_PARAM_DOUBLE, 1},
        {"dgain.b_gain", &m_pipeline->dgain_cfg_reg->Gain_B, ISP_PARAM_DOUBLE, 1}
    });

    // DPC 参数
    m_mapping.append({
        {"dpc.enable", &m_pipeline->dpc_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"dpc.mode", &m_pipeline->dpc_cfg_reg->mode, ISP_PARAM_INT, 1}
    });

    // BLC 参数
    m_mapping.append({
        {"blc.enable", &m_pipeline->blc_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"blc.r_offset", &m_pipeline->blc_cfg_reg->parameter[0], ISP_PARAM_INT, 1},
        {"blc.gr_offset", &m_pipeline->blc_cfg_reg->parameter[1], ISP_PARAM_INT, 1},
        {"blc.gb_offset", &m_pipeline->blc_cfg_reg->parameter[2], ISP_PARAM_INT, 1},
        {"blc.b_offset", &m_pipeline->blc_cfg_reg->parameter[3], ISP_PARAM_INT, 1}
    });

    // LSC 参数 (数组处理)
    m_mapping.append({
        {"lsc.enable", &m_pipeline->lsc_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"lsc.r_gain", m_pipeline->lsc_cfg_reg->rGain, ISP_PARAM_INT_ARRAY, 13*17},
        {"lsc.gr_gain", m_pipeline->lsc_cfg_reg->GrGain, ISP_PARAM_INT_ARRAY, 13*17},
        {"lsc.gb_gain", m_pipeline->lsc_cfg_reg->GbGain, ISP_PARAM_INT_ARRAY, 13*17},
        {"lsc.b_gain", m_pipeline->lsc_cfg_reg->bGain, ISP_PARAM_INT_ARRAY, 13*17}
    });

    // AWB
    m_mapping.append({
        {"awb.enable", &m_pipeline->awb_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"awb.r_gain", &m_pipeline->awb_cfg_reg->r_gain, ISP_PARAM_U16, 1},
        {"awb.g_gain", &m_pipeline->awb_cfg_reg->g_gain, ISP_PARAM_U16, 1},
        {"awb.b_gain", &m_pipeline->awb_cfg_reg->b_gain, ISP_PARAM_U16, 1}
    });

    // CCM
    m_mapping.append({
        {"ccm.enable", &m_pipeline->ccm_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"ccm.coeff", m_pipeline->ccm_cfg_reg->ccm_coeff, ISP_PARAM_INT_ARRAY, 9},
        {"ccm.offset", m_pipeline->ccm_cfg_reg->offset_out, ISP_PARAM_INT_ARRAY, 3}
    });

    // Gamma
    m_mapping.append({
        {"gamma.enable", &m_pipeline->gamma_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"gamma.value", &m_pipeline->gamma_cfg_reg->gamma, ISP_PARAM_DOUBLE, 1}
    });

    // NR_RAW 参数
    m_mapping.append({
        {"nr_raw.enable", &m_pipeline->nr_raw_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"nr_raw.h", &m_pipeline->nr_raw_cfg_reg->h, ISP_PARAM_INT, 1}
    });

    // WBC 参数
    m_mapping.append({
        {"wbc.enable", &m_pipeline->wbc_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"wbc.m_nR", &m_pipeline->wbc_cfg_reg->m_nR, ISP_PARAM_U16, 1},
        {"wbc.m_nGr", &m_pipeline->wbc_cfg_reg->m_nGr, ISP_PARAM_U16, 1},
        {"wbc.m_nGb", &m_pipeline->wbc_cfg_reg->m_nGb, ISP_PARAM_U16, 1},
        {"wbc.m_nB", &m_pipeline->wbc_cfg_reg->m_nB, ISP_PARAM_U16, 1}
    });

    // DMS 参数
    m_mapping.append({
        {"dms.enable", &m_pipeline->dms_cfg_reg->enable, ISP_PARAM_INT, 1}
    });

    // Sharpen (EE) 参数
    m_mapping.append({
        {"sharpen.enable", &m_pipeline->sharpen_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"sharpen.sharpen_param", &m_pipeline->sharpen_cfg_reg->sharpen_param, ISP_PARAM_INT, 1}
    });

    // CSC 参数
    m_mapping.append({
        {"csc.enable", &m_pipeline->csc_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"csc.offset_in", m_pipeline->csc_cfg_reg->offset_in, ISP_PARAM_INT_ARRAY, 3},
        {"csc.offset_out", m_pipeline->csc_cfg_reg->offset_out, ISP_PARAM_INT_ARRAY, 3},
        {"csc.coeff", m_pipeline->csc_cfg_reg->coeff, ISP_PARAM_INT_ARRAY, 9}
    });

    // NR_YUV 参数
    m_mapping.append({
        {"nryuv.enable", &m_pipeline->nryuv_cfg_reg->enable, ISP_PARAM_INT, 1},
        {"nryuv.y_sigma2", &m_pipeline->nryuv_cfg_reg->y_sigma2, ISP_PARAM_INT, 1},
        {"nryuv.y_inv_sigma2", &m_pipeline->nryuv_cfg_reg->y_inv_sigma2, ISP_PARAM_INT, 1},
        {"nryuv.uv_sigma2", &m_pipeline->nryuv_cfg_reg->uv_sigma2, ISP_PARAM_INT, 1},
        {"nryuv.uv_inv_sigma2", &m_pipeline->nryuv_cfg_reg->uv_inv_sigma2, ISP_PARAM_INT, 1},
        {"nryuv.y_filt", &m_pipeline->nryuv_cfg_reg->y_filt, ISP_PARAM_INT, 1},
        {"nryuv.uv_filt", &m_pipeline->nryuv_cfg_reg->uv_filt, ISP_PARAM_INT, 1},
        {"nryuv.y_inv_filt", &m_pipeline->nryuv_cfg_reg->y_inv_filt, ISP_PARAM_INT, 1},
        {"nryuv.uv_inv_filt", &m_pipeline->nryuv_cfg_reg->uv_inv_filt, ISP_PARAM_INT, 1},
        {"nryuv.y_H2", &m_pipeline->nryuv_cfg_reg->y_H2, ISP_PARAM_INT, 1},
        {"nryuv.y_invH2", &m_pipeline->nryuv_cfg_reg->y_invH2, ISP_PARAM_INT, 1},
        {"nryuv.uv_H2", &m_pipeline->nryuv_cfg_reg->uv_H2, ISP_PARAM_INT, 1},
        {"nryuv.uv_invH2", &m_pipeline->nryuv_cfg_reg->uv_invH2, ISP_PARAM_INT, 1}
    });


    // 构建快速查找表
    for (const auto& param : m_mapping) {
        m_paramMap[param.name] = param;
    }
}

bool ISPConfig::importConfig(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << filePath;
        return false;
    }

    QTextStream in(&file);
    int lineNumber = 0;
    QString currentSection; // 存储当前节名称

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        // 跳过空行和注释
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        // 检查是否是节标题 [SectionName]
        if (line.startsWith('[') && line.endsWith(']')) {
            currentSection = line.mid(1, line.length() - 2);
            qDebug() << "进入节:" << currentSection;
            continue;
        }

        // 解析键值对 (支持 key = value 和 key=value 格式)
        static QRegularExpression re("^([^=]+?)\\s*=\\s*(.+)$");
        QRegularExpressionMatch match = re.match(line);

        if (match.hasMatch()) {
            QString key = match.captured(1).trimmed();
            QString value = match.captured(2).trimmed();

            // 如果当前有节名称，构建完整参数名: section.key
            QString fullKey = key;
            if (!currentSection.isEmpty()) {
                fullKey = currentSection + "." + key;
            }

            qDebug() << "处理参数:" << fullKey << "=" << value;
            processParameter(fullKey, value);
        } else {
            qWarning() << "配置错误(行" << lineNumber << "):" << line;
        }
    }
    file.close();

    // 导入后验证配置
    if (!validateConfig()) {
        qWarning() << "导入的配置未通过验证";
        return false;
    }

    return true;
}

bool ISPConfig::exportConfig(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法创建配置文件:" << filePath;
        return false;
    }

    QTextStream out(&file);
    out << "# ISPConfig - ECUSTISP 参数配置\n";
    out << "# 生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n\n";

    // 按模块分组导出
    QMap<QString, QMap<QString, QString>> sectionMap;

    for (const auto& param : m_mapping) {
        QString section;
        QString paramName = param.name;

        // 提取章节名
        int dotPos = param.name.indexOf('.');
        if (dotPos != -1) {
            section = param.name.left(dotPos);
            paramName = param.name.mid(dotPos + 1);
        } else {
            section = "";//全局参数不放入任何节
        }

        // 获取参数值字符串
        QString valueStr;
        switch (param.type) {
        case ISP_PARAM_INT:
            valueStr = QString::number(*static_cast<int*>(param.ptr));
            break;
        case ISP_PARAM_DOUBLE:
            valueStr = QString::number(*static_cast<double*>(param.ptr), 'f', 4);
            break;
        case ISP_PARAM_U16:
            valueStr = QString::number(*static_cast<u16*>(param.ptr));
            break;
        case ISP_PARAM_INT_ARRAY: {
            int* arrayPtr = static_cast<int*>(param.ptr);
            QStringList values;
            for (int i = 0; i < param.size; i++) {
                values << QString::number(arrayPtr[i]);
            }
            valueStr = values.join(",");
            break;
        }
        case ISP_PARAM_U16_ARRAY: {
            u16* arrayPtr = static_cast<u16*>(param.ptr);
            QStringList values;
            for (int i = 0; i < param.size; i++) {
                values << QString::number(arrayPtr[i]);
            }
            valueStr = values.join(",");
            break;
        }
        }

        sectionMap[section][paramName] = valueStr;
    }

    // 写入文件
    for (auto sit = sectionMap.constBegin(); sit != sectionMap.constEnd(); ++sit) {
        out << "\n[" << sit.key() << "]\n";

        for (auto pit = sit.value().constBegin(); pit != sit.value().constEnd(); ++pit) {
            out << pit.key() << " = " << pit.value() << "\n";
        }
    }

    file.close();
    return true;
}



void ISPConfig::processParameter(const QString& key, const QString& value) {
    if (!m_paramMap.contains(key)) {
        qWarning() << "未知配置参数:" << key;
        return;
    }

    const ISPConfigParam& param = m_paramMap[key];
    bool conversionOk = true;

    switch (param.type) {
    case ISP_PARAM_INT:
        *static_cast<int*>(param.ptr) = value.toInt(&conversionOk);
        break;

    case ISP_PARAM_DOUBLE:
        *static_cast<double*>(param.ptr) = value.toDouble(&conversionOk);
        break;

    case ISP_PARAM_U16: {
        int intValue = value.toInt(&conversionOk);
        if (conversionOk) {
            *static_cast<u16*>(param.ptr) = static_cast<u16>(intValue);
        }
        break;
    }

    case ISP_PARAM_INT_ARRAY: {
        QStringList values = value.split(',', Qt::SkipEmptyParts);
        if (values.size() != param.size) {
            qWarning() << "数组大小不匹配:" << key
                       << "期望:" << param.size << "实际:" << values.size();
            return;
        }

        int* arrayPtr = static_cast<int*>(param.ptr);
        for (int i = 0; i < param.size; i++) {
            arrayPtr[i] = values[i].toInt(&conversionOk);
            if (!conversionOk) break;
        }
        break;
    }

    case ISP_PARAM_U16_ARRAY: {
        QStringList values = value.split(',', Qt::SkipEmptyParts);
        if (values.size() != param.size) {
            qWarning() << "数组大小不匹配:" << key
                       << "期望:" << param.size << "实际:" << values.size();
            return;
        }

        u16* arrayPtr = static_cast<u16*>(param.ptr);
        for (int i = 0; i < param.size; i++) {
            int intValue = values[i].toInt(&conversionOk);
            if (conversionOk) {
                arrayPtr[i] = static_cast<u16>(intValue);
            } else {
                break;
            }
        }
        break;
    }
    }

    if (!conversionOk) {
        qWarning() << "参数值转换失败:" << key << "=" << value;
    }
}

bool ISPConfig::validateConfig() {
    // // 基础参数验证
    // if (m_pipeline->isp_cfg_reg->input_width <= 0 ||
    //     m_pipeline->isp_cfg_reg->input_height <= 0) {
    //     qCritical() << "无效的图像尺寸";
    //     return false;
    // }

    // // 传感器位宽验证
    // if (m_pipeline->isp_cfg_reg->sensor_bits < 8 ||
    //     m_pipeline->isp_cfg_reg->sensor_bits > 16) {
    //     qCritical() << "传感器位宽必须在8-16位之间";
    //     return false;
    // }

    // // 增益参数验证
    // if (m_pipeline->dgain_cfg_reg->Gain_R < 0.1 ||
    //     m_pipeline->dgain_cfg_reg->Gain_R > 10.0) {
    //     qCritical() << "R增益超出有效范围(0.1-10.0)";
    //     return false;
    // }

    // // 黑电平偏移验证
    // for (int i = 0; i < 4; i++) {
    //     if (m_pipeline->blc_cfg_reg->parameter[i] < 0 ||
    //         m_pipeline->blc_cfg_reg->parameter[i] > 1023) {
    //         qCritical() << "黑电平偏移超出有效范围(0-1023)";
    //         return false;
    //     }
    // }

    // // 镜头阴影校正增益验证
    // for (int i = 0; i < 13*17; i++) {
    //     if (m_pipeline->lsc_cfg_reg->rGain[i] < 100 ||
    //         m_pipeline->lsc_cfg_reg->rGain[i] > 5000) {
    //         qCritical() << "LSC R增益超出有效范围(100-5000)";
    //         return false;
    //     }
    // }
    return true;
}
