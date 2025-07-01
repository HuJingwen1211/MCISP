#include "configurable_tab.h"

// 定义静态常量
const QMap<QString, QMap<QString, int>> ConfigurableTab::defaultParams = {
    {"BLC", {{"blc_r", 0}, {"blc_gr", 0}, {"blc_gb", 0}, {"blc_b", 0}}},
    {"LSC", {{"lsc_enable", 0}, {"lsc_center_x", 0}, {"lsc_center_y", 0}, {"lsc_radius", 0}, {"lsc_strength", 0}}},
    {"NR_RAW", {{"nr_raw_enable", 0}, {"nr_raw_strength", 0}, {"nr_raw_threshold", 0}}},
    {"AWB", {{"awb_enable", 0}, {"awb_r_gain", 1024}, {"awb_g_gain", 1024}, {"awb_b_gain", 1024}}},
    {"CCM", {{"ccm_enable", 0}, {"ccm_11", 1024}, {"ccm_12", 0}, {"ccm_13", 0}, {"ccm_21", 0}, {"ccm_22", 1024}, {"ccm_23", 0}, {"ccm_31", 0}, {"ccm_32", 0}, {"ccm_33", 1024}}},
    {"NR_YUV", {{"nr_yuv_enable", 0}, {"nr_yuv_strength", 0}, {"nr_yuv_threshold", 0}}}
};
