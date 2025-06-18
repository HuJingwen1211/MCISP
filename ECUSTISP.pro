#-------------------------------------------------
#
# Project created by QtCreator 2023-05-15T17:29:51
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ECUSTISP
TEMPLATE = app
RC_ICONS =app.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG(release, debug|release) {
    # 在发布版本中禁用qDebug输出
    DEFINES += QT_NO_DEBUG_OUTPUT
}


SOURCES += \
    Link_tab/awb_tab.cpp \
    Link_tab/blc_tab.cpp \
    Link_tab/capture_tab.cpp \
    Link_tab/ccm_tab.cpp \
    Link_tab/crop_tab.cpp \
    Link_tab/csc_tab.cpp \
    Link_tab/debug.cpp \
    Link_tab/dms_tab.cpp \
    Link_tab/ee_tab.cpp \
    Link_tab/enable_tab.cpp \
    Link_tab/gamma_tab.cpp \
    Link_tab/gb_tab.cpp \
    Link_tab/lsc_tab.cpp \
    Link_tab/nr_raw_tab.cpp \
    Link_tab/nr_yuv_tab.cpp \
    Link_tab/scale_tab.cpp \
    Link_tab/tm_tab.cpp \
    Link_tab/yfc_tab.cpp \
        main.cpp \
        mainwindow.cpp \
    isp_tunning.cpp \
    isp_pipeline.cpp \
    my_graphicsview.cpp \
    Dialog/dgain_dialog.cpp \
    rgb_viewer.cpp \
    yuv_viewer.cpp \
    Dialog/nlm_nr_dialog.cpp \
    Dialog/ccm_dialog.cpp \
    Dialog/sharpen_dialog.cpp \
    Dialog/dpc_dialog.cpp \
    Dialog/blc_dialog.cpp \
    Dialog/lsc_dialog.cpp \
    Dialog/nryuv_dialog.cpp \
    Dialog/gamma_dialog.cpp \
    Dialog/awb_manual_dialog.cpp \
    Dialog/csc_dialog.cpp \
    link_board.cpp \
    Link_tab/test_tab.cpp \
    Link_tab/dpc_tab.cpp

HEADERS += \
    Link_tab/awb_tab.h \
    Link_tab/blc_tab.h \
    Link_tab/capture_tab.h \
    Link_tab/ccm_tab.h \
    Link_tab/crop_tab.h \
    Link_tab/csc_tab.h \
    Link_tab/debug.h \
    Link_tab/dms_tab.h \
    Link_tab/ee_tab.h \
    Link_tab/enable_tab.h \
    Link_tab/gamma_tab.h \
    Link_tab/gb_tab.h \
    Link_tab/lsc_tab.h \
    Link_tab/nr_raw_tab.h \
    Link_tab/nr_yuv_tab.h \
    Link_tab/scale_tab.h \
    Link_tab/tm_tab.h \
    Link_tab/yfc_tab.h \
        mainwindow.h \
    isp_tunning.h \
    isp_pipeline.h \
    my_graphicsview.h \
    Dialog/dgain_dialog.h \
    rgb_viewer.h \
    yuv_viewer.h \
    Dialog/nlm_nr_dialog.h \
    Dialog/ccm_dialog.h \
    Dialog/sharpen_dialog.h \
    Dialog/dpc_dialog.h \
    Dialog/blc_dialog.h \
    Dialog/lsc_dialog.h \
    Dialog/nryuv_dialog.h \
    Dialog/gamma_dialog.h \
    Dialog/awb_manual_dialog.h \
    Dialog/csc_dialog.h \
    link_board.h \
    Link_tab/test_tab.h \
    Link_tab/dpc_tab.h

FORMS += \
    Link_tab/awb_tab.ui \
    Link_tab/blc_tab.ui \
    Link_tab/capture_tab.ui \
    Link_tab/ccm_tab.ui \
    Link_tab/crop_tab.ui \
    Link_tab/csc_tab.ui \
    Link_tab/debug.ui \
    Link_tab/dms_tab.ui \
    Link_tab/ee_tab.ui \
    Link_tab/enable_tab.ui \
    Link_tab/gamma_tab.ui \
    Link_tab/gb_tab.ui \
    Link_tab/lsc_tab.ui \
    Link_tab/nr_raw_tab.ui \
    Link_tab/nr_yuv_tab.ui \
    Link_tab/scale_tab.ui \
    Link_tab/tm_tab.ui \
    Link_tab/yfc_tab.ui \
        mainwindow.ui \
    isp_tunning.ui \
    Dialog/dgain_dialog.ui \
    rgb_viewer.ui \
    yuv_viewer.ui \
    Dialog/nlm_nr_dialog.ui \
    Dialog/ccm_dialog.ui \
    Dialog/sharpen_dialog.ui \
    Dialog/dpc_dialog.ui \
    Dialog/blc_dialog.ui \
    Dialog/lsc_dialog.ui \
    Dialog/nryuv_dialog.ui \
    Dialog/gamma_dialog.ui \
    Dialog/awb_manual_dialog.ui \
    Dialog/csc_dialog.ui \
    link_board.ui \
    Link_tab/test_tab.ui \
    Link_tab/dpc_tab.ui

RESOURCES += \
    res.qrc

RESOURCES += $$PWD/Theme/dark/darkstyle.qrc
RESOURCES += $$PWD/Theme/light/lightstyle.qrc

DISTFILES +=
