#include "isp_tunning.h"
#include "ui_isp_tunning.h"
#include "stdlib.h"
#include <QFileDialog>
#include <QDebug>
Tunning_Tab::Tunning_Tab(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Tunning_Tab)
{
    ui->setupUi(this);
    isp=new ISP_Pipeline();        ////初始化isp对象,调用构造函数分配空间
    init_modules_reg();
    view=new My_GraphicsView();

//    workerThread = new QThread;
//    //moveToThread(workerThread);
//    connect(workerThread, &QThread::started, this, &Tunning_Tab::Run_Pipeline);
//    QObject::connect(this, &Tunning_Tab::finished, [&]() {  ////绑定运行完成信号与操作函数
//        ui->btn_isp_run->setEnabled(true);
//        qDebug()<<"Finish Pipeline";
//     });
}

Tunning_Tab::~Tunning_Tab()    ///////释放动态指针所指向的内存空间
{
    delete ui;
    delete view;
    delete isp;
//    workerThread->quit();
//    workerThread->wait();
//    delete workerThread;

}


/////////////////初始化模块寄存器值
void Tunning_Tab::init_modules_reg()
{
    ////DGain reg
    isp->dgain_cfg_reg->Gain_R=1.0;
    isp->dgain_cfg_reg->Gain_Gr=1.0;
    isp->dgain_cfg_reg->Gain_Gb=1.0;
    isp->dgain_cfg_reg->Gain_B=1.0;
    ////DPC reg
    isp->dpc_cfg_reg->mode=1;  ///初始为中值校正
    ////BLC reg
    isp->blc_cfg_reg->parameter[0]=0;
    isp->blc_cfg_reg->parameter[1]=0;
    isp->blc_cfg_reg->parameter[2]=0;
    isp->blc_cfg_reg->parameter[3]=0;
    ////LSC reg
    u16 rgain[13*17]={2776,2684,2728,2690,2640,2580,2522,2474,2504,2550,2610,2682,2778,2862,2970,2920,3356,2759,2687,2690,2633,2555,2497,2429,2395,2397,2449,2519,2625,2719,2813,2872,2956,3108,2825,2707,2678,2603,2521,2464,2376,2344,2333,2399,2469,2587,2695,2787,2891,2952,3129,2846,2720,2658,2574,2496,2418,2332,2254,2242,2318,2428,2552,2654,2764,2878,2978,3138,2862,2735,2665,2568,2481,2385,2268,2155,2123,2218,2364,2533,2645,2784,2893,2985,3189,2897,2767,2679,2577,2485,2379,2216,2099,2061,2165,2323,2534,2653,2801,2911,3027,3216,2952,2808,2694,2642,2506,2392,2226,2100,2074,2174,2354,2566,2696,2850,2962,3090,3272,3010,2864,2795,2691,2578,2473,2323,2229,2205,2307,2453,2651,2781,2917,3046,3166,3357,3091,2933,2871,2780,2678,2587,2469,2391,2384,2479,2599,2740,2881,3014,3134,3242,3427,3172,3024,2962,2886,2802,2696,2614,2574,2580,2662,2746,2882,2998,3132,3240,3336,3478,3201,3095,3057,2999,2915,2829,2751,2715,2728,2799,2891,3027,3140,3255,3327,3390,3557,3218,3139,3128,3080,3029,2955,2887,2856,2869,2939,3013,3143,3252,3349,3409,3411,3654,3428,3176,3238,3210,3178,3112,3070,3052,3088,3112,3206,3324,3396,3466,3532,3542,4158 };
    u16 grgain[13*17]={2718,2620,2664,2622,2570,2510,2458,2434,2440,2480,2520,2598,2686,2748,2836,2808,3224,2683,2631,2631,2579,2503,2447,2380,2357,2351,2390,2450,2541,2626,2715,2768,2831,2957,2743,2657,2608,2546,2466,2406,2335,2297,2291,2338,2401,2498,2593,2695,2776,2839,2959,2774,2664,2596,2516,2446,2360,2290,2226,2204,2270,2364,2472,2572,2674,2778,2840,2996,2790,2671,2595,2513,2427,2335,2229,2145,2111,2187,2310,2457,2557,2672,2779,2864,3022,2818,2695,2607,2529,2426,2332,2197,2098,2059,2143,2278,2456,2561,2691,2792,2891,3057,2850,2730,2636,2554,2446,2352,2208,2100,2074,2150,2300,2476,2604,2722,2832,2938,3092,2901,2786,2699,2613,2507,2411,2291,2205,2185,2267,2386,2551,2669,2795,2903,3005,3159,2961,2853,2773,2688,2597,2507,2403,2349,2339,2410,2514,2637,2755,2881,2983,3069,3225,3022,2928,2862,2784,2698,2610,2524,2492,2500,2562,2634,2752,2846,2978,3060,3150,3274,3061,2961,2932,2880,2791,2722,2652,2613,2621,2683,2755,2864,2963,3068,3149,3202,3347,3072,3007,2979,2947,2899,2838,2767,2745,2752,2803,2869,2977,3073,3156,3216,3234,3441,3254,3060,3082,3054,3016,2980,2926,2902,2898,2954,3034,3128,3194,3260,3292,3338,3810 };
    u16 gbgain[13*17]={2708,2594,2654,2610,2554,2502,2448,2424,2426,2466,2520,2594,2678,2748,2828,2796,3186,2681,2626,2618,2571,2498,2436,2377,2350,2347,2380,2442,2533,2617,2705,2761,2817,2951,2734,2658,2601,2536,2459,2400,2326,2294,2284,2331,2397,2492,2584,2684,2769,2821,2963,2762,2658,2590,2518,2436,2360,2282,2220,2198,2272,2356,2472,2564,2666,2768,2848,2980,2787,2672,2586,2504,2423,2335,2227,2139,2107,2187,2311,2449,2556,2670,2776,2857,3013,2813,2696,2601,2523,2427,2331,2197,2093,2059,2143,2279,2456,2563,2687,2793,2891,3045,2844,2726,2644,2550,2446,2350,2210,2100,2076,2156,2306,2478,2600,2728,2830,2942,3098,2907,2789,2703,2616,2507,2417,2296,2203,2193,2275,2391,2558,2665,2788,2906,3005,3150,2969,2857,2785,2693,2598,2514,2412,2350,2348,2417,2517,2645,2755,2887,2983,3067,3223,3036,2934,2872,2796,2698,2622,2536,2504,2502,2578,2642,2758,2856,2978,3072,3148,3282,3063,2971,2939,2887,2805,2733,2663,2619,2635,2693,2758,2871,2971,3078,3155,3198,3340,3081,3025,2988,2961,2915,2845,2784,2755,2758,2822,2877,2988,3081,3171,3219,3235,3437,3276,3062,3078,3060,3018,2992,2926,2916,2920,2968,3056,3130,3202,3278,3290,3332,3826 };
    u16 bgain[13*17]={2760,2630,2702,2642,2588,2518,2466,2414,2442,2476,2522,2608,2696,2784,2876,2852,3314,2732,2645,2639,2591,2511,2459,2387,2351,2344,2384,2449,2533,2645,2730,2789,2851,3005,2797,2664,2622,2553,2468,2411,2334,2295,2279,2328,2393,2505,2600,2709,2801,2873,3027,2814,2666,2614,2530,2450,2366,2288,2222,2196,2262,2350,2464,2582,2706,2778,2870,3048,2834,2673,2604,2518,2431,2340,2229,2143,2106,2174,2308,2451,2564,2684,2792,2879,3059,2858,2698,2620,2527,2421,2331,2199,2103,2061,2138,2271,2451,2564,2705,2813,2915,3081,2878,2732,2640,2562,2448,2346,2212,2106,2082,2148,2290,2478,2602,2732,2850,2976,3126,2939,2794,2717,2621,2505,2414,2295,2210,2189,2268,2379,2545,2671,2806,2923,3026,3213,3009,2858,2801,2697,2607,2511,2404,2351,2335,2412,2514,2644,2765,2895,3004,3099,3277,3060,2946,2888,2804,2702,2628,2538,2496,2494,2564,2648,2766,2868,3000,3096,3188,3354,3119,2987,2965,2899,2815,2739,2657,2627,2625,2683,2767,2884,2999,3099,3183,3259,3396,3135,3042,3023,2992,2926,2857,2791,2753,2756,2818,2891,3009,3104,3202,3265,3296,3528,3338,3090,3138,3096,3058,3020,2932,2936,2926,2984,3080,3158,3258,3322,3348,3392,3908 };
    for(int i=0;i<(13*17);i++){       /////初始化比较麻烦
        isp->lsc_cfg_reg->rGain[i] = rgain[i];
        isp->lsc_cfg_reg->GrGain[i]= grgain[i];
        isp->lsc_cfg_reg->GbGain[i]= gbgain[i];
        isp->lsc_cfg_reg->bGain[i] = bgain[i];
    }
    ////NR_RAW reg
    isp->nr_raw_cfg_reg->h=100;
    ////AWB reg
    isp->awb_cfg_reg->r_gain=4096;
    isp->awb_cfg_reg->g_gain=4096;
    isp->awb_cfg_reg->b_gain=4096;
    ////WBC reg
    isp->wbc_cfg_reg->m_nR=4096;
    isp->wbc_cfg_reg->m_nGr=4096;
    isp->wbc_cfg_reg->m_nGb=4096;
    isp->wbc_cfg_reg->m_nB=4096;
    ////Demosaic reg

    ////CCM reg
    isp->ccm_cfg_reg->ccm_coeff[0]=1024;
    isp->ccm_cfg_reg->ccm_coeff[1]=0;
    isp->ccm_cfg_reg->ccm_coeff[2]=0;
    isp->ccm_cfg_reg->ccm_coeff[3]=0;
    isp->ccm_cfg_reg->ccm_coeff[4]=1024;
    isp->ccm_cfg_reg->ccm_coeff[5]=0;
    isp->ccm_cfg_reg->ccm_coeff[6]=0;
    isp->ccm_cfg_reg->ccm_coeff[7]=0;
    isp->ccm_cfg_reg->ccm_coeff[8]=1024;

    isp->ccm_cfg_reg->offset_out[0]=0;
    isp->ccm_cfg_reg->offset_out[0]=0;
    isp->ccm_cfg_reg->offset_out[0]=0;
    ////EE reg
    isp->sharpen_cfg_reg->sharpen_param=1;
    ////Gamma
    isp->gamma_cfg_reg->gamma=1.3;
    ////CSC
    isp->csc_cfg_reg->coeff[0]=67;
    isp->csc_cfg_reg->coeff[1]=174;
    isp->csc_cfg_reg->coeff[2]=15;
    isp->csc_cfg_reg->coeff[3]=-36;
    isp->csc_cfg_reg->coeff[4]=-92;
    isp->csc_cfg_reg->coeff[5]=128;
    isp->csc_cfg_reg->coeff[6]=128;
    isp->csc_cfg_reg->coeff[7]=-118;
    isp->csc_cfg_reg->coeff[8]=-10;
    isp->csc_cfg_reg->offset_in[0]=0;
    isp->csc_cfg_reg->offset_in[1]=0;
    isp->csc_cfg_reg->offset_in[2]=0;
    isp->csc_cfg_reg->offset_out[0]=0;
    isp->csc_cfg_reg->offset_out[1]=512;
    isp->csc_cfg_reg->offset_out[2]=512;
    ////NR_YUV reg
    isp->nryuv_cfg_reg->y_sigma2=3600;
    isp->nryuv_cfg_reg->uv_sigma2=6400;
    isp->nryuv_cfg_reg->y_filt=6;
    isp->nryuv_cfg_reg->uv_filt=6;
    ////YFC reg
}


////////////初始化用户输入参数：图像尺寸、位宽、初始化内存空间在判断用户输入尺寸和读入文件大小一致后
void Tunning_Tab::init_raw_image()
{
    isp->isp_cfg_reg->input_width=ui->edit_imwidth->text().toInt();     //////width
    isp->isp_image->input_width=ui->edit_imwidth->text().toInt();
    isp->isp_cfg_reg->input_height=ui->edit_imheight->text().toInt();   /////height
    isp->isp_image->input_height=ui->edit_imheight->text().toInt();
    isp->isp_cfg_reg->sensor_bits=ui->spin_sensorbits->value();         /////sensor_bits
    isp->isp_cfg_reg->bayer_pattern=ui->combx_impattern->currentIndex();/////Bayer Pattern
    isp->isp_image->pic_size=isp->isp_image->input_width * isp->isp_image->input_height;  ///raw image size
}


///////////////可视化RAW图像
void Tunning_Tab::raw2clor(u16 *BAYER_DAT)
{
    /////将raw 数据导入到raw_color_image中并根据Bayer格式填充其他两个通道为0
    int offset=isp->isp_cfg_reg->sensor_bits-8;
    int pattern=isp->isp_cfg_reg->bayer_pattern;
    for(int row=0;row<isp->isp_image->input_height;row++){
        for(int col=0;col<isp->isp_image->input_width;col++){
            u8 pixel_channel = static_cast<u8>((((row & 1) << 1) + (col & 1)) ^ pattern);
            int pixel_value=get_pixel_value(isp->isp_cfg_reg,BAYER_DAT,row,col);
            pixel_value= pixel_value>>offset;       /////将数据移位到8bit范围以便显示RGB888
            pixel_value=qBound(0,  static_cast<int>(pixel_value), 255);  ////钳位到[0.255]

            if (pixel_channel == 0){    ///r
               isp->raw_clor_image->setPixel(col,row,qRgb(pixel_value,0,0));///r
            }
            else if (pixel_channel == 1){ ///gr
               isp->raw_clor_image->setPixel(col,row,qRgb(0,pixel_value,0));//gr
            }
            else if (pixel_channel == 2){  ///gb
              isp->raw_clor_image->setPixel(col,row,qRgb(0,pixel_value,0));//gb
            }
            else{                    ///b
              isp->raw_clor_image->setPixel(col,row,qRgb(0,0,pixel_value));///b
            }
        }
    }
}

//////////rgb Domain image view////////////////
void Tunning_Tab::rgb2view(u16 *RGB_DATA[])
{
    int offset=isp->isp_cfg_reg->sensor_bits-8;
    for(int row=0;row<isp->isp_cfg_reg->input_height;row++){
        for(int col=0;col<isp->isp_cfg_reg->input_width;col++){
            int pixel_pos=row*isp->isp_cfg_reg->input_width+col;////像素位置
            int pixel_R=RGB_DATA[RGB_R][pixel_pos]>>offset;
            int pixel_G=RGB_DATA[RGB_G][pixel_pos]>>offset;
            int pixel_B=RGB_DATA[RGB_B][pixel_pos]>>offset;
            u8 set_R,set_G,set_B;
            set_R=static_cast<u8>(qBound(0, pixel_R, 255));     /////钳位
            set_G=static_cast<u8>(qBound(0, pixel_G, 255));
            set_B=static_cast<u8>(qBound(0, pixel_B, 255));
            isp->rgb_color_image->setPixel(col,row,qRgb(set_R,set_G,set_B));  ////导入用于显示的值
        }
    }
}


///////yuv Domain image view/////////////
void Tunning_Tab::yuv2view(u16 *YUV_DATA[])
{
    ////int offset=isp->isp_cfg_reg->sensor_bits-8;
    int offset=2;    //////YUV输入是10bit的数据
    for(int row=0;row<isp->isp_cfg_reg->input_height;row++){
        for(int col=0;col<isp->isp_cfg_reg->input_width;col++){
            int pixel_pos=row*isp->isp_cfg_reg->input_width+col;////像素位置
            int Y=YUV_DATA[YUV_Y][pixel_pos]>>offset;
            int U=YUV_DATA[YUV_U][pixel_pos]>>offset;
            int V=YUV_DATA[YUV_V][pixel_pos]>>offset;
            int temp_R,temp_G,temp_B;
            temp_R=static_cast<int>(Y + 1.4746 *(V-128));              //////BT2020转换公式
            temp_G=static_cast<int>(Y - 0.1645 * (U - 128) - 0.5713 * (V - 128));
            temp_B=static_cast<int>(Y + 1.8814 * (U - 128));
            temp_R = qBound(0, temp_R, 255);
            temp_G = qBound(0, temp_G, 255);
            temp_B = qBound(0, temp_B, 255);
            u8 pixel_R= static_cast<u8>(temp_R);      //////将YUV格式的数据转换为RGB进行显示
            u8 pixel_G= static_cast<u8>(temp_G);
            u8 pixel_B= static_cast<u8>(temp_B);
            isp->rgb_color_image->setPixel(col,row,qRgb(pixel_R,pixel_G,pixel_B));  ////导入用于显示的值
        }
    }

}


/////////////Open Raw image/////////////////////////
void Tunning_Tab::on_btn_raw_open_clicked()
{
    if(isp){           ////在同一tab重复open时判断是否已经创建isp，如果已经创建则删除重新创建和配置，只有第一次open不执行
        //delete isp;     ///////future:后期或者只删除那部分会被重新申请的内存空间
        //isp=new ISP_Pipeline();
        isp->clear_data();
        init_modules_reg();
        pipeline_check_reset();    ///重新打开后重置复选框

    }

    init_raw_image();      ////初始化raw图像长宽、尺寸、后面判断读入文件大小和输入尺寸一致后再申请内存空间
//    qDebug()<<"inputwidth"<< isp->isp_cfg_reg->input_width;
//    qDebug()<<"inputheight"<< isp->isp_cfg_reg->input_height;
//    qDebug()<<"sensorbits"<< isp->isp_cfg_reg->sensor_bits;
//    qDebug()<<"pattern"<< isp->isp_cfg_reg->bayer_pattern;

    /////open file dialog to get image path
    QString file_name = QFileDialog::getOpenFileName(this, "Chose RAW Image", "", "Images(*.raw *.cfa)");
    if (file_name.isEmpty()){ //如果一个文件都没选
        QMessageBox::warning(nullptr, "Warning", "You don't select any file!");
        //qDebug("empty file!!");
        return;
    }

    /////将QString转换为字符数组指针类型
    QByteArray byteArray = file_name.toUtf8();  ////结尾有/0
//    for (int i = 0; i < sizeof(isp->isp_cfg_reg->input_image_file); i++) {
//        isp->isp_cfg_reg->input_image_file[i] = 0;
//    }
    strncpy(isp->isp_cfg_reg->input_image_file,byteArray.constData(),FILE_NAME_MAX);  ///image name
    ///////将raw数据导入到指针指向的地址
    int isOk=load_raw_image(isp->isp_cfg_reg,isp->isp_image);
    if(isOk!=0){    ////load失败则返回，不进行接下来的操作
        return;
    }
    ui->image_preview_box->setTitle("raw image preview: "+file_name);   ///显示文件路径
    ////创建用于显示的图像，需要将BAYER数据的另外两个通道补0;
    isp->raw_clor_image=new QImage(isp->isp_image->input_width, isp->isp_image->input_height, QImage::Format_RGB888);
    isp->rgb_color_image=new QImage(isp->isp_image->input_width, isp->isp_image->input_height, QImage::Format_RGB888);

    if ((*isp->raw_clor_image).isNull()) {
        QMessageBox::critical(nullptr, "Error", "Failed to create image!");
        //qDebug() << "Failed to create image!";
        return;
    }
    //qDebug("creat QImage Successfully!!!");

    raw2clor(isp->isp_image->BAYER_DAT);           ////将填充raw另外两个通道为0，用于彩色显示
    view->SetImage(*isp->raw_clor_image);          ////将Qimage对象传入view
    QGridLayout *layout=ui->preview_layout;        //////获取image_preview_box下的layout
    layout->addWidget(view);                       ////在layout添加view
}


void Tunning_Tab::on_btn_update_clicked()
{
    if(!isp->isp_image->BAYER_DAT){
        return;
    }
    pipeline_check_reset();
    isp->isp_cfg_reg->sensor_bits=ui->spin_sensorbits->value();
    isp->isp_cfg_reg->bayer_pattern=ui->combx_impattern->currentIndex();     ////0:rggb 1:grbg 2:gbrg 3:bggr
    raw2clor(isp->isp_image->BAYER_DAT);////将填充raw另外两个通道为0，用于彩色显示
    view->SetImage(*isp->raw_clor_image); ////更新视图中的图片
}


////识别双击的listWidgetItem并绑定相应的处理函数来弹出相应的对话框
void Tunning_Tab::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString item_text = item->text();
    //qDebug()<<"Item="<<item_text;
    if(item_text=="DPC"){
        DPC_Double_Clicked();
    }else if(item_text=="BLC"){
        BLC_Double_Clicked();
    }
    else if(item_text=="DGain"){
        DGain_Double_Clicked();
    }
    else if(item_text=="LSC"){
        LSC_Double_Clicked();
    }
    else if(item_text=="NR RAW"){
        NR_RAW_Double_Clicked();
    }
    else if(item_text=="AWB"){
        AWB_Double_Clicked(ui->combx_AWB);  ////通过判断当下AWB选择的方法
    }
    else if(item_text=="Demosaic"){

    }
    else if(item_text=="CCM"){
        CCM_Double_Clicked();
    }
    else if(item_text=="EE"){
        EE_Double_Clicked();
    }
    else if(item_text=="TM"){

    }
    else if(item_text=="Gamma"){
        Gamma_Double_Clicked();
    }
    else if(item_text=="CSC"){
        CSC_Double_Clicked();
    }
    else if(item_text=="NR YUV"){
        NRYUV_Double_Clicked();
    }
    else if(item_text=="YFC"){

    }
}

/////DGain Dialog
void Tunning_Tab::DGain_Double_Clicked()
{
     Dgain_Dialog *dgain_dlg=new  Dgain_Dialog(this);//////打开DGain参数配置窗口
     ////回显上次设置的值
     dgain_dlg->set_Gain(isp->dgain_cfg_reg->Gain_R,isp->dgain_cfg_reg->Gain_Gr,isp->dgain_cfg_reg->Gain_Gb, isp->dgain_cfg_reg->Gain_B);
     int ret =dgain_dlg->exec();  /////非模态弹出，背景不可以选择
     if (ret==QDialog::Accepted){ /////通过对话框选择确认获取Dgain模块的参数值
         isp->dgain_cfg_reg->Gain_R=dgain_dlg->get_Gain_R();
         isp->dgain_cfg_reg->Gain_Gr=dgain_dlg->get_Gain_Gr();
         isp->dgain_cfg_reg->Gain_Gb=dgain_dlg->get_Gain_Gb();
         isp->dgain_cfg_reg->Gain_B=dgain_dlg->get_Gain_B();
     }
     //qDebug()<<"Gain_R="<<isp->dgain_cfg_reg->Gain_R;
     delete  dgain_dlg;
}

//////NR_RAW  Dialog
void Tunning_Tab::NR_RAW_Double_Clicked()
{
    if(ui->combx_NR_RAW->currentIndex()==0){     /////根据选择的算法弹出对应的参数配置弹窗
        nlm_nr_dialog *nr_nlm_dlg=new nlm_nr_dialog(this);
        nr_nlm_dlg->set_nlm_h(isp->nr_raw_cfg_reg->h);
        int ret=nr_nlm_dlg->exec();
        if(ret==QDialog::Accepted){
            isp->nr_raw_cfg_reg->h=nr_nlm_dlg->get_nlm_h();
        }
        delete nr_nlm_dlg;   ///结束后需要释放内存
    }else if(ui->combx_NR_RAW->currentIndex()==1){
       // qDebug()<<"chose biliteral algorithm!!";
    }
}

//////AWB Manual Dialog
void Tunning_Tab::AWB_Double_Clicked(QComboBox *AWB_Method)
{
    if(AWB_Method->currentText()=="Manual"){    ////只有选择了Manual 模式双击AWB才会出现弹窗
        awb_manual_dialog *awb_dlg=new awb_manual_dialog(this);
        awb_dlg->set_RGain(static_cast<int>(isp->wbc_cfg_reg->m_nR));     /////回显
        awb_dlg->set_GrGain(static_cast<int>(isp->wbc_cfg_reg->m_nGr));
        awb_dlg->set_GbGain(static_cast<int>(isp->wbc_cfg_reg->m_nGb));
        awb_dlg->set_BGain(static_cast<int>(isp->wbc_cfg_reg->m_nB));
        int ret=awb_dlg->exec();
        if(ret==QDialog::Accepted){
            isp->wbc_cfg_reg->m_nR=static_cast<u16>(awb_dlg->get_RGain());
            isp->wbc_cfg_reg->m_nGr=static_cast<u16>(awb_dlg->get_GrGain());
            isp->wbc_cfg_reg->m_nGb=static_cast<u16>(awb_dlg->get_GbGain());
            isp->wbc_cfg_reg->m_nB=static_cast<u16>(awb_dlg->get_BGain());
        }
        delete awb_dlg;
    }
}


//////CCM Dialog
void Tunning_Tab::CCM_Double_Clicked()
{
    ccm_dialog *ccm_dlg=new ccm_dialog(this);
    ccm_dlg->set_ccm_coeff(isp->ccm_cfg_reg->ccm_coeff);    /////回显当前设置的ccm寄存器值
    ccm_dlg->set_offset_out(isp->ccm_cfg_reg->offset_out);
    int ret=ccm_dlg->exec();   ////执行会话窗
    if(ret==QDialog::Accepted){  ///点击OK后获取ui值
        int *coeff=ccm_dlg->get_ccm_coeff();
        int *offset=ccm_dlg->get_offset_out();
        for(int i=0;i<9;i++){
            isp->ccm_cfg_reg->ccm_coeff[i]=coeff[i];
        }
        for(int i=0;i<3;i++){
            isp->ccm_cfg_reg->offset_out[i]=offset[i];
        }
        delete [] coeff;    ///使用完后清理
        delete [] offset;
    }
    delete ccm_dlg;      ///清理
}

//////CSC Dialog
void Tunning_Tab::CSC_Double_Clicked()
{
    csc_dialog *csc_dlg=new csc_dialog(this);
    csc_dlg->set_offsetin(isp->csc_cfg_reg->offset_in);    ////回显
    csc_dlg->set_csccoeff(isp->csc_cfg_reg->coeff);
    csc_dlg->set_offsetout(isp->csc_cfg_reg->offset_out);
    int ret=csc_dlg->exec();
    if(ret==QDialog::Accepted){
        int *offsetin=csc_dlg->get_offsetin();
        int *csccoeff=csc_dlg->get_csccoeff();
        int *offsetout=csc_dlg->get_offsetout();
        for(int i=0;i<3;i++){
            isp->csc_cfg_reg->offset_in[i]=offsetin[i];
            isp->csc_cfg_reg->offset_out[i]=offsetout[i];
        }
        for(int i=0;i<9;i++){
            isp->csc_cfg_reg->coeff[i]=csccoeff[i];
        }
        delete [] offsetin;      ///回收
        delete [] offsetout;
        delete [] csccoeff;
    }
    delete csc_dlg;
}

///////EE Dialog
void Tunning_Tab::EE_Double_Clicked()
{
    sharpen_dialog *sharpen_dlg=new sharpen_dialog(this);
    sharpen_dlg->set_sharpen_param(isp->sharpen_cfg_reg->sharpen_param); ///回显上次值
    int ret=sharpen_dlg->exec();
    if(ret==QDialog::Accepted){
        isp->sharpen_cfg_reg->sharpen_param=sharpen_dlg->get_sharpen_param();  ///通过UI设置值
    }
    delete sharpen_dlg;
}

//////DPC Dialog
void Tunning_Tab::DPC_Double_Clicked()
{
    dpc_dialog *dpc_dlg=new dpc_dialog(this);
    dpc_dlg->set_dpc_mode(isp->dpc_cfg_reg->mode); ///回显
    int ret=dpc_dlg->exec();
    if(ret==QDialog::Accepted){
        isp->dpc_cfg_reg->mode=dpc_dlg->get_dpc_mode();
    }
    delete dpc_dlg;
}

//////BLC Dialog
void Tunning_Tab::BLC_Double_Clicked()
{
    blc_dialog *blc_dlg=new blc_dialog(this);
    blc_dlg->set_blc_paramter(isp->blc_cfg_reg->parameter);
    int ret=blc_dlg->exec();
    if(ret==QDialog::Accepted){
        int *blc_param=blc_dlg->get_blc_paramter();
        for(int i=0;i<4;i++){
            isp->blc_cfg_reg->parameter[i]=blc_param[i];
        }
        delete [] blc_param;
    }
    delete blc_dlg;
}

//////LSC Dialog
void Tunning_Tab::LSC_Double_Clicked()
{
    lsc_dialog *lsc_dlg=new lsc_dialog(this);
    lsc_dlg->set_R_Gain_Mat(isp->lsc_cfg_reg->rGain);
    lsc_dlg->set_Gr_Gain_Mat(isp->lsc_cfg_reg->GrGain);
    lsc_dlg->set_Gb_Gain_Mat(isp->lsc_cfg_reg->GbGain);
    lsc_dlg->set_B_Gain_Mat(isp->lsc_cfg_reg->bGain);
    int ret=lsc_dlg->exec();
    if(ret==QDialog::Accepted){
        int *rgain=lsc_dlg->get_R_Gain_Mat();
        int *grgain=lsc_dlg->get_Gr_Gain_Mat();
        int *gbgain=lsc_dlg->get_Gb_Gain_Mat();
        int *bgain=lsc_dlg->get_B_Gain_Mat();
        for(int i=0;i<13*17;i++){
            isp->lsc_cfg_reg->rGain[i]=rgain[i];
            isp->lsc_cfg_reg->GrGain[i]=grgain[i];
            isp->lsc_cfg_reg->GbGain[i]=gbgain[i];
            isp->lsc_cfg_reg->bGain[i]=bgain[i];
        }
        delete [] rgain;delete [] grgain;delete [] gbgain;delete [] bgain;
    }
    delete lsc_dlg;
}

///////NR_YUV  Dialog
void Tunning_Tab::NRYUV_Double_Clicked()
{
    nryuv_dialog *nryuv_dlg=new nryuv_dialog(this);
    nryuv_dlg->set_ysigma2(isp->nryuv_cfg_reg->y_sigma2);  ////回显
    nryuv_dlg->set_uvsigma2(isp->nryuv_cfg_reg->uv_sigma2);
    nryuv_dlg->set_yfilt(isp->nryuv_cfg_reg->y_filt);
    nryuv_dlg->set_uvfilt(isp->nryuv_cfg_reg->uv_filt);
    int ret=nryuv_dlg->exec();
    if(ret==QDialog::Accepted){
        isp->nryuv_cfg_reg->y_sigma2=nryuv_dlg->get_ysigma2();
        isp->nryuv_cfg_reg->uv_sigma2=nryuv_dlg->get_uvsigma2();
        isp->nryuv_cfg_reg->y_filt=nryuv_dlg->get_yfilt();
        isp->nryuv_cfg_reg->uv_filt=nryuv_dlg->get_uvfilt();
    }
    delete nryuv_dlg;
}

////Gamma Dialog
void Tunning_Tab::Gamma_Double_Clicked()
{
    gamma_dialog *gamma_dlg=new gamma_dialog(this);
    gamma_dlg->set_gamma(isp->gamma_cfg_reg->gamma);
    int ret=gamma_dlg->exec();
    if(ret==QDialog::Accepted){
        isp->gamma_cfg_reg->gamma=gamma_dlg->get_gamma();
    }
    delete gamma_dlg;
}


//////将用户的选择导入寄存器使能中
void Tunning_Tab::Item_check2reg()
{
    int itemCount = ui->listWidget->count(); // 获取项目数量
    for (int i = 0; i < itemCount; i++) {      ////////////////更新模块的选中状态
        QListWidgetItem* item = ui->listWidget->item(i); // 获取索引为 i 的项目
        QString text = item->text(); // 获取项目的文本
        if(item->text()=="DGain"){
            if(item->checkState()==Qt::Checked){
                isp->dgain_cfg_reg->enable=1;     ///设置enable
                isp->current_pattern=BAYER;    /////指示当前执行后得到的图片为BAYER格式
            }else{
                isp->dgain_cfg_reg->enable=0;
            }
        }else if(item->text()=="DPC"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=BAYER;
                isp->dpc_cfg_reg->enable=1;
            }else{
                isp->dpc_cfg_reg->enable=0;
            }
        }else if(item->text()=="BLC"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=BAYER;
                isp->blc_cfg_reg->enable=1;
            }else{
                isp->blc_cfg_reg->enable=0;
            }
        }else if(item->text()=="LSC"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=BAYER;
                isp->lsc_cfg_reg->enable=1;
            }else{
                isp->lsc_cfg_reg->enable=0;
            }
        }else if(item->text()=="NR RAW"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=BAYER;
                isp->nr_raw_cfg_reg->enable=1;
            }else{
                isp->nr_raw_cfg_reg->enable=0;
            }
        }else if(item->text()=="AWB"){
            if(item->checkState()==Qt::Checked){   /////WBC与AWB相辅相成
                isp->current_pattern=BAYER;
                isp->awb_cfg_reg->enable=1;
                isp->wbc_cfg_reg->enable=1;
            }else{
                isp->awb_cfg_reg->enable=0;
                isp->wbc_cfg_reg->enable=0;
            }

        }else if(item->text()=="Demosaic"){    //////DMS
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=RGB;        /////指示当前执行后得到RGB图片
                isp->dms_cfg_reg->enable=1;
            }else{
                isp->dms_cfg_reg->enable=0;
            }
        }else if(item->text()=="CCM"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=RGB;        /////指示当前执行后得到RGB图片
                isp->ccm_cfg_reg->enable=1;
            }else{
                isp->ccm_cfg_reg->enable=0;
            }
        }else if(item->text()=="EE"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=RGB;        /////指示当前执行后得到RGB图片
                isp->sharpen_cfg_reg->enable=1;
            }else{
                isp->sharpen_cfg_reg->enable=0;
            }
        }else if(item->text()=="TM"){

        }else if(item->text()=="Gamma"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=RGB;        /////指示当前执行后得到RGB图片
                isp->gamma_cfg_reg->enable=1;
            }else{
                isp->gamma_cfg_reg->enable=0;
            }
        }else if(item->text()=="CSC"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=YUV444;        /////指示当前执行后得到YUV图片
                isp->csc_cfg_reg->enable=1;
            }else{
                isp->csc_cfg_reg->enable=0;
            }
        }else if(item->text()=="NR YUV"){
            if(item->checkState()==Qt::Checked){
                isp->current_pattern=YUV444;        /////指示当前执行后得到YUV图片
                isp->nryuv_cfg_reg->enable=1;
            }else{
                isp->nryuv_cfg_reg->enable=0;
            }
        }else if(item->text()=="YFC"){     ////Not used

        }
    }

}

/////重置所有算法复选框为unchecked
void Tunning_Tab::pipeline_check_reset()
{
     int itemCount = ui->listWidget->count();
     for(int i=0;i<itemCount;i++){
         QListWidgetItem* item = ui->listWidget->item(i);
         item->setCheckState(Qt::Unchecked);
     }
}


////////Run Pipeline可能导致阻塞，需要使用多线程
void Tunning_Tab::Run_Pipeline()
{
    ////判断是否有图片
    if(!isp->isp_image->BAYER_DAT){
        QMessageBox::critical(nullptr, "Error", "You just not open any file, and last open is cleaned, Please reselect raw file!");
        //qDebug()<<"no file";
        return;
    }

    Item_check2reg();////每次运行前更新模块使能选项

    /////判断RGB域的算法必须先经过dms
    if(!isp->dms_cfg_reg->enable && (isp->ccm_cfg_reg->enable ||
                                     isp->ccm_cfg_reg->enable ||
                                     isp->sharpen_cfg_reg->enable ||
                                     isp->gamma_cfg_reg->enable||
                                     isp->csc_cfg_reg->enable)){
        QMessageBox::critical(nullptr, "Error", "Algorithm in RGB domain must Demosaic first, Please recheck the pipline!");
        return;
    }
    //////判断YUV域的算法是否先经过了csc
    if(!isp->csc_cfg_reg->enable && isp->nryuv_cfg_reg->enable){
        QMessageBox::critical(nullptr, "Error", "Algorithm in YUV domain must CSC first, Please recheck the pipline!");
        return;
    }

   ////先设置current_BAYER_DATA为原样
   copy_rawdata(isp->isp_cfg_reg,isp->isp_image);////初始化为原样,用户可以多次修改算法后运行
   ///////////////////////Bayer Domain//////////////////////////////////////
   ////DPC
   dpc(isp->isp_image,isp->dpc_cfg_reg,isp->isp_cfg_reg);
   ////BLC
   blc(isp->isp_image,isp->blc_cfg_reg,isp->isp_cfg_reg);
   ////LSC
   lsc(isp->isp_image,isp->lsc_cfg_reg,isp->isp_cfg_reg);
   ////NR_RAW
   nr_raw_nlm(isp->isp_image,isp->nr_raw_cfg_reg,isp->isp_cfg_reg);
   ///DGain
   dgian(isp->isp_image,isp->dgain_cfg_reg,isp->isp_cfg_reg);
   ////AWB
   switch(ui->combx_AWB->currentIndex()){          ////////AWB
   case 0:
       awb_GW(isp->isp_image,isp->awb_cfg_reg,isp->isp_cfg_reg);
       break;
   case 1:
       awb_maxRGB(isp->isp_image,isp->awb_cfg_reg,isp->isp_cfg_reg);
       break;
   case 2:
       awb_LWP(isp->isp_image,isp->awb_cfg_reg,isp->isp_cfg_reg);
       break;
   case 3:
       isp->awb_cfg_reg->r_gain = isp->wbc_cfg_reg->m_nR ;    //////Manual模式则awb增益为wbc的增益
       isp->awb_cfg_reg->g_gain = isp->wbc_cfg_reg->m_nGr;
       isp->awb_cfg_reg->g_gain = isp->wbc_cfg_reg->m_nGb;
       isp->awb_cfg_reg->b_gain = isp->wbc_cfg_reg->m_nB ;
       break;
   default:
        awb_GW(isp->isp_image,isp->awb_cfg_reg,isp->isp_cfg_reg);
       break;
   }
   ///////////这里添加一个手动设置白平衡增益的分支
   isp->wbc_cfg_reg->m_nR  = isp->awb_cfg_reg->r_gain;
   isp->wbc_cfg_reg->m_nGr = isp->awb_cfg_reg->g_gain;
   isp->wbc_cfg_reg->m_nGb = isp->awb_cfg_reg->g_gain;
   isp->wbc_cfg_reg->m_nB  = isp->awb_cfg_reg->b_gain;
   ////WBC
   wbc(isp->isp_image,isp->wbc_cfg_reg,isp->isp_cfg_reg);  /////WBC
   ////Demosaic
   demosaic_malvar(isp->isp_image,isp->dms_cfg_reg,isp->isp_cfg_reg);
   ///////////////////////////RGB Domain/////////////////////////////////////
   ////CCM
   ccm(isp->isp_image,isp->ccm_cfg_reg,isp->isp_cfg_reg);
   ////EE
   sharpen(isp->isp_image,isp->sharpen_cfg_reg,isp->isp_cfg_reg);
   ////TM

   ////Gamma
   gamma(isp->isp_image, isp->gamma_cfg_reg, isp->isp_cfg_reg);  ////GAMMA
   ////CSC
   csc(isp->isp_image, isp->csc_cfg_reg, isp->isp_cfg_reg);
   /////////////////////////////YUV Domain//////////////////////////////////
   ////NR_YUV
   nr_yuv444(isp->isp_image, isp->nryuv_cfg_reg, isp->isp_cfg_reg);
   ////YFC
   ////////////////Display Imgae from BAYER/RGB/YUV depends on user chose//////
   if(isp->current_pattern==BAYER){
       raw2clor(isp->isp_image->current_BAYER_DAT);   ////可视化RAW数据
       view->SetImage(*isp->raw_clor_image);
   }else if(isp->current_pattern==RGB){
       rgb2view(isp->isp_image->RGB_DAT);             ///可视化RGB数据
       view->SetImage(*isp->rgb_color_image);
   }else if(isp->current_pattern==YUV444){
       yuv2view(isp->isp_image->YUV_DAT);            ///可视化YUV数据
       view->SetImage(*isp->rgb_color_image);
   }

   emit finished();    ////运行完成信号
}


////////////////ISP Pipeline Run/////////////////
void Tunning_Tab::on_btn_isp_run_clicked()
{
//    QThread workerThread;
//    //this->moveToThread(&workerThread);   ///将当前对象移至多线程
//    ui->btn_isp_run->setEnabled(false);
//    QObject::connect(&workerThread, SIGNAL(started()), this, SLOT(Run_Pipeline())); ////绑定多线程与Run_Pipeline
//    QObject::connect(this, &Tunning_Tab::finished, [&]() {  ////绑定运行完成信号与操作函数
//        ui->btn_isp_run->setEnabled(true);
//        qDebug()<<"Finish Pipeline";
//     });
//    workerThread.start(); // 启动新线程


    QThread *workerThread = new QThread;    ////始终无法实现无阻塞
    //this->moveToThread(workerThread);
     //连接新线程的started信号到执行复杂算法的槽函数
    connect(workerThread, &QThread::started, this, &Tunning_Tab::Run_Pipeline);
     //连接复杂算法执行完毕后的信号到新线程的退出函数
    connect(this, &Tunning_Tab::finished, workerThread, &QThread::quit);
     //连接新线程的finished信号到新线程的删除函数
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    //运行线程
    workerThread->start();

    //Run_Pipeline();
}

