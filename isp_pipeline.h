#ifndef ISP_PIPELINE_H
#define ISP_PIPELINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <QString>
#include <QDebug>
#include <QObject>
#include <QMessageBox>
#include <QFile>
#include <QThread>
#include <QObject>

#define FILE_NAME_MAX 511  ////文件名大小

typedef uint8_t     u8  ;
typedef uint16_t    u16 ;
typedef uint32_t    u32 ;
typedef uint64_t    u64 ;
typedef int32_t    int32;

// struct which stores image in ISP pipeline
typedef struct {
    int	input_width     ;
    int	input_height    ;
    int pic_size        ;   // size of picture = input_width * input_height
    int	output_width    ;   // width of output
    int output_height   ;
    u16	*BAYER_DAT      ;   // data in bayer domain
    u16	*current_BAYER_DAT;
    u16 *RGB_DAT[3]     ;   // data in RGB domain, [0] for R, [1] for G, [2] for B
    u16 *YUV_DAT[3]     ;   // data in YUV domain, [0] for Y, [1] for U, [2] for V
} ISP_IMAGE;

// struct for top-level ISP pipeline
typedef struct {
    int input_width	;
    int input_height;
    int sensor_bits ;       // CIS's output pixel width
    int bayer_pattern;      // CIS's bayer pattern

    // char input_image_file[FILE_NAME_MAX];       // input image file name
    // char output_image_file[FILE_NAME_MAX];      // output image file name
    QString input_image_file;
    QString output_image_file;

    int yuv_10bit   ;       // if 1, output and store data of yuv in 10bits; if 0, it is 8bits
    int read_yuv_10bit   ;       // if 1, read data of yuv in 10bits; if 0, it is 8bits
    int dbg_level	;       // debug level to print different

}ISP_PARAM;
//////////enum/////////////////
typedef enum {
    BAYER       =   0,          // image in BAYER domain
    RGB         =   1,          // image in RGB domain
    YUV444      =   2,          // image in YUV444 format
    YUV422      =   3,          // image in YUV422 format
    YUV420      =   4,          // image in YUV420 format
} ISP_IMAGE_TYPE_t;         // stands for different image type

typedef enum {
    BAYER_R     =   0,      // The R channel in the Bayer domain image
    BAYER_GR    =   1,      // The GR channel ...
    BAYER_GB    =   2,      // The GB channel ...
    BAYER_B     =   3,      // The B channel ...

    RGB_R       =   0,      // Analogously to above
    RGB_G       =   1,
    RGB_B       =   2,

    YUV_Y       =   0,
    YUV_U       =   1,
    YUV_V       =   2

} ISP_IMAGE_COLOR_t;

////DGain reg
typedef struct{
    int enable;
    double Gain_R;
    double Gain_Gr;
    double Gain_Gb;
    double Gain_B;
}dgain_reg_t;
/////DPC reg
typedef struct{
    int     enable;
    int      mode;
}dpc_reg_t;
/////BLC reg
typedef struct{
    int     enable;
    int     parameter[4];
}blc_reg_t;
/////LSC reg
typedef struct{
    int     enable;
    int rGain[13*17];
    int GrGain[13*17];
    int GbGain[13*17];
    int bGain[13*17];
}lsc_reg_t;
/////NR_RAW reg
typedef struct{
    int     enable;
    int     h; //NLM smooth parameter
}NR_RAW_reg_t;

///AWB reg
typedef struct{
    int enable;
    u16  r_gain;     ////use 15bit quantify float num,3 fot integer,12 for float
    u16  g_gain;
    u16  b_gain;
}awb_reg_t;
//////WBC reg
typedef struct{
    int     enable;
    u16 m_nR;   //wbc gian of each channel
    u16 m_nGr;
    u16 m_nGb;
    u16 m_nB;
}wbc_reg_t;
////DMS reg
typedef struct{
    int     enable;
}dms_reg_t;
typedef struct  ////最终插值的rgb数据
{
    u16 r;
    u16 g;
    u16 b;
}pixel_t;
typedef struct ////计算中使用有符号数来暂存
{
    int32 r;
    int32 g;
    int32 b;
}pixelTmp_t;
/////CCM reg
typedef struct{
    int     enable;

    int     ccm_coeff[9];   ///10bit
    int     offset_out[3];  ///
}ccm_reg_t;

/////EE reg
typedef struct{
    int enable;
    int sharpen_param;
}sharpen_reg_t;

/////TM reg

/////Gamma reg
typedef struct{
    int     enable;
    double  gamma;
    int     gm_lut[129];
}gamma_register;

/////CSC reg
typedef struct{
    int     enable;
    int     offset_in[3];
    int     offset_out[3];
    int     coeff[9];
}csc_reg_t;
/////NR_YUV reg
typedef struct{
    int     enable;

    int     y_sigma2;
    int     y_inv_sigma2;
    int     uv_sigma2;
    int     uv_inv_sigma2;

    int     y_filt;
    int     uv_filt;
    int     y_inv_filt;
    int     uv_inv_filt;

    int     y_H2;
    int     y_invH2;
    int     uv_H2;
    int     uv_invH2;
}nryuv_reg_t;
/////YFC reg


//////////


//////////////////////isp对象///////////
class ISP_Pipeline : public QObject
{
    Q_OBJECT
public:
    ISP_IMAGE       *isp_image;
    ISP_PARAM       *isp_cfg_reg;

    dgain_reg_t     *dgain_cfg_reg;
    NR_RAW_reg_t    *nr_raw_cfg_reg;
    awb_reg_t       *awb_cfg_reg;
    wbc_reg_t       *wbc_cfg_reg;
    dms_reg_t       *dms_cfg_reg;
    gamma_register  *gamma_cfg_reg;
    ccm_reg_t       *ccm_cfg_reg;
    sharpen_reg_t   *sharpen_cfg_reg;
    dpc_reg_t       *dpc_cfg_reg;
    blc_reg_t       *blc_cfg_reg;
    lsc_reg_t       *lsc_cfg_reg;
    csc_reg_t       *csc_cfg_reg;
    nryuv_reg_t     *nryuv_cfg_reg;

    QImage          *raw_clor_image;    ////可视化raw图像
    QImage          *rgb_color_image;   ////可视化rgb图像
    ISP_IMAGE_TYPE_t current_pattern;  ///记录isp流程最后得到的图像格式
public:
    ISP_Pipeline();
    ~ISP_Pipeline();
    void clear_data();


    // 新增：异步处理函数
    void processAsync(const QString& imagePath);

signals:
    // 新增：处理完成信号
    void processingFinished();

private slots:
    // 新增：线程完成时的槽函数
    void onThreadFinished();

private:
    // 新增：处理线程
    QThread* m_thread;
    
    // 新增：在线程中执行的函数
    void processInThread();
};

/////Common
u16  get_pixel_value(const ISP_PARAM *isp_param, u16 *image, int row, int col);
void set_pixel_value(const ISP_PARAM *isp_param, u16 *image, int row, int col,const u16 value);
int load_raw_image(ISP_PARAM *isp_param, ISP_IMAGE *image);
u16  clip_to_sensorbits(int sensorbits,int value);
void copy_rawdata(ISP_PARAM *isp_param, ISP_IMAGE *image);

/////DGain
void dgian(ISP_IMAGE *isp_image,dgain_reg_t* dgain_cfg_reg,const ISP_PARAM *isp_param);
/////DPC
u16 min_Fun(u16 x, u16 y, u16 x1, u16 y1);
u16 dead_pixel_detection(u16 pixel[9]);
u16 median_filter(u16 pixel[9]);
u16 gradient_filter(u16 pixel[9]);
void dpc(ISP_IMAGE *isp_image, dpc_reg_t* dpc_cfg_reg ,const ISP_PARAM *isp_param);
/////BLC
void blc(ISP_IMAGE *isp_image, blc_reg_t* blc_cfg_reg ,const ISP_PARAM *isp_param);
/////LSC
int16_t Bilinear_Interpolation( u16 LeftTopGain, u16 LeftDownGain,u16 RightTopGain, u16 RightDownGain,u16 BlockWidthCount, u16 BlockHeightCount,u16 blockWidth_1, u16 blockHeight_1);
void lsc(ISP_IMAGE *isp_image,lsc_reg_t* lsc_cfg_reg, const ISP_PARAM *isp_param);
/////NR_RAW
void nr_raw_nlm(ISP_IMAGE *isp_image,NR_RAW_reg_t*nr_raw_cfg_reg,const ISP_PARAM *isp_param);
/////AWB
void awb_GW(ISP_IMAGE *isp_image,awb_reg_t* awb_cfg_reg,const ISP_PARAM *isp_param);     //gray world
void awb_maxRGB(ISP_IMAGE* isp_image,awb_reg_t* awb_cfg_reg,const ISP_PARAM *isp_param); //max-RGB
void awb_LWP(ISP_IMAGE* isp_image,awb_reg_t* awb_cfg_reg,const ISP_PARAM *isp_param);    ///LWP
/////WBC
void wbc(ISP_IMAGE* image, wbc_reg_t* wbc_cfg_reg,const ISP_PARAM *isp_param);
/////DMS
pixel_t demosaicInterpol(u16 rawWindow[5][5], u16 bayerPattern,ISP_PARAM *isp_param);
void demosaic_malvar(ISP_IMAGE *isp_image,dms_reg_t *dms_cfg_reg, ISP_PARAM *isp_param);
/////CCM
void ccm(ISP_IMAGE *isp_image,ccm_reg_t *ccm_cfg_reg,const ISP_PARAM *isp_param);
/////EE
void sharpen(ISP_IMAGE *isp_image,sharpen_reg_t *sharpen_cfg_reg,const ISP_PARAM *isp_param);
/////TM

/////Gamma
void generate_lut(double gamma, int gm_lut[]);
void gamma(ISP_IMAGE *isp_image, gamma_register *gamma_reg, const ISP_PARAM *isp_param);
/////CSC
void csc(ISP_IMAGE *isp_image,csc_reg_t* csc_cfg_reg,const ISP_PARAM *isp_param);
/////NR_YUV
void nr_yuv444(ISP_IMAGE *isp_image,nryuv_reg_t* nryuv_cfg_reg,const ISP_PARAM *isp_param);
/////YFC
#endif // ISP_PIPELINE_H
