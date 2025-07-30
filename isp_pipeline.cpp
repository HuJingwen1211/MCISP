#include "isp_pipeline.h"

ISP_Pipeline::ISP_Pipeline()
{
    isp_cfg_reg=new ISP_PARAM();
    isp_image=new ISP_IMAGE();
    dgain_cfg_reg=new dgain_reg_t();
    awb_cfg_reg=new awb_reg_t();
    wbc_cfg_reg=new wbc_reg_t();
    dms_cfg_reg=new dms_reg_t();
    nr_raw_cfg_reg=new NR_RAW_reg_t();
    gamma_cfg_reg=new gamma_register();
    ccm_cfg_reg=new ccm_reg_t();
    sharpen_cfg_reg=new sharpen_reg_t();
    dpc_cfg_reg=new dpc_reg_t();
    blc_cfg_reg=new blc_reg_t();
    lsc_cfg_reg=new lsc_reg_t();
    csc_cfg_reg=new csc_reg_t();
    nryuv_cfg_reg=new nryuv_reg_t();

    // 新增：初始化线程
    m_thread = nullptr;
}

//////在此处清理isp对象申请的动态内存，使用delete isp时候会自动调用
ISP_Pipeline::~ISP_Pipeline()
{
    // 新增：清理线程
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
    /////isp_image

    if(isp_image->BAYER_DAT!=nullptr){        ////这些先判断是否非空，否则会导致程序闪退
        delete [] isp_image->BAYER_DAT;
        isp_image->BAYER_DAT=nullptr;
    }
    if(isp_image->current_BAYER_DAT!=nullptr){
        delete [] isp_image->current_BAYER_DAT;
        isp_image->current_BAYER_DAT=nullptr;
    }

    for(int i=0;i<3;i++){
        if(isp_image->RGB_DAT[i]!=nullptr){
            delete [] isp_image->RGB_DAT[i];
            isp_image->RGB_DAT[i]=nullptr;
        }
        if(isp_image->YUV_DAT[i]!=nullptr){
            delete [] isp_image->YUV_DAT[i];
            isp_image->YUV_DAT[i]=nullptr;
        }
    }
    delete isp_image;
    isp_image=nullptr;
    /////ispcfg_reg
    delete isp_cfg_reg;
    isp_cfg_reg=nullptr;
    /////dgain_cfg_reg
    delete dgain_cfg_reg;
    dgain_cfg_reg=nullptr;
    /////nr_raw_cfg_reg
    delete nr_raw_cfg_reg;
    nr_raw_cfg_reg=nullptr;
    /////awb_cfg_reg
    delete awb_cfg_reg;
    awb_cfg_reg=nullptr;
    /////wbc_cfg_reg
    delete wbc_cfg_reg;
    wbc_cfg_reg=nullptr;
    /////dms_cfg_reg
    delete dms_cfg_reg;
    dms_cfg_reg=nullptr;
    /////gamma_cfg_reg
    delete gamma_cfg_reg;
    gamma_cfg_reg=nullptr;
    /////ccm_cfg_reg
    delete ccm_cfg_reg;
    ccm_cfg_reg=nullptr;
    /////sharpen_cfg_reg
    delete sharpen_cfg_reg;
    sharpen_cfg_reg=nullptr;
    /////dpc_cfg_reg
    delete dpc_cfg_reg;
    dpc_cfg_reg=nullptr;
    /////blc_cfg_reg
    delete blc_cfg_reg;
    blc_cfg_reg=nullptr;
    /////lsc_cfg_reg
    delete lsc_cfg_reg;
    lsc_cfg_reg=nullptr;
    /////csc_cfg_reg
    delete csc_cfg_reg;
    csc_cfg_reg=nullptr;
    /////nryuv_cfg_reg
    delete nryuv_cfg_reg;
    nryuv_cfg_reg=nullptr;
    /////raw_color_image
    if(raw_clor_image!=nullptr){
        delete raw_clor_image;
        raw_clor_image=nullptr;
    }
    ////rgb_color_image
    if(rgb_color_image!=nullptr){
      delete rgb_color_image;
      //delete rgb_color_image;
      rgb_color_image=nullptr;
    }
}

void ISP_Pipeline::processAsync(const QString& imagePath)
{
    // 设置输入文件路径
    isp_cfg_reg->input_image_file = imagePath;
    
    // 创建线程
    m_thread = QThread::create([this]() {
        this->processInThread();
    });
    
    // 连接信号槽
    connect(m_thread, &QThread::finished, this, &ISP_Pipeline::onThreadFinished);
    
    // 启动线程
    m_thread->start();
}

void ISP_Pipeline::processInThread()
{
    // 加载图像
    load_raw_image(isp_cfg_reg, isp_image);
    
    // 重要：先设置current_BAYER_DATA为原样
    copy_rawdata(isp_cfg_reg, isp_image);
    
    ///////////////////////Bayer Domain//////////////////////////////////////
    // 注意：不检查enable状态，直接调用所有算法
    dpc(isp_image, dpc_cfg_reg, isp_cfg_reg);
    blc(isp_image, blc_cfg_reg, isp_cfg_reg);
    lsc(isp_image, lsc_cfg_reg, isp_cfg_reg);
    nr_raw_nlm(isp_image, nr_raw_cfg_reg, isp_cfg_reg);
    dgian(isp_image, dgain_cfg_reg, isp_cfg_reg);
    
   ////AWB
    awb_GW(isp_image, awb_cfg_reg, isp_cfg_reg);


    // 重要：手动设置白平衡增益
    wbc_cfg_reg->m_nR = awb_cfg_reg->r_gain;
    wbc_cfg_reg->m_nGr = awb_cfg_reg->g_gain;
    wbc_cfg_reg->m_nGb = awb_cfg_reg->g_gain;
    wbc_cfg_reg->m_nB = awb_cfg_reg->b_gain;
    
    wbc(isp_image, wbc_cfg_reg, isp_cfg_reg);
    demosaic_malvar(isp_image, dms_cfg_reg, isp_cfg_reg);
    
    ///////////////////////////RGB Domain/////////////////////////////////////
    ccm(isp_image, ccm_cfg_reg, isp_cfg_reg);
    sharpen(isp_image, sharpen_cfg_reg, isp_cfg_reg);
    gamma(isp_image, gamma_cfg_reg, isp_cfg_reg);
    csc(isp_image, csc_cfg_reg, isp_cfg_reg);
    
    /////////////////////////////YUV Domain//////////////////////////////////
    nr_yuv444(isp_image, nryuv_cfg_reg, isp_cfg_reg);
    // 重要：根据实际执行的算法重新设置current_pattern
    // 这个逻辑应该和Item_check2reg()中的逻辑保持一致
    if (nryuv_cfg_reg->enable) {
        current_pattern = YUV444;  // NR YUV被启用，最终得到YUV数据
    } else if (csc_cfg_reg->enable) {
        current_pattern = YUV444;  // CSC被启用，最终得到YUV数据
    } else if (gamma_cfg_reg->enable || sharpen_cfg_reg->enable || ccm_cfg_reg->enable) {
        current_pattern = RGB;     // RGB域算法被启用，最终得到RGB数据
    } else if (dms_cfg_reg->enable) {
        current_pattern = RGB;     // Demosaic被启用，最终得到RGB数据
    } else {
        current_pattern = BAYER;   // 只有Bayer域算法被启用
    }
}

// 新增：线程完成时的槽函数
void ISP_Pipeline::onThreadFinished()
{
    emit processingFinished();
}

void ISP_Pipeline::clear_data()    ////只清理new出来的数据
{

    if(isp_image->BAYER_DAT!=nullptr){
        delete [] isp_image->BAYER_DAT;
        isp_image->BAYER_DAT=nullptr;
    }

    if(isp_image->current_BAYER_DAT!=nullptr){
        delete [] isp_image->current_BAYER_DAT;
        isp_image->current_BAYER_DAT=nullptr;
    }

    for(int i=0;i<3;i++){
        if(isp_image->RGB_DAT[i]!=nullptr){
            delete [] isp_image->RGB_DAT[i];
            isp_image->RGB_DAT[i]=nullptr;
        }
        if(isp_image->YUV_DAT[i]!=nullptr){
            delete [] isp_image->YUV_DAT[i];
            isp_image->YUV_DAT[i]=nullptr;
        }
    }

    if(raw_clor_image!=nullptr){  /////这里QImage类型居然是用free来释放空间,大坑啊啊啊啊，使用delete raw_clor_image会导致程序奔溃，TMD
        free(raw_clor_image);
        raw_clor_image=nullptr;
    }
    if(rgb_color_image!=nullptr){
      free(rgb_color_image);
      rgb_color_image=nullptr;
    }
}

///////////////////////////common function //////////////////////////////////////////////
//////get RAW image pixel value by (row col)//////////////////////////////////////////
u16 get_pixel_value(const ISP_PARAM *isp_param, u16 *image, int row, int col)
{
    if (row < 0) {
        row = 0;
    }else if (row > isp_param->input_height-1) {
        row = isp_param->input_height-1;
    }

    if (col < 0) {
        col = 0;
    }else if (col > isp_param->input_width-1) {
        col = isp_param->input_width-1;
    }

    return image[row * isp_param->input_width + col];
}

////////set pixel value by (row,col)/////////////////////////////////////////////////
void set_pixel_value(const ISP_PARAM *isp_param, u16 *image, int row, int col, const u16 value)
{
    image[row * isp_param->input_width + col] = value;
}

/////////load image by type/////////////////////////////////////////////////////////
int load_raw_image(ISP_PARAM *isp_param, ISP_IMAGE *image)
{
    // FILE* image_file = fopen(isp_param->input_image_file, "rb");
    // if (!image_file) {
    //     QMessageBox::critical(nullptr, "Error", "Can not open file!");
    //     //qDebug("Err: load_image: file not found! (%s)\n", isp_param->input_image_file);
    //     return -1;
    // }

    QFile image_file(isp_param->input_image_file);
    if (!image_file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        return -1;
    }

    qint64 fileSize = image_file.size();

    long expectedSize;

    if(isp_param->sensor_bits <= 8){
        expectedSize = isp_param->input_height*isp_param->input_width;
    }else{
        expectedSize = isp_param->input_height*isp_param->input_width*2;
    }
    if ( fileSize != expectedSize ){         //////判断读取的raw图像大小是否与输入的尺寸一致
        QMessageBox::critical(nullptr, "Error", "File size not match the width and height, please check and retry!");
        image_file.close();   //字节数不符合则关闭文件并退出
        return -1;
    }
    QDataStream stream(&image_file);
    stream.setByteOrder(QDataStream::LittleEndian); // 设置字节序（可选）

    //////打开文件和输入尺寸一致再申请动态内存空间
    image->BAYER_DAT=new u16[image->pic_size]();          /////init memory for raw image
    image->current_BAYER_DAT=new u16[image->pic_size]();  ////初始当前用于显示的raw

    image->RGB_DAT[RGB_R]=new u16[image->pic_size]();   //////初始RGB数据
    image->RGB_DAT[RGB_G]=new u16[image->pic_size]();
    image->RGB_DAT[RGB_B]=new u16[image->pic_size]();

    image->YUV_DAT[YUV_Y]=new u16[image->pic_size]();   //////初始YUV数据
    image->YUV_DAT[YUV_U]=new u16[image->pic_size]();
    image->YUV_DAT[YUV_V]=new u16[image->pic_size]();

    /////如果一切无误则从该文件中读取数据
    if(isp_param->sensor_bits <= 8){
        // fread(raw8_data, sizeof(u8), static_cast<size_t>(image->pic_size), image_file);
        for (int i = 0; i < image->pic_size; i++) {
            uint8_t u8_val;
            stream>>u8_val;
            image->BAYER_DAT[i] = static_cast<u16>(u8_val);
        }
    }else{
        for (int i=0;i<image->pic_size;i++){
            uint16_t u16_val;
            stream>>u16_val;
            image->BAYER_DAT[i] = u16_val;
        }
        // fread(image->BAYER_DAT, sizeof(u16), static_cast<size_t>(image->input_width*image->input_height) , image_file);
    }
    // fclose(image_file);
    image_file.close();
    return 0;
}


///////copy BAYER_DATA to current_BAYER_DATA for reset
void copy_rawdata(ISP_PARAM *isp_param, ISP_IMAGE *image)
{   u16 src_t;
    for(int row=0;row<image->input_height;row++){
        for(int col=0;col<image->input_width;col++){
            src_t=get_pixel_value(isp_param,image->BAYER_DAT,row,col);     ///read
            set_pixel_value(isp_param, image->current_BAYER_DAT, row,col,src_t);///write
        }
    }
}


////////////////////clip function/////////////////////int/////////////////////////////
u16 clip_to_sensorbits(int sensorbits,  int value)
{
    int maxval=(1<<sensorbits) -1; /////注意优先级顺序,要打括号表明意义
    u16 result;
    if(value<0){
        result=0;
    }else if (value>maxval) {
        result=maxval;
    }else{
        result=value;
    }
    return result;
}


///////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////Modules function///////////////////////////////////
//////////////////////////////////////////DGain////////////////////////////////////
void dgian(ISP_IMAGE *isp_image,dgain_reg_t* dgain_cfg_reg,const ISP_PARAM *isp_param){
    if (!dgain_cfg_reg->enable) {
        //qDebug("dgain is not enabled");
        return ;
    }
    double gain_t=1;
    u16 src_t;
    u16 dst_t;
    for(int row=0;row<isp_param->input_height;row++){
        for(int col=0;col<isp_param->input_width;col++){
            int dst_temp;
            src_t=get_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col);
            u8 pixel_pattern=(((row & 1) << 1) + (col & 1)) ^ isp_param->bayer_pattern;
            if(pixel_pattern==0){
                gain_t=dgain_cfg_reg->Gain_R;
            }else if(pixel_pattern==1){
                gain_t=dgain_cfg_reg->Gain_Gr;
            }else if(pixel_pattern==2){
                gain_t=dgain_cfg_reg->Gain_Gb;
            }else{
                gain_t=dgain_cfg_reg->Gain_B;
            }
            dst_temp=static_cast<u16>(std::round(src_t*gain_t));  ////四舍五入
            dst_t=clip_to_sensorbits(isp_param->sensor_bits,dst_temp);  /////钳位
            set_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col,dst_t);  ////保存值
        }
    }
}



//////////////////////////////////////////NR_RAW/////////////////////////////////////
void nr_raw_nlm(ISP_IMAGE *isp_image, NR_RAW_reg_t *nr_raw_cfg_reg, const ISP_PARAM *isp_param)
{
     if (!nr_raw_cfg_reg->enable) {
         //qDebug("NR_RAW is not enabled\n");
         return ;
     }

     int image_width     = isp_param->input_width;
     int image_height    = isp_param->input_height;

     int pixel_weight[7] = {255,93,34,12,5,2,1};

     u64 Eurp_dist = 0;
     u64 Eurp_temp = 0;

     int w = 0;
     int wmax = 0;
     u64 sum_weight = 0;
     u64 average = 0;

     for(int i = 5;i < image_height - 5;i++){
         for(int j = 5;j < image_width - 5;j++){
             wmax = 0;
             sum_weight = 0;
             average = 0;
             for(int m = i - 4;m < i + 5;m += 2){
                 for(int n = j - 4;n < j + 5;n += 2){
                     if(m == i && n == j){
                         continue;
                     }
                     Eurp_dist = (isp_image->current_BAYER_DAT[(i - 1) * image_width + (j - 1)] - isp_image->current_BAYER_DAT[(m - 1) * image_width + (n - 1)]) *
                                 (isp_image->current_BAYER_DAT[(i - 1) * image_width + (j - 1)] - isp_image->current_BAYER_DAT[(m - 1) * image_width + (n - 1)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i - 1) * image_width + (j)] - isp_image->current_BAYER_DAT[(m - 1) * image_width + (n)]) *
                                  (isp_image->current_BAYER_DAT[(i - 1) * image_width + (j)] - isp_image->current_BAYER_DAT[(m - 1) * image_width + (n)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i - 1) * image_width + (j + 1)] - isp_image->current_BAYER_DAT[(m - 1) * image_width + (n + 1)]) *
                                  (isp_image->current_BAYER_DAT[(i - 1) * image_width + (j + 1)] - isp_image->current_BAYER_DAT[(m - 1) * image_width + (n + 1)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i) * image_width + (j - 1)] - isp_image->current_BAYER_DAT[(m) * image_width + (n - 1)]) *
                                  (isp_image->current_BAYER_DAT[(i) * image_width + (j - 1)] - isp_image->current_BAYER_DAT[(m) * image_width + (n - 1)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i) * image_width + (j)] - isp_image->current_BAYER_DAT[(m) * image_width + (n)]) *
                                  (isp_image->current_BAYER_DAT[(i) * image_width + (j)] - isp_image->current_BAYER_DAT[(m) * image_width + (n)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i) * image_width + (j + 1)] - isp_image->current_BAYER_DAT[(m) * image_width + (n + 1)]) *
                                  (isp_image->current_BAYER_DAT[(i) * image_width + (j + 1)] - isp_image->current_BAYER_DAT[(m) * image_width + (n + 1)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i + 1) * image_width + (j - 1)] - isp_image->current_BAYER_DAT[(m + 1) * image_width + (n - 1)]) *
                                  (isp_image->current_BAYER_DAT[(i + 1) * image_width + (j - 1)] - isp_image->current_BAYER_DAT[(m + 1) * image_width + (n - 1)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i + 1) * image_width + (j)] - isp_image->current_BAYER_DAT[(m + 1) * image_width + (n)]) *
                                  (isp_image->current_BAYER_DAT[(i + 1) * image_width + (j)] - isp_image->current_BAYER_DAT[(m + 1) * image_width + (n)]);
                     Eurp_dist += (isp_image->current_BAYER_DAT[(i + 1) * image_width + (j + 1)] - isp_image->current_BAYER_DAT[(m + 1) * image_width + (n + 1)]) *
                                  (isp_image->current_BAYER_DAT[(i + 1) * image_width + (j + 1)] - isp_image->current_BAYER_DAT[(m + 1) * image_width + (n + 1)]);
                     Eurp_temp = Eurp_dist/9/nr_raw_cfg_reg->h/nr_raw_cfg_reg->h;
                     if(Eurp_temp > 6){
                         w = 0;
                     }else{
                         w = pixel_weight[Eurp_temp];
                     }
                     if(w > wmax){
                         wmax = w;
                     }
                     sum_weight += w;
                     average += w * isp_image->current_BAYER_DAT[m * image_width + n];
                 }
             }
             average += wmax * isp_image->current_BAYER_DAT[i * image_width +j];
             sum_weight += wmax;
             if(sum_weight > 0){
                 isp_image->current_BAYER_DAT[i * image_width + j] = average / sum_weight;
             }
         }
     }

}

///////////////////////////////////////////AWB////////////////////////////////////
void awb_GW(ISP_IMAGE* isp_image,awb_reg_t* awb_cfg_reg,const ISP_PARAM *isp_param) {

    if (!awb_cfg_reg->enable) {
        return ;
    }

    u64 r_total=1; ///avoid num/0
    u64 g_total=1;
    u64 b_total=1;

    //////default gain
    u16 r_gain=1;
    u16 g_gain=1;
    u16 b_gain=1;

    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;

    for (size_t row = 0; row < image_height; row++) {      //traverse all pixel to get total of each channel
        for (size_t col = 0; col < image_width; col++) {
            u16 pixel_value=0;
            u8  pixel_channel = (((row & 1) << 1) + (col & 1)) ^ isp_param->bayer_pattern;//judge pixel from r,gr,gb,b
            pixel_value=get_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col);  //traverse all image pixel data
            //pixel_value=pixel_value>>4; //////防止累加太多溢出
            if (pixel_channel == 0)
            {
                r_total = r_total + pixel_value;
            }
            else if (pixel_channel == 1)
            {
                g_total = g_total + pixel_value;
            }
            else if (pixel_channel == 2)
            {
                g_total = g_total + pixel_value;
            }
            else
            {
                b_total = b_total + pixel_value;
            }
        }
    }


    r_gain = (g_total<<11)/r_total;     ////g need /2,so it is <<11
    g_gain = 1<<12;
    b_gain = (g_total<<11)/b_total;

    awb_cfg_reg->r_gain = r_gain; //write the gain of each channel to the awb_cfg_reg
    awb_cfg_reg->g_gain = g_gain;
    awb_cfg_reg->b_gain = b_gain;

}



void awb_maxRGB(ISP_IMAGE* isp_image,awb_reg_t* awb_cfg_reg,const ISP_PARAM *isp_param){

    if (!awb_cfg_reg->enable) {
        return ;
    }

    u16 r_max=0;
    u16 g_max=0;
    u16 b_max=0;

    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;

    for (size_t row = 0; row < image_height; row++) {      //traverse all pixel to get total of each channel
        for (size_t col = 0; col < image_width; col++) {
            u16 pixel_value=0;
            u8  pixel_channel = (((row & 1) << 1) + (col & 1)) ^ isp_param->bayer_pattern;//judge pixel from r,gr,gb,b
            pixel_value=get_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col);  //traverse all image pixel data
            if (pixel_channel == 0)
            {
               if(pixel_value>r_max){   //get the max value of each channel
                r_max=pixel_value;
               }
            }
            else if (pixel_channel == 1)
            {
                if(pixel_value>g_max){
                g_max=pixel_value;
               }

            }
            else if (pixel_channel == 2)
            {
                if(pixel_value>g_max){
                g_max=pixel_value;
               }

            }
            else
            {
                if(pixel_value>b_max){
                b_max=pixel_value;
               }
            }
        }
    }

    awb_cfg_reg->r_gain =(g_max<<12)/r_max;   //caculate gain for each channel
    awb_cfg_reg->g_gain =1<<12;
    awb_cfg_reg->b_gain =(g_max<<12)/b_max;

}


void awb_LWP(ISP_IMAGE* isp_image,awb_reg_t* awb_cfg_reg,const ISP_PARAM *isp_param){

    if (!awb_cfg_reg->enable) {
        return ;
    }

    u16 max_reg[3][200];
    u16 block_size=50;

    u64 r_max_total=1;
    u64 g_max_total=1;
    u64 b_max_total=1;

    u16 r_gain=1;
    u16 g_gain=1;
    u16 b_gain=1;

    u16 row_block_size=isp_image->input_width/block_size;

    for(u16 row=0;row<isp_image->input_height;row++){
        for(u16 col=0;col<isp_image->input_width;col++){
            u16 pixel_value=0;
            u8  bayer_pattern = (((row & 1) << 1) + (col & 1)) ^ isp_param->bayer_pattern;//judge pixel from r,gr,gb,b
            pixel_value=get_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col);  //traverse all image pixel data

            u16 block_k=col/block_size;
            if(row%block_size<2 && col%block_size<2){
                    if (bayer_pattern == 0){     ///初始化r
                        max_reg[0][block_k]=pixel_value;
                    }

                    else if (bayer_pattern == 1 || bayer_pattern == 2){ ////初始化g
                        max_reg[1][block_k]=pixel_value;
                    }

                    else{          ////初始化b
                        max_reg[2][block_k]=pixel_value;
                    }

                }else{  ///不是位于每块开始的左上角四个则开始比较

                    if (bayer_pattern == 0)         //r
                    {
                        if(pixel_value > max_reg[0][block_k]){
                            max_reg[0][block_k]=pixel_value;
                        }
                    }
                    else if (bayer_pattern == 1 || bayer_pattern == 2)   //g
                    {
                        if(pixel_value > max_reg[1][block_k]){
                            max_reg[1][block_k]=pixel_value;
                        }
                    }
                    else                           //b
                    {
                        if(pixel_value > max_reg[2][block_k]){
                            max_reg[2][block_k]=pixel_value;
                        }
                    }
                }
                if((row+1)%block_size==0 && col==(isp_image->input_width-1)){  ///一行像素块结束,进行统计

                    for(u16 k=0; k<row_block_size; k++){   ////没有提升白块的纯度
                        r_max_total += max_reg[0][k];
                        g_max_total += max_reg[1][k];
                        b_max_total += max_reg[2][k];
                    }
                }
            }
        }

        r_gain=(g_max_total<<12)/r_max_total;
        g_gain=1<<12;
        b_gain=(g_max_total<<12)/b_max_total;

        awb_cfg_reg->r_gain = r_gain; //write the gain of each channel to the awb_cfg_reg
        awb_cfg_reg->g_gain = g_gain;
        awb_cfg_reg->b_gain = b_gain;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////WBC/////////////////////////////////////////////////////////////
void wbc(ISP_IMAGE* isp_image, wbc_reg_t* wbc_cfg_reg,const ISP_PARAM *isp_param){

    if (!wbc_cfg_reg->enable) {
        return ;
    }
    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;


    for (size_t row = 0; row < image_height; row++) {      //traverse all pixel to get total of each channel
        for (size_t col = 0; col < image_width; col++) {
            u16 pixel_value=0;
            u16 dst_pixel;    //the final output pixel value
            int dst_temp;
            u16 pixel_gain;  //every pixel gain
            u8  pixel_channel = (((row & 1) << 1) + (col & 1)) ^ isp_param->bayer_pattern;//judge pixel from r,gr,gb,b
            pixel_value=get_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col);  //traverse all image pixel data
            if (pixel_channel == 0)
            {
                pixel_gain=wbc_cfg_reg->m_nR;
            }
            else if (pixel_channel == 1)
            {
                pixel_gain=wbc_cfg_reg->m_nGr;
            }
            else if (pixel_channel == 2)
            {
                pixel_gain=wbc_cfg_reg->m_nGb;
            }
            else
            {
                pixel_gain=wbc_cfg_reg->m_nB;
            }
            ////注意应先右移再类型转换
            dst_temp=((pixel_value*pixel_gain+2048)>>12);  //turn double format to u16
            dst_pixel= clip_to_sensorbits(isp_param->sensor_bits,dst_temp); //constraint in 14bit
            set_pixel_value(isp_param, isp_image->current_BAYER_DAT, row,col,dst_pixel);//take care isp_image->BAYER_DAT
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////DMS////////////////////////////////////////////////////////////////
u16 dms_clip_to_sensorbits(int sensorbits,int32 value){
    int maxval=(1<<sensorbits) -1;   ////运算符的优先顺序，打上括号表明意思
    u16 result;
    if(value<0){
        result=0;
    }else if(value>maxval){
        result=maxval;
    }else
        result=value;
    return result;
}

pixel_t demosaicInterpol(u16 rawWindow[5][5], u16 bayerPattern,ISP_PARAM *isp_param)   //////当个像素的插值
{
    pixelTmp_t  pixTmp;
    pixel_t   pixel;
    ///根据像素的不同类型进行不同的插值流程
    if (bayerPattern == 0) ///R的插值
    {
        pixTmp.r = rawWindow[2][2];
        pixTmp.g = 4 * rawWindow[2][2] - rawWindow[0][2] - rawWindow[2][0] - rawWindow[4][2] - rawWindow[2][4] \
                   + 2 * (rawWindow[3][2] + rawWindow[2][3] + rawWindow[1][2] + rawWindow[2][1]);
        pixTmp.b = 6 * rawWindow[2][2] - 3 * (rawWindow[0][2] + rawWindow[2][0] + rawWindow[4][2] + rawWindow[2][4]) / 2 \
                   + 2 * (rawWindow[1][1] + rawWindow[1][3] + rawWindow[3][1] + rawWindow[3][3]);
        pixTmp.g = pixTmp.g / 8;
        pixTmp.b = pixTmp.b / 8;
    }
    else if (bayerPattern == 1) //Gr的插值
    {
        pixTmp.r = 5 * rawWindow[2][2] - rawWindow[2][0] - rawWindow[1][1] - rawWindow[3][1] - rawWindow[1][3] - rawWindow[3][3] - rawWindow[2][4] \
                   + (rawWindow[0][2] + rawWindow[4][2]) / 2 + 4 * (rawWindow[2][1] + rawWindow[2][3]);
        pixTmp.g = rawWindow[2][2];
        pixTmp.b = 5 * rawWindow[2][2] - rawWindow[0][2] - rawWindow[1][1] - rawWindow[1][3] - rawWindow[4][2] - rawWindow[3][1] - rawWindow[3][3] \
                   + (rawWindow[2][0] + rawWindow[2][4]) / 2 + 4 * (rawWindow[1][2] + rawWindow[3][2]);
        pixTmp.r = pixTmp.r / 8;
        pixTmp.b = pixTmp.b / 8;
    }
    else if (bayerPattern == 2) ///Gb的插值
    {
        pixTmp.r = 5 * rawWindow[2][2] - rawWindow[0][2] - rawWindow[1][1] - rawWindow[1][3] - rawWindow[4][2] - rawWindow[3][1] - rawWindow[3][3] \
                   + (rawWindow[2][0] + rawWindow[2][4]) / 2 + 4 * (rawWindow[1][2] + rawWindow[3][2]);
        pixTmp.g = rawWindow[2][2];
        pixTmp.b = 5 * rawWindow[2][2] - rawWindow[2][0] - rawWindow[1][1] - rawWindow[3][1] - rawWindow[1][3] - rawWindow[3][3] - rawWindow[2][4] \
                   + (rawWindow[0][2] + rawWindow[4][2]) / 2 + 4 * (rawWindow[2][1] + rawWindow[2][3]);
        pixTmp.r = pixTmp.r / 8;
        pixTmp.b = pixTmp.b / 8;
    }
    else   ////B的插值
    {
        pixTmp.r = 6 * rawWindow[2][2] - 3 * (rawWindow[0][2] + rawWindow[2][0] + rawWindow[4][2] + rawWindow[2][4]) / 2 \
                   + 2 * (rawWindow[1][1] + rawWindow[1][3] + rawWindow[3][1] + rawWindow[3][3]);
        pixTmp.g = 4 * rawWindow[2][2] - rawWindow[0][2] - rawWindow[2][0] - rawWindow[4][2] - rawWindow[2][4] \
                   + 2 * (rawWindow[3][2] + rawWindow[2][3] + rawWindow[1][2] + rawWindow[2][1]);
        pixTmp.b = rawWindow[2][2];
        pixTmp.r = pixTmp.r / 8;
        pixTmp.g = pixTmp.g / 8;
    }
    ///限值
    pixel.r = dms_clip_to_sensorbits(isp_param->sensor_bits,pixTmp.r);
    pixel.g = dms_clip_to_sensorbits(isp_param->sensor_bits,pixTmp.g);
    pixel.b = dms_clip_to_sensorbits(isp_param->sensor_bits,pixTmp.b);
//      pixel.r=pixTmp.r;
//      pixel.g=pixTmp.g;
//      pixel.b=pixTmp.b;


    return pixel; ///返回插值好后的三通道数据
}

void demosaic_malvar(ISP_IMAGE *isp_image,dms_reg_t *dms_cfg_reg,ISP_PARAM *isp_param)   /////插值算法
{
    if (!dms_cfg_reg->enable) {
        //qDebug("demosaic is not enabled\n");
        return ;
    }

    u16 srcPixel;
    u16 rawWindow[5][5]; ///最小处理窗口
    u8 i;

    u16 lineBuf[4][8192];   ///行缓存
    pixel_t outPixel;
    for(int row = 0; row < isp_param->input_height; row++){
        for(int col = 0; col < isp_param->input_width; col++){
            srcPixel = get_pixel_value(isp_param,isp_image->current_BAYER_DAT,row,col);  /////注意此处为current_BAYER_DAT
            int pixel_pos = row * isp_param->input_width + col;
                for(i = 0; i < 5; i++){
                    rawWindow[i][0] = rawWindow[i][1];
                    rawWindow[i][1] = rawWindow[i][2];
                    rawWindow[i][2] = rawWindow[i][3];
                    rawWindow[i][3] = rawWindow[i][4];  ///窗口向前移动一格像素区域
                }

                for(i = 0; i < 4; i++) {
                    rawWindow[i][4] = lineBuf[i][col];
                }
                rawWindow[4][4] = srcPixel;
                for(i = 0; i < 4; i++) {
                    lineBuf[i][col] = rawWindow[i+1][4];
                }
                ///已经缓存好最小处理窗口，则开始计算插值
                if((row > 3) && (col > 3)){
                    u16 bayerPattern = (((row & 1) << 1) + (col & 1))^isp_param->bayer_pattern;
                    outPixel = demosaicInterpol(rawWindow, bayerPattern,isp_param);
                    isp_image->RGB_DAT[0][pixel_pos]=outPixel.r;
                    isp_image->RGB_DAT[1][pixel_pos]=outPixel.g;
                    isp_image->RGB_DAT[2][pixel_pos]=outPixel.b;
                } else {              ////边界点未插值使用0填充
                    isp_image->RGB_DAT[0][pixel_pos]=0;
                    isp_image->RGB_DAT[1][pixel_pos]=0;
                    isp_image->RGB_DAT[2][pixel_pos]=0;
                }
        }

    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////GAMMA////////////////////////////////////////////////////////////


void generate_lut(double gamma, int gm_lut[]) {

    float curve_x[129];
    curve_x[0]=0;
    for (int i = 1; i < 129; i++)
    {
        curve_x[i] = curve_x[i-1]+ 0.0078125;
    };

    for (int i = 0; i < 129 ;i++)
    {
        gm_lut[i] = (int)(((float)pow(curve_x[i],gamma))*1023);
    };
}


void gamma(ISP_IMAGE *isp_image, gamma_register *gamma_reg, const ISP_PARAM *isp_param) {

    if (!gamma_reg->enable){
        return ;
    }

    int 	src_w[3];   ///暂存rgb计算中间结果
    int	    index;
    int	    x_val;
    int	    y_pos_0;
    int	    y_pos_1;
    int 	temp_0;
    int 	temp_1;

    generate_lut(1.0/gamma_reg->gamma, gamma_reg->gm_lut);  ////产生gamma分段线性值

    for (int row = 0; row < isp_param->input_height; row++) {
        for (int col = 0; col < isp_param->input_width; col++) {
            int pixel_pos = row * isp_param->input_width + col;   ////值的坐标
            src_w[0] = isp_image->RGB_DAT[0][pixel_pos];
            src_w[1] = isp_image->RGB_DAT[1][pixel_pos];
            src_w[2] = isp_image->RGB_DAT[2][pixel_pos];

            for (size_t i = 0; i < 3; i++) {	//xkISP的方法就是分段线性拟合gamma曲线
                index = src_w[i] >> (isp_param->sensor_bits-7);    // 每128一段，获取段起始坐标12-7=4
                x_val = src_w[i] & ((1<<(isp_param->sensor_bits-7))-1);  // x_pos是x-a的值
                int sensorbits = isp_param->sensor_bits;
                // y_pos_0 =   (gamma_reg->gm_lut[index]) * (1<<(isp_param->sensor_bits-10)); /////查找表查出来的值是10位的,需要与输入的比特位相同则要补偿
                // y_pos_1 =   (gamma_reg->gm_lut[index+1]) *(1<<(isp_param->sensor_bits-10));
                y_pos_0 =  sensorbits >= 10 ? (gamma_reg->gm_lut[index]) <<(sensorbits - 10) : (gamma_reg->gm_lut[index])>>(10 - sensorbits); /////查找表查出来的值是10位的,需要与输入的比特位相同则要补偿
                y_pos_1 =   sensorbits >= 10 ? (gamma_reg->gm_lut[index+1]) <<(sensorbits - 10) : (gamma_reg->gm_lut[index+1])>>(10 - sensorbits);

                temp_0  =   (y_pos_1 - y_pos_0) * x_val;   ///获取段内偏移值
                temp_1  =   y_pos_0 + (temp_0 >> 7);       ///获取整体偏移值，>>7是除以步长
                isp_image->RGB_DAT[i][pixel_pos] = clip_to_sensorbits(isp_param->sensor_bits,temp_1);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////CCM//////////////////////////////////////////////////////
void ccm(ISP_IMAGE *isp_image,ccm_reg_t *ccm_cfg_reg,const ISP_PARAM *isp_param) {

    if (!ccm_cfg_reg->enable){
        return ;
    }

    int src_w[3];
    int temp_0;
    int temp_1;

    for (int row=0; row<isp_param->input_height; row++){
        for (int col=0; col<isp_param->input_width; col++){
            int pixel_pos = row * isp_param->input_width + col;
            src_w[0] = isp_image->RGB_DAT[0][pixel_pos];
            src_w[1] = isp_image->RGB_DAT[1][pixel_pos];
            src_w[2] = isp_image->RGB_DAT[2][pixel_pos];

            for (int k = 0; k < 3; k++) {
                temp_0 = src_w[0] * ccm_cfg_reg->ccm_coeff[3*k] + src_w[1] * ccm_cfg_reg->ccm_coeff[3*k+1] + src_w[2] *ccm_cfg_reg->ccm_coeff[3*k+2];
                temp_1 = (temp_0 >> 10) + ccm_cfg_reg->offset_out[k];
                isp_image->RGB_DAT[k][pixel_pos] = clip_to_sensorbits(isp_param->sensor_bits,temp_1);	// 没看懂xk直接把21位截成14位的操作

            }

        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////Sharpen//////////////////////////////////////////////////////////
void sharpen(ISP_IMAGE *isp_image,sharpen_reg_t *sharpen_cfg_reg,const ISP_PARAM *isp_param){

    if (!sharpen_cfg_reg->enable){
        return ;
    }

    const int gaussKernel[5][5] = {     ////高斯卷积核函数,17bit位宽
        { 493 , 1969 , 3118 , 1969 , 493  },
        { 1969, 7853 , 12440, 7853 , 1969 },
        { 3118, 12440, 19704, 12440, 3118 },
        { 1969, 7853 , 12440, 7853 , 1969 },
        { 493 , 1969 , 3118 , 1969 , 493  }
    };

    int i, j, k, l;
    long long int r, g, b;
    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;
    for (i = 2; i <image_height - 2; i++) {
        for (j = 2; j < image_width	 - 2; j++) {
            r = g = b = 0;
            for (k = -2; k <= 2; k++) {
                for (l = -2; l <= 2; l++) {     /////(i,j)点像素值用8领域像素值的高斯加权得到
                    r += isp_image->RGB_DAT[0][(i + k) * image_width	 + j + l] * gaussKernel[k + 2][l + 2];
                    g += isp_image->RGB_DAT[1][(i + k) * image_width	 + j + l] * gaussKernel[k + 2][l + 2];
                    b += isp_image->RGB_DAT[2][(i + k) * image_width	 + j + l] * gaussKernel[k + 2][l + 2];
                    }
            }

            r =  isp_image->RGB_DAT[0][i * image_width + j]+sharpen_cfg_reg->sharpen_param *(isp_image->RGB_DAT[0][i * image_width + j] - (r >> 17));
            g =  isp_image->RGB_DAT[1][i * image_width + j]+sharpen_cfg_reg->sharpen_param *(isp_image->RGB_DAT[1][i * image_width + j] - (g >> 17));
            b =  isp_image->RGB_DAT[2][i * image_width + j]+sharpen_cfg_reg->sharpen_param *(isp_image->RGB_DAT[2][i * image_width + j] - (b >> 17));

            r=clip_to_sensorbits(isp_param->sensor_bits,r);
            g=clip_to_sensorbits(isp_param->sensor_bits,g);
            b=clip_to_sensorbits(isp_param->sensor_bits,b);

            isp_image->RGB_DAT[0][i * image_width + j] =  r ;
            isp_image->RGB_DAT[1][i * image_width + j] =  g ;
            isp_image->RGB_DAT[2][i * image_width + j] =  b ;
         }
     }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////DPC///////////////////////////////////////////////////////////////


////求四个数中的最小值
u16 min_Fun(u16 x, u16 y, u16 x1, u16 y1){
  u16 temp ,temp1 ,temp2;
  temp2 = (temp = x < y ? x : y) < (temp1 = x1 < y1 ? x1 : y1) ? temp : temp1;  /////求四个数的最小值
  return temp2;
}
/////坏点检测
u16 dead_pixel_detection(u16 pixel[9]){
    u16 temp[9];
    u16 flag_dead = 0;
    u16 diff = 0;
    u16 avg = 0;
    u16 thres[2] = {0};
    for (int k = 0; k < 9; k++)
        temp[k] = pixel[k];      /////
    u16 t = 0;
    for (int i = 0; i < 8; i++) {
        int f = 1;       ////如果某次比较没有换位置，那么就表明已经排序好了，不必再进行新一轮的判断了
        for (int j = 0; j < 8 - i; j++)
        {
            if (temp[j] > temp[j + 1])
            {
                t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
                f = 0;
            }
        }
        if (1 == f)
            break;
    }
    diff = temp[7] - temp[1];  ////次大点减次小点
    avg = (temp[0] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[8] - pixel[0]) / 6;  ////此处应该再减去temp7和temp1
    thres[0] = avg - diff;
    thres[1] = avg + diff;

    if(thres[0] < 0)
        thres[0] = 0;

    if(pixel[0] > thres[1] || pixel[0] < thres[0])   ///坏点检测
        flag_dead = 1;
    else
        flag_dead = 0;

    return flag_dead;
}

////中值滤波
u16 median_filter(u16 pixel[9]){
    /*bubble sort from min to max*/
    u16 temp[8];
    u16 result = 0;
    for (int k = 0; k < 8; k++)
        temp[k] = pixel[k+1];
    u16 t = 0;
    for (int i = 0; i < 7; i++) {
        int f = 1;
        for (int j = 0; j < 7 - i; j++)
        {
            if (temp[j] > temp[j + 1])
            {
                t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
                f = 0;
            }
        }
        if (1 == f)
            break;
    }

    result =(temp[3] + temp[4] ) >> 1;
    return result;
}

//////基于梯度的滤波
u16 gradient_filter(u16 pixel[9]){
    u16 dv, dh, ddr, ddl;   ////水平、垂直，\，/
    dv  = abs(2 * pixel[0] - pixel[2] - pixel[7]);
    dh  = abs(2 * pixel[0] - pixel[4] - pixel[5]);
    ddl = abs(2 * pixel[0] - pixel[1] - pixel[8]);
    ddr = abs(2 * pixel[0] - pixel[3] - pixel[6]);
    if(dv == min_Fun(dv, dh, ddl, ddr)){
        pixel[0] = (pixel[2] + pixel[7] + 1) >> 1;
    }
    else if (dh == min_Fun(dv, dh, ddl, ddr)){
        pixel[0] = (pixel[4] + pixel[5] + 1) >> 1;
    }
    else if (ddl == min_Fun(dv, dh, ddl, ddr)){
        pixel[0] = (pixel[1] + pixel[8] + 1) >> 1;
    }
    else{
        pixel[0] = (pixel[3] + pixel[6] + 1) >> 1;
    }

    return pixel[0];
}

////坏点校正主函数
void dpc(ISP_IMAGE *isp_image, dpc_reg_t* dpc_cfg_reg ,const ISP_PARAM *isp_param){

    if (!dpc_cfg_reg->enable) {
        return ;
    }

    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;
    int pixel_pos       = 0;

    u16 flag;
    u16 dst_t=0;
    u16 rawWindow[5][5] = {};
    u16 lineBuffer[4][4096];
    u16 pix[9];
    memset(pix, 0, sizeof(pix));

    for(size_t row = 0; row < image_height; row++){
        for (size_t col = 0; col < image_width; col++){

            pixel_pos = row * image_width + col;

            for(u8 i = 0; i < 5; i++){
                rawWindow[i][0] = rawWindow[i][1];
                rawWindow[i][1] = rawWindow[i][2];
                rawWindow[i][2] = rawWindow[i][3];
                rawWindow[i][3] = rawWindow[i][4];
            }

            for(u8 i = 0; i < 4; i++){
                rawWindow[i][4] = lineBuffer[i][col];
            }

            rawWindow[4][4] = isp_image->current_BAYER_DAT[pixel_pos];

            for(u8 i = 0; i < 4; i++){
                lineBuffer[i][col] = rawWindow[i+1][4];
            }

            /*dead pixel detecion could be modified*/
            if((row >= 2) && (col >= 2) && (row < image_height-2) && (col < image_width-2)){
                u8 dpc_pattern = (((row & 1) << 1) + (col & 1))^isp_param->bayer_pattern;
                pix[0] = rawWindow[2][2];  ////待检测的坏点

                if(dpc_pattern == 0 || dpc_pattern == 3){  /////R和B的周边8邻域
                    pix[1] = rawWindow[0][0];
                    pix[2] = rawWindow[0][2];
                    pix[3] = rawWindow[0][4];
                    pix[4] = rawWindow[2][0];
                    pix[5] = rawWindow[2][4];
                    pix[6] = rawWindow[4][0];
                    pix[7] = rawWindow[4][2];
                    pix[8] = rawWindow[4][4];

                    flag = dead_pixel_detection(pix);
                    if(flag == 1){
                        switch(dpc_cfg_reg->mode){
                        case 0:
                            dst_t = (pix[2] + pix[4] + pix[5] + pix[7]) >> 2;  ////均值校正
                            break;

                        case 1:
                            dst_t = median_filter(pix);   ////中值校正
                            break;

                        case 2:
                            dst_t = gradient_filter(pix);    ////基于梯度的校正
                            break;
                        }
                    } else{
                        dst_t = pix[0];
                    }

                } else{    ////G的周边8邻域
                    pix[1] = rawWindow[0][2];
                    pix[2] = rawWindow[1][1];
                    pix[3] = rawWindow[1][3];
                    pix[4] = rawWindow[2][0];
                    pix[5] = rawWindow[2][4];
                    pix[6] = rawWindow[3][1];
                    pix[7] = rawWindow[3][3];
                    pix[8] = rawWindow[4][2];

                    flag = dead_pixel_detection(pix);
                    if(flag == 1){
                        switch(dpc_cfg_reg->mode){
                        case 0:
                            dst_t = (pix[2] + pix[4] + pix[5] + pix[7]) >> 2;
                            break;

                        case 1:
                            dst_t = median_filter(pix);
                            break;

                        case 2:
                            dst_t = gradient_filter(pix);
                            break;
                        }
                    } else{
                    dst_t = pix[0];
                    }
                }

                isp_image->current_BAYER_DAT[pixel_pos] = dst_t;
            } else{
                isp_image->current_BAYER_DAT[pixel_pos] = 0;
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////BLC////////////////////////////////////////////////////////////////////////
void blc(ISP_IMAGE *isp_image, blc_reg_t* blc_cfg_reg ,const ISP_PARAM *isp_param){

    if (!blc_cfg_reg->enable) {
        return ;
    }

    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;
    int pixel_pos       = 0;
    int r = 0, b = 0, gr = 0, gb = 0;     ////在进行有符号与无符号数之间加减运算时需要注意

    for (size_t row = 0; row < image_height; row++){
        for (size_t col = 0; col < image_width; col++) {
            pixel_pos   =   row * image_width + col;
            switch (isp_param->bayer_pattern){
            /*rggb*/
            case 0:
                if((row % 2) == 0){
                if ((col % 2) == 0){
                    r = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[0];
                    r = clip_to_sensorbits(isp_param->sensor_bits, r);
                    isp_image->current_BAYER_DAT[pixel_pos] = r;
                }
                else{
                    gr = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[1];
                    gr = clip_to_sensorbits(isp_param->sensor_bits, gr);
                    isp_image->current_BAYER_DAT[pixel_pos] = gr;
                }
            }
            else{
                if ((col % 2) == 0){
                    gb = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[2];
                    gb = clip_to_sensorbits(isp_param->sensor_bits, gb);
                    isp_image->current_BAYER_DAT[pixel_pos] = gb;
                }
                else{
                    b = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[3];
                    b = clip_to_sensorbits(isp_param->sensor_bits, b);
                    isp_image->current_BAYER_DAT[pixel_pos] = b;
                }
            }
                break;

            /*grbg*/
            case 1:
                if((row % 2) == 0){
                    if ((col % 2) == 0){
                        gr = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[1];
                        gr = clip_to_sensorbits(isp_param->sensor_bits, gr);
                        isp_image->current_BAYER_DAT[pixel_pos] = gr;
                    }
                    else{
                        r = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[0];
                        r = clip_to_sensorbits(isp_param->sensor_bits, r);
                        isp_image->current_BAYER_DAT[pixel_pos] = r;
                    }
                }
                else{
                    if ((col % 2) == 0){
                        b = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[3];
                        b = clip_to_sensorbits(isp_param->sensor_bits, b);
                        isp_image->current_BAYER_DAT[pixel_pos] = b;
                    }
                    else{
                        gb = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[2];
                        gb = clip_to_sensorbits(isp_param->sensor_bits, gb);
                        isp_image->current_BAYER_DAT[pixel_pos] = gb;
                    }
                }
                break;

            /*gbrg*/
            case 2:
                if((row % 2) == 0){
                    if ((col % 2) == 0){
                        gb = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[2];
                        gb = clip_to_sensorbits(isp_param->sensor_bits, gb);
                        isp_image->current_BAYER_DAT[pixel_pos] = gb;
                    }
                    else{
                        b = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[3];
                        b = clip_to_sensorbits(isp_param->sensor_bits, b);
                        isp_image->current_BAYER_DAT[pixel_pos] = b;
                    }
                }
                else{
                    if ((col % 2) == 0){
                        r = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[0];
                        r = clip_to_sensorbits(isp_param->sensor_bits, r);
                        isp_image->current_BAYER_DAT[pixel_pos] = gb;
                    }
                    else{
                        gr = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[1];
                        gr = clip_to_sensorbits(isp_param->sensor_bits, gr);
                        isp_image->current_BAYER_DAT[pixel_pos] = gr;
                    }
                }
                break;

            /*bggr*/
            case 3:
                if((row % 2) == 0){
                    if ((col % 2) == 0){
                        b = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[3];
                        b = clip_to_sensorbits(isp_param->sensor_bits, b);
                        isp_image->current_BAYER_DAT[pixel_pos] = b;
                    }
                    else{
                        gb = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[2];
                        gb = clip_to_sensorbits(isp_param->sensor_bits, gb);
                        isp_image->current_BAYER_DAT[pixel_pos] = gb;
                    }
                }
                else{
                    if ((col % 2) == 0){
                        gr = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[1];
                        gr = clip_to_sensorbits(isp_param->sensor_bits, gr);
                        isp_image->current_BAYER_DAT[pixel_pos] = gr;
                    }
                    else{
                        r = isp_image->current_BAYER_DAT[pixel_pos] - blc_cfg_reg->parameter[0];
                        r = clip_to_sensorbits(isp_param->sensor_bits, b);
                        isp_image->current_BAYER_DAT[pixel_pos] = r;
                    }
                }
                break;

            default:
                break;
            }
        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////LSC//////////////////////////////////////////////////////////////////////////////

////双线性距离插值
int16_t Bilinear_Interpolation( u16 LeftTopGain, u16 LeftDownGain,
                                u16 RightTopGain, u16 RightDownGain,
                                u16 BlockWidthCount, u16 BlockHeightCount,
                                u16 blockWidth_1, u16 blockHeight_1){
    int16_t result;

    int16_t tmp_t = 0;
    int16_t tmp_d = 0;
    int16_t LT = LeftTopGain;
    int16_t LD = LeftDownGain;
    int16_t RT = RightTopGain;
    int16_t RD = RightDownGain;

    // Bilinear Interpolation compute optimization
    tmp_t = LT - (((LT - RT) * BlockWidthCount * blockWidth_1 + 128) >> 19);
    tmp_d = LD - (((LD - RD) * BlockWidthCount * blockWidth_1 + 128) >> 19);
    result = tmp_t - (((tmp_t - tmp_d) * BlockHeightCount * blockHeight_1 + 128) >> 15);

    return result;
}

////LSC 主函数
void lsc(ISP_IMAGE *isp_image,lsc_reg_t* lsc_cfg_reg ,const ISP_PARAM *isp_param){

    if (!lsc_cfg_reg->enable) {
        return ;
    }

    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;
    int pixel_pos       = 0;
    u16 pix             = 0;
    int32_t pix1        = 0;
    u16 pix3            = 0;
    u8  lsc_pattern         = isp_param->bayer_pattern;

    /****************************************************
    optimization of caluculations
    block_height_1 = 2^15 / block_height
    block_width_1 = 2^19 / block_width
    according to the Gain, 2^19 and 2^15 could be changed
    *****************************************************/
    u16 block_height    = (image_height - 1) / 12 + 1;;
    u16 block_width     = (image_width - 1)  / 16 + 1;
    u32 block_height_1  = 32768 / block_height;
    u16 block_width_1   = 524388 / block_width;

    static u8 block_width_count  = 0;
    static u8 block_height_count = 0;

    static u8 line_is_blue; //0: red; 1: blue
    static u16 gain_0_lt;
    static u16 gain_0_rt;
    static u16 gain_0_ld;
    static u16 gain_0_rd;
    static u16 gain_1_lt;
    static u16 gain_1_rt;
    static u16 gain_1_ld;
    static u16 gain_1_rd;
    static u16 gain_2_lt;
    static u16 gain_2_rt;
    static u16 gain_2_ld;
    static u16 gain_2_rd;
    static u16 gain_3_lt;
    static u16 gain_3_rt;
    static u16 gain_3_ld;
    static u16 gain_3_rd;
    static u16 gain_rGb_t_nxt;
    static u16 gain_rGb_d_nxt;
    static u16 gain_Grb_t_nxt;
    static u16 gain_Grb_d_nxt;
    static u8 block_count_rGr;
    static u8 block_count_Gbb;
    u16 gain_lt;
    u16 gain_rt;
    u16 gain_ld;
    u16 gain_rd;

                    gain_0_lt = lsc_cfg_reg->rGain[0];
                    gain_0_rt = lsc_cfg_reg->rGain[1];
                    gain_0_ld = lsc_cfg_reg->rGain[17];
                    gain_0_rd = lsc_cfg_reg->rGain[18];
                    gain_1_lt = lsc_cfg_reg->GrGain[0];
                    gain_1_rt = lsc_cfg_reg->GrGain[1];
                    gain_1_ld = lsc_cfg_reg->GrGain[17];
                    gain_1_rd = lsc_cfg_reg->GrGain[18];
                    gain_2_lt = lsc_cfg_reg->GbGain[0];
                    gain_2_rt = lsc_cfg_reg->GbGain[1];
                    gain_2_ld = lsc_cfg_reg->GbGain[17];
                    gain_2_rd = lsc_cfg_reg->GbGain[18];
                    gain_3_lt = lsc_cfg_reg->bGain[0];
                    gain_3_rt = lsc_cfg_reg->bGain[1];
                    gain_3_ld = lsc_cfg_reg->bGain[17];
                    gain_3_rd = lsc_cfg_reg->bGain[18];
                    gain_rGb_t_nxt = line_is_blue ? lsc_cfg_reg->GbGain[2] : lsc_cfg_reg->rGain[2];
                    gain_rGb_d_nxt = line_is_blue ? lsc_cfg_reg->GbGain[19] : lsc_cfg_reg->rGain[19];
                    gain_Grb_t_nxt = line_is_blue ? lsc_cfg_reg->bGain[2] : lsc_cfg_reg->GrGain[2];
                    gain_Grb_d_nxt = line_is_blue ? lsc_cfg_reg->bGain[19] : lsc_cfg_reg->GrGain[19];
                    line_is_blue   = (lsc_pattern > 1); //0: red; 1: blue

    for(size_t row = 0; row < image_height; row++){
        for (size_t col = 0; col < image_width; col++){
            pixel_pos   =   row * image_width + col;

            lsc_pattern = (((row & 1) << 1) + (col & 1)) ^ isp_param->bayer_pattern;
            if ( lsc_pattern == 0){       // RGGB
                gain_lt = gain_0_lt;
                gain_rt = gain_0_rt;
                gain_ld = gain_0_ld;
                gain_rd = gain_0_rd;
            }
            else if ( lsc_pattern == 1){  //GRBR
                gain_lt = gain_1_lt;
                gain_rt = gain_1_rt;
                gain_ld = gain_1_ld;
                gain_rd = gain_1_rd;
            }
            else if ( lsc_pattern == 2){  //GBRG
                gain_lt = gain_2_lt;
                gain_rt = gain_2_rt;
                gain_ld = gain_2_ld;
                gain_rd = gain_2_rd;
            }
            else if ( lsc_pattern == 3){  //BGGR
                gain_lt = gain_3_lt;
                gain_rt = gain_3_rt;
                gain_ld = gain_3_ld;
                gain_rd = gain_3_rd;
            }

            pix3 = isp_image->current_BAYER_DAT[pixel_pos];
            pix1 = pix3 * Bilinear_Interpolation( gain_lt, gain_ld, gain_rt, gain_rd,
                                                  block_width_count, block_height_count,
                                                  block_width_1, block_height_1);

            /* according to gain */
            pix1 = (pix1 + 1024) >> 11;

            pix =clip_to_sensorbits(isp_param->sensor_bits, pix1);
            isp_image->current_BAYER_DAT[pixel_pos] = pix;

            // update the gain
            if((col == 0) && (row == 0)){
                block_count_rGr = 2;
            }
            else if((col == image_width - 1) && (line_is_blue == 0)){
                if((block_height_count == block_height - 1) || (block_height_count == block_height -2)){
                    block_count_rGr += 2;
                }
                else{
                    block_count_rGr -= 15;
                }
            }
            else if((block_width_count == block_width - 1) && (line_is_blue == 0)){
                block_count_rGr++;
            }

            if((col == 0) && (row == 0)){
                block_count_Gbb = 2;
            }
            else if((col == image_width - 1) && (line_is_blue == 1))
            {
                if((block_height_count == block_height - 1) || (block_height_count == block_height -2)){
                    block_count_Gbb += 2;
                }
                else{
                    block_count_Gbb -= 15;
                }
            }
            else if((block_width_count == block_width - 1) && (line_is_blue == 1)){
                block_count_Gbb++;
            }

            if(block_width > 12) {
                if((block_width_count == block_width - 1) && (line_is_blue == 0)){
                    gain_0_lt = gain_0_rt;
                    gain_0_ld = gain_0_rd;
                    gain_0_rt = gain_rGb_t_nxt;
                    gain_0_rd = gain_rGb_d_nxt;
                    gain_1_lt = gain_1_rt;
                    gain_1_ld = gain_1_rd;
                    gain_1_rt = gain_Grb_t_nxt;
                    gain_1_rd = gain_Grb_d_nxt;
                }
                else if((line_is_blue == 1) && (col == 5) && (row != 0)){
                    gain_0_lt = lsc_cfg_reg->rGain[block_count_rGr - 2];
                }
                else if((line_is_blue == 1) && (col == 6) && (row != 0)){
                    gain_0_rt = lsc_cfg_reg->rGain[block_count_rGr - 1];
                }
                else if((line_is_blue == 1) && (col == 7) && (row != 0)){
                    gain_0_ld = lsc_cfg_reg->rGain[block_count_rGr + 15];
                }
                else if((line_is_blue == 1) && (col == 8) && (row != 0)){
                    gain_0_rd = lsc_cfg_reg->rGain[block_count_rGr + 16];
                }
                else if((line_is_blue == 1) && (col == 9) && (row != 0)){
                    gain_1_lt = lsc_cfg_reg->GrGain[block_count_rGr - 2];
                }
                else if((line_is_blue == 1) && (col == 10) && (row != 0)){
                    gain_1_rt = lsc_cfg_reg->GrGain[block_count_rGr - 1];
                }
                else if((line_is_blue == 1) && (col == 11) && (row != 0))
                {
                    gain_1_ld = lsc_cfg_reg->GrGain[block_count_rGr + 15];
                }
                else if((line_is_blue == 1) && (col == 12) && (row != 0)){
                    gain_1_rd = lsc_cfg_reg->GrGain[block_count_rGr + 16];
                }
            }

            if(block_width > 12) {
                if((block_width_count == block_width - 1) && (line_is_blue == 1)){
                    gain_2_lt = gain_2_rt;
                    gain_2_ld = gain_2_rd;
                    gain_2_rt = gain_rGb_t_nxt;
                    gain_2_rd = gain_rGb_d_nxt;
                    gain_3_lt = gain_3_rt;
                    gain_3_ld = gain_3_rd;
                    gain_3_rt = gain_Grb_t_nxt;
                    gain_3_rd = gain_Grb_d_nxt;
                }
                else if((line_is_blue == 0) && (col == 5) && (row != 0)){
                    gain_2_lt = lsc_cfg_reg->GbGain[block_count_Gbb - 2];
                }
                else if((line_is_blue == 0) && (col == 6) && (row != 0)){
                    gain_2_rt = lsc_cfg_reg->GbGain[block_count_Gbb - 1];
                }
                else if((line_is_blue == 0) && (col == 7) && (row != 0)){
                    gain_2_ld = lsc_cfg_reg->GbGain[block_count_Gbb + 15];
                }
                else if((line_is_blue == 0) && (col == 8) && (row != 0)){
                    gain_2_rd = lsc_cfg_reg->GbGain[block_count_Gbb + 16];
                }
                else if((line_is_blue == 0) && (col == 9) && (row != 0)){
                    gain_3_lt = lsc_cfg_reg->bGain[block_count_Gbb - 2];
                }
                else if((line_is_blue == 0) && (col == 10) && (row != 0)){
                    gain_3_rt = lsc_cfg_reg->bGain[block_count_Gbb - 1];
                }
                else if((line_is_blue == 0) && (col == 11) && (row != 0)){
                    gain_3_ld = lsc_cfg_reg->bGain[block_count_Gbb + 15];
                }
                else if((line_is_blue == 0) && (col == 12) && (row != 0)){
                    gain_3_rd = lsc_cfg_reg->bGain[block_count_Gbb + 16];
                }
            }

            if(block_width_count == 0){
                gain_rGb_t_nxt = (line_is_blue) ? lsc_cfg_reg->GbGain[block_count_Gbb] : lsc_cfg_reg->rGain[block_count_rGr];
            }
            else if(block_width_count == 1){
                gain_Grb_t_nxt = (line_is_blue) ? lsc_cfg_reg->bGain[block_count_Gbb] : lsc_cfg_reg->GrGain[block_count_rGr];
            }
            else if(block_width_count == 2){
                gain_rGb_d_nxt = (line_is_blue) ? lsc_cfg_reg->GbGain[block_count_Gbb + 17] : lsc_cfg_reg->rGain[block_count_rGr + 17];
            }
            else if(block_width_count == 3){
                gain_Grb_d_nxt = (line_is_blue) ? lsc_cfg_reg->bGain[block_count_Gbb + 17] : lsc_cfg_reg->GrGain[block_count_rGr + 17];
            }

            if((col == image_width - 1) && (block_height_count == block_height - 1)){
                block_height_count = 0;
            }
            else if(col == image_width - 1){
                block_height_count++;
            }

            if((block_width_count == block_width - 1) || (col == image_width - 1)){
                block_width_count = 0;
            }
            else{
                block_width_count++;
            }

            //update line is blue
            if(col == image_width - 1){
                line_is_blue = (line_is_blue == 0);
            }

        }
    }

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////CSC///////////////////////////////////////////////////////////////////////
// BT2020
void csc(ISP_IMAGE *isp_image,csc_reg_t *csc_cfg_reg,const ISP_PARAM *isp_param) {

    if (!csc_cfg_reg->enable) {
        return ;
    }

    int image_width     =   isp_image->input_width;
    int image_height    =   isp_image->input_height;
    int sensor_bits     =   isp_param->sensor_bits;
    int pixel_pos       =   0;
    int temp_0          =   0;
    int temp_1          =   0;
    u16 pix_rgb[3]      =   {0};
    u16 pix_rgb_tmp[3]  =   {0};
    u16 pix_yuv[3]      =   {0};

    for (size_t row = 0; row < image_height; row++) {
        for (size_t col = 0; col < image_width; col++) {
            pixel_pos   =   row * image_width + col;

            for (size_t i = 0; i < 3; i++) {    ////将输入的图像转为10bit位宽
                pix_rgb_tmp[i]  =   isp_image->RGB_DAT[i][pixel_pos] + csc_cfg_reg->offset_in[i];
                u16 rgb_trans = sensor_bits>10?pix_rgb_tmp[i]>>(sensor_bits-10):pix_rgb_tmp[i]<<(10-sensor_bits);
                if (rgb_trans > 1023) {
                    pix_rgb[i]  =   1023;
                } else {
                    pix_rgb[i]  =   rgb_trans;  ///转为10bit
                }
            }

            for (size_t i = 0; i < 3; i++) {
                // the coeff is quantized to 8 bits
                temp_0  =   pix_rgb[0] * csc_cfg_reg->coeff[3*i] + pix_rgb[1] * csc_cfg_reg->coeff[3*i+1] + pix_rgb[2] * csc_cfg_reg->coeff[3*i+2];
                temp_1  =   ((temp_0+128)>> 8) + csc_cfg_reg->offset_out[i];  /////coeff是8位的
                pix_yuv[i]  =clip_to_sensorbits(10,temp_1);       ///钳位到[0,1023]
                ///在10bit位宽下转换后转回原位宽
                uint16_t origin_bityuv = sensor_bits>10? pix_yuv[i]<<(sensor_bits-10) : pix_yuv[i]>>(10-sensor_bits);
                // isp_image->YUV_DAT[i][pixel_pos] = pix_yuv[i];    // output is 10 bits
                isp_image->YUV_DAT[i][pixel_pos] = origin_bityuv;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////NR_YUV////////////////////////////////////////////////////////////////
u32 nr_yuv_weight2_clip(int count){
    if(count > 15)
        count = 15;
    else if(count < 0)
        count = 0;
    return (u32)count;
}

u16 nr_yuv_dif_abs(u16 a, u16 b) {
    u16 dif;
    if (a > b)
        dif = a - b;
    else
        dif = b - a;
    return dif;
}


u16 nr_yuv_nlm(u16 window[9][9], u16 sigma2, u16 H2, u32 invH2, int nlm_en) {
    u16 weight_1[8]={255,226,200,176,156,138,122,108};
    u16 weight_2[16]={88,72,59,48,39,32,26,21,17,14,11,9,7,5,2,0};
    u16 total_q;

    u32 diff;
    u32 diff_1;
    u32 diff_2;
    u32 diff_3;
    u32 diff_tmp;
    u16 weight = 0, max_weight = 0;
    u16 total_weight =0;
    u32 total_value = 0;

    for (size_t i = 1; i < 8; i++) {
        for (size_t j = 1; j < 8; j++) {

            if (i != 4 || j != 4) {
                u16 dis_1   =   0;
                u32 dis_11  =   0;
                dis_1   =   nr_yuv_dif_abs(window[i-1][j-1], window[3][3]);
                dis_11  =   dis_1 * dis_1;

                u16 dis_2   =   0;
                u32 dis_22  =   0;
                dis_2   =   nr_yuv_dif_abs(window[i-1][j], window[3][4]);
                dis_22  =   dis_2 * dis_2;

                u16 dis_3   =   0;
                u32 dis_33  =   0;
                dis_3   =   nr_yuv_dif_abs(window[i-1][j+1], window[3][5]);
                dis_33  =   dis_3 * dis_3;

                u16 dis_4   =   0;
                u32 dis_44  =   0;
                dis_4   =   nr_yuv_dif_abs(window[i][j-1], window[4][3]);
                dis_44  =   dis_4 * dis_4;

                u16 dis_5   =   0;
                u32 dis_55  =   0;
                dis_5   =   nr_yuv_dif_abs(window[i][j], window[4][4]);
                dis_55  =   dis_5 * dis_5;

                u16 dis_6   =   0;
                u32 dis_66  =   0;
                dis_6   =   nr_yuv_dif_abs(window[i][j+1], window[4][5]);
                dis_66  =   dis_6 * dis_6;

                u16 dis_7   =   0;
                u32 dis_77  =   0;
                dis_7   =   nr_yuv_dif_abs(window[i+1][j-1], window[5][3]);
                dis_77  =   dis_7 * dis_7;

                u16 dis_8   =   0;
                u32 dis_88  =   0;
                dis_8   =   nr_yuv_dif_abs(window[i+1][j], window[5][4]);
                dis_88  =   dis_8 * dis_8;

                u16 dis_9   =   0;
                u32 dis_99  =   0;
                dis_9   =   nr_yuv_dif_abs(window[i+1][j+1], window[5][5]);
                dis_99  =   dis_9 * dis_9;

                diff_1  =   dis_11 + dis_22 + dis_33;
                diff_2  =   dis_44 + dis_55 + dis_66;
                diff_3  =   dis_77 + dis_88 + dis_99;

                // it made an approximation here
                diff    =   (diff_1+diff_2+diff_3) >> 3;

                if (diff < 2*sigma2) {
                    diff = 0;
                }else {
                    diff = diff - 2*sigma2;
                }

                u32 count = 0;
                if (H2 == 0) {
                    weight = 0;
                }else if (diff <= H2) {
                    diff_tmp    = 7 * diff;
                    count       = (diff_tmp * invH2)>>14;
                    weight      = weight_1[count];
                }else {
                    diff_tmp    = 5 * diff;
                    count       = (diff_tmp * invH2)>>14;
                    count       = nr_yuv_weight2_clip(((int)count-5));
                    weight      = weight_2[count];
                }

                if(weight > max_weight){
                    max_weight = weight;
                }
                total_weight    = total_weight + weight;
                total_value     = total_value + window[i][j] * weight;
            }
        }
    }

    total_weight    = total_weight + max_weight;
    total_value     = total_value + window[4][4] * max_weight;

    if(total_weight==0 || nlm_en==0) {
        return window[4][4];
    }else {
        total_q = total_value / total_weight;
        return clip_to_sensorbits(10,total_q);
    }
}

void nr_yuv444(ISP_IMAGE *isp_image, nryuv_reg_t *nryuv_cfg_reg, const ISP_PARAM *isp_param)
{

    if (!nryuv_cfg_reg->enable) {
        return ;
    }

    nryuv_cfg_reg->y_inv_sigma2=static_cast<int>((1.0/ nryuv_cfg_reg->y_sigma2)*16384);
    nryuv_cfg_reg->uv_inv_sigma2=static_cast<int>((1.0/ nryuv_cfg_reg->uv_sigma2)*16384);

    nryuv_cfg_reg->y_H2      =   (nryuv_cfg_reg->y_filt * nryuv_cfg_reg->y_filt * nryuv_cfg_reg->y_sigma2 + 255) >> 8;
    nryuv_cfg_reg->y_invH2   =   (nryuv_cfg_reg->y_inv_filt * nryuv_cfg_reg->y_inv_filt * nryuv_cfg_reg->y_inv_sigma2 + 3) >> 2;
    nryuv_cfg_reg->uv_H2     =   (nryuv_cfg_reg->uv_filt * nryuv_cfg_reg->uv_filt * nryuv_cfg_reg->uv_sigma2 + 255) >> 8;
    nryuv_cfg_reg->uv_invH2  =   (nryuv_cfg_reg->uv_inv_filt * nryuv_cfg_reg->uv_inv_filt * nryuv_cfg_reg->uv_inv_sigma2 + 3) >> 2;

    int image_width     = isp_image->input_width;
    int image_height    = isp_image->input_height;
    int pixel_pos       = 0;
    u16 y_window[9][9];
    u16 u_window[9][9];
    u16 v_window[9][9];
    u16 y_line_buf[8][8192];
    u16 u_line_buf[8][8192];
    u16 v_line_buf[8][8192];
    // the yuv image data after NR operation,
    // this is to avoid interfering with the process of nlm.
    u16* yuv_after_nr[3];

    // u16 line_buf[4][8192];
    yuv_after_nr[0] =   (u16*)malloc(image_height*image_width*sizeof(u16));
    yuv_after_nr[1] =   (u16*)malloc(image_height*image_width*sizeof(u16));
    yuv_after_nr[2] =   (u16*)malloc(image_height*image_width*sizeof(u16));

    memset(y_window, 0, sizeof(y_window));
    memset(u_window, 0, sizeof(u_window));
    memset(v_window, 0, sizeof(v_window));
    memset(y_line_buf, 0, sizeof(y_line_buf));
    memset(u_line_buf, 0, sizeof(u_line_buf));
    memset(v_line_buf, 0, sizeof(v_line_buf));
    memcpy(yuv_after_nr[0], isp_image->YUV_DAT[0], image_height*image_width*sizeof(u16));
    memcpy(yuv_after_nr[1], isp_image->YUV_DAT[1], image_height*image_width*sizeof(u16));
    memcpy(yuv_after_nr[2], isp_image->YUV_DAT[2], image_height*image_width*sizeof(u16));

    // basic bilinear interpolation
    for (size_t row = 0; row < image_height; row++) {
        for (size_t col = 0; col < image_width; col++) {
            pixel_pos   =   row * image_width + col;

            window_move:
            for (size_t i = 0; i < 9; i++) {
                for (size_t j = 0; j < 8; j++) {    // notice that it's 8! not 9.
                    y_window[i][j]  =   y_window[i][j+1];
                    u_window[i][j]  =   u_window[i][j+1];
                    v_window[i][j]  =   v_window[i][j+1];
                }
            }

            window_move2:
            for (size_t i = 0; i < 8; i++) {    // notice that it's 8! not 9.
                y_window[i][8]  =   y_line_buf[i][col];
                u_window[i][8]  =   u_line_buf[i][col];
                v_window[i][8]  =   v_line_buf[i][col];
            }
            y_window[8][8]  =   isp_image->YUV_DAT[0][pixel_pos];
            u_window[8][8]  =   isp_image->YUV_DAT[1][pixel_pos];
            v_window[8][8]  =   isp_image->YUV_DAT[2][pixel_pos];

            lines_read:
            for(size_t i = 0; i < 7; i++){
                y_line_buf[i][col] = y_window[i+1][8];
                u_line_buf[i][col] = u_window[i+1][8];
                v_line_buf[i][col] = v_window[i+1][8];
            }
            y_line_buf[7][col]  = isp_image->YUV_DAT[0][pixel_pos];
            u_line_buf[7][col]  = isp_image->YUV_DAT[1][pixel_pos];
            v_line_buf[7][col]  = isp_image->YUV_DAT[2][pixel_pos];

            int nlm_en  =   (row > 7) && (col > 7);
            if (nlm_en) {
                int pixel_dis = 4*image_width+4;
                yuv_after_nr[0][pixel_pos-pixel_dis] =   nr_yuv_nlm(y_window, nryuv_cfg_reg->y_sigma2,  nryuv_cfg_reg->y_H2,  nryuv_cfg_reg->y_invH2,  nlm_en);
                yuv_after_nr[1][pixel_pos-pixel_dis] =   nr_yuv_nlm(u_window, nryuv_cfg_reg->uv_sigma2, nryuv_cfg_reg->uv_H2, nryuv_cfg_reg->uv_invH2, nlm_en);
                yuv_after_nr[2][pixel_pos-pixel_dis] =   nr_yuv_nlm(v_window, nryuv_cfg_reg->uv_sigma2, nryuv_cfg_reg->uv_H2, nryuv_cfg_reg->uv_invH2, nlm_en);
            }
        }
    }

    memcpy(isp_image->YUV_DAT[0], yuv_after_nr[0], image_height*image_width*sizeof(u16));
    memcpy(isp_image->YUV_DAT[1], yuv_after_nr[1], image_height*image_width*sizeof(u16));
    memcpy(isp_image->YUV_DAT[2], yuv_after_nr[2], image_height*image_width*sizeof(u16));

    free(yuv_after_nr[0]);
    free(yuv_after_nr[1]);
    free(yuv_after_nr[2]);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






