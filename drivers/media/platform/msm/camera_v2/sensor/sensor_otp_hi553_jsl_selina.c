#define HW_CMR_LOG_TAG "sensor_otp_hi553_jsl_selina"

#include <linux/hw_camera_common.h>
#include "msm_sensor.h"
#include "sensor_otp_common_if.h"

//Golden sensor typical ratio

static int RG_Ratio_Typical = 0x13d;
static int BG_Ratio_Typical = 0x12f;

//#define QTECH_VENDOR_ID 0x06
//#define CHICONY_VENDOR_ID 0x02
//#define HI553_JSL_SELINA_QTECH_MODULE_VCM_OFFSET 150
//#define HI553_JSL_SELINA_CHICONY_MODULE_VCM_OFFSET 150

#define HI553_JSL_SELINA_MODULE_REG_NO   16
#define HI553_JSL_SELINA_AWB_REG_NO      29
//#define HI553_JSL_SELINA_AF_REG_NO       4
//#define HI553_JSL_SELINA_LSC_REG_NO     1105
#define HI553_JSL_SELINA_GROUP_FLAG_NO     1
#define HI553_JSL_SELINA_CHECKSUM_OFFSET      1
#define HI553_JSL_SELINA_I2C_RETRY_TIMES 3

#define loge_if_ret(x) \
{\
	if (x<0) \
	{\
		CMR_LOGE("'%s' failed", #x); \
		return -1; \
	} \
}

//OTP info struct
typedef struct hi553_jsl_selina_otp_info {
	uint8_t  vendor_id;
	uint8_t  af_flag;
	uint8_t  year;
	uint8_t  month;
	uint8_t  day;
	uint8_t  sensor_id;
	uint8_t  lens_id;
	uint8_t  lense_id;	
	uint8_t  vcm_id;	
	uint8_t  driver_ic;
	uint16_t rg_ratio;
	uint16_t bg_ratio;
	uint16_t gb_gr_ratio;
	uint16_t af_macro_dac;
	uint16_t af_infinity_dac;
}st_hi553_jsl_selina_otp_info;

//hi553_jsl_selina has three groups: [1,2,3]
typedef enum hi553_jsl_selina_groups_count{
	GROUP_1 = 0,
	GROUP_2,
	GROUP_3,
	GROUP_MAX
}enum_hi553_jsl_selina_groups;

typedef enum hi553_jsl_selina_groups_type{
	GROUP_TYPE_MODULE = 0,
	GROUP_TYPE_AWB,
//	GROUP_TYPE_AF,
//	GROUP_TYPE_LSC,
	GROUP_TYPE_MAX
}enum_hi553_jsl_selina_type;

static uint16_t hi553_jsl_selina_module_info_otp_read_addr[1][2] = {
/*module_addr awb_addr  af_addr   lsc_addr */
	{0x0501,   0x0535},//group1
//	{0x0513,   0x0554},//group2
//	{0x0524,   0x0572},//group3
};

static struct msm_camera_i2c_reg_array hi553_jsl_selina_otp_init_setting[]=
{

   {0x0a00, 0x0000}, //stream off
   {0x0e00, 0x0102}, //tg_pmem_sckpw/sdly
   {0x0e02, 0x0102}, //tg_dmem_sckpw/sdly
   {0x2000, 0x4031},
   {0x2002, 0x8400},
   {0x2004, 0x430d},
   {0x2006, 0x430c},
   {0x2008, 0x0900},
   {0x200a, 0x7312},
   {0x200c, 0x43d2},
   {0x200e, 0x0f82},
   {0x2010, 0x0cff},
   {0x2012, 0x0cff},
   {0x2014, 0x0cff},
   {0x2016, 0x0cff},
   {0x2018, 0x0cff},
   {0x201a, 0x0cff},
   {0x201c, 0x0c32},
   {0x201e, 0x40f2},
   {0x2020, 0x000e},
   {0x2022, 0x0f90},
   {0x2024, 0x43d2},
   {0x2026, 0x0180},
   {0x2028, 0x4392},
   {0x202a, 0x760e},
   {0x202c, 0x0900},
   {0x202e, 0x760c},
   {0x2030, 0x9382},
   {0x2032, 0x760c},
   {0x2034, 0x2002},
   {0x2036, 0x0c64},
   {0x2038, 0x3ffb},
   {0x203a, 0x421f},
   {0x203c, 0x760a},
   {0x203e, 0x931f},
   {0x2040, 0x2023},
   {0x2042, 0x421d},
   {0x2044, 0x018a},
   {0x2046, 0x4d82},
   {0x2048, 0x7600},
   {0x204a, 0x4382},
   {0x204c, 0x7602},
   {0x204e, 0x0260},
   {0x2050, 0x0000},
   {0x2052, 0x0c64},
   {0x2054, 0x0240},
   {0x2056, 0x0000},
   {0x2058, 0x0260},
   {0x205a, 0x0000},
   {0x205c, 0x0c1e},
   {0x205e, 0x4382},
   {0x2060, 0x7602},
   {0x2062, 0x4d82},
   {0x2064, 0x7600},
   {0x2066, 0x0270},
   {0x2068, 0x0000},
   {0x206a, 0x0c1c},
   {0x206c, 0x0270},
   {0x206e, 0x0001},
   {0x2070, 0x421f},
   {0x2072, 0x7606},
   {0x2074, 0x4fc2},
   {0x2076, 0x0188},
   {0x2078, 0x0260},
   {0x207a, 0x0000},
   {0x207c, 0x421f},
   {0x207e, 0x7606},
   {0x2080, 0x4fc2},
   {0x2082, 0x0188},
   {0x2084, 0x4d0c},
   {0x2086, 0x3fd0},
   {0x2088, 0x903f},
   {0x208a, 0x0201},
   {0x208c, 0x23cd},
   {0x208e, 0x531d},
   {0x2090, 0x4d0e},
   {0x2092, 0x108e},
   {0x2094, 0xf37e},
   {0x2096, 0xc312},
   {0x2098, 0x100e},
   {0x209a, 0x110e},
   {0x209c, 0x110e},
   {0x209e, 0x110e},
   {0x20a0, 0x4c0f},
   {0x20a2, 0x108f},
   {0x20a4, 0xf37f},
   {0x20a6, 0xc312},
   {0x20a8, 0x100f},
   {0x20aa, 0x110f},
   {0x20ac, 0x110f},
   {0x20ae, 0x110f},
   {0x20b0, 0x9f0e},
   {0x20b2, 0x240e},
   {0x20b4, 0x0261},
   {0x20b6, 0x0000},
   {0x20b8, 0x4382},
   {0x20ba, 0x7600},
   {0x20bc, 0x4382},
   {0x20be, 0x7602},
   {0x20c0, 0x0260},
   {0x20c2, 0x0000},
   {0x20c4, 0x0c64},
   {0x20c6, 0x0240},
   {0x20c8, 0x0000},
   {0x20ca, 0x0260},
   {0x20cc, 0x0000},
   {0x20ce, 0x0c1e},
   {0x20d0, 0x4382},
   {0x20d2, 0x7602},
   {0x20d4, 0x4d82},
   {0x20d6, 0x7600},
   {0x20d8, 0x0270},
   {0x20da, 0x0000},
   {0x20dc, 0x0c1c},
   {0x20de, 0x0270},
   {0x20e0, 0x0001},
   {0x20e2, 0x421f},
   {0x20e4, 0x7606},
   {0x20e6, 0x4fc2},
   {0x20e8, 0x0188},
   {0x20ea, 0x0260},
   {0x20ec, 0x0000},
   {0x20ee, 0x3fc6},
   {0x20f0, 0x4030},
   {0x20f2, 0xf0f4},
   {0x20f4, 0xdf02},
   {0x20f6, 0x3ffe},
   {0x20f8, 0x430e},
   {0x20fa, 0x430f},
   {0x20fc, 0x3c08},
   {0x20fe, 0xc312},
   {0x2100, 0x100d},
   {0x2102, 0x100c},
   {0x2104, 0x2802},
   {0x2106, 0x5a0e},
   {0x2108, 0x6b0f},
   {0x210a, 0x5a0a},
   {0x210c, 0x6b0b},
   {0x210e, 0x930c},
   {0x2110, 0x23f6},
   {0x2112, 0x930d},
   {0x2114, 0x23f4},
   {0x2116, 0x4130},
   {0x2118, 0x4030},
   {0x211a, 0xf0f8},
   {0x211c, 0xee0e},
   {0x211e, 0x403b},
   {0x2120, 0x0011},
   {0x2122, 0x3c05},
   {0x2124, 0x100d},
   {0x2126, 0x6e0e},
   {0x2128, 0x9a0e},
   {0x212a, 0x2801},
   {0x212c, 0x8a0e},
   {0x212e, 0x6c0c},
   {0x2130, 0x6d0d},
   {0x2132, 0x831b},
   {0x2134, 0x23f7},
   {0x2136, 0x4130},
   {0x2138, 0xef0f},
   {0x213a, 0xee0e},
   {0x213c, 0x4039},
   {0x213e, 0x0021},
   {0x2140, 0x3c0a},
   {0x2142, 0x1008},
   {0x2144, 0x6e0e},
   {0x2146, 0x6f0f},
   {0x2148, 0x9b0f},
   {0x214a, 0x2805},
   {0x214c, 0x2002},
   {0x214e, 0x9a0e},
   {0x2150, 0x2802},
   {0x2152, 0x8a0e},
   {0x2154, 0x7b0f},
   {0x2156, 0x6c0c},
   {0x2158, 0x6d0d},
   {0x215a, 0x6808},
   {0x215c, 0x8319},
   {0x215e, 0x23f1},
   {0x2160, 0x4130},
   {0x2162, 0x4130},
   {0x2ffe, 0xf000},
 //    EOFIRM
 //--------------------------------------------------------------------
 // end of software code
 //--------------------------------------------------------------------
  
    {0x0b00, 0x0000},
    {0x0b02, 0x1807},
    {0x0b04, 0x3540},
    {0x0b06, 0x3540},
    {0x0b08, 0x0000},
    {0x0b0a, 0x0000},
    {0x0b0c, 0x0000},
    {0x0b0e, 0x8200},
    {0x0b10, 0x0020},
    {0x0b12, 0x0000},
    {0x0b14, 0x001c},
    {0x0b16, 0x6e0b},
    {0x0b18, 0xf20b},
    {0x0b1a, 0x0000},
    {0x0b1c, 0x0000},
    {0x0b1e, 0x0081},
    {0x0b20, 0x0000},
    {0x0b22, 0x4c80},
    {0x0b24, 0x0000},
    {0x0b26, 0x0000},
    {0x0b28, 0x0000},
    {0x0e0a, 0x0001}, //tg_pmem_cen, rom_cen enable
    {0x004a, 0x0100}, //tg enable,hdr off
    {0x0a00, 0x0100}, //stream on

};
/*OTP READ STATUS*/
#define HI553_JSL_SELINA_OTP_MODULE_INFO_READ   (1 << 0)
#define HI553_JSL_SELINA_OTP_AWB_READ           (1 << 1)
//#define HI553_JSL_SELINA_OTP_AF_READ            (1 << 2)
//#define HI553_JSL_SELINA_OTP_LSC_READ           (1 << 3)
#define HI553_JSL_SELINA_OTP_FAIL_FLAG          (1 << 4)

#define HI553_JSL_SELINA_OTP_SUCCESS (HI553_JSL_SELINA_OTP_MODULE_INFO_READ|HI553_JSL_SELINA_OTP_AWB_READ)

#define HI553_JSL_SELINA_MMI_OTP_MODULE_INFO_FLAG  (1 << 0)
#define HI553_JSL_SELINA_MMI_OTP_AWB_FLAG          (1 << 1)
//#define HI553_JSL_SELINA_MMI_OTP_AF_FLAG           (1 << 2)
//#define HI553_JSL_SELINA_MMI_OTP_LSC_FLAG          (1 << 3)
#define HI553_JSL_SELINA_MMI_OTP_FAIL (HI553_JSL_SELINA_MMI_OTP_MODULE_INFO_FLAG|HI553_JSL_SELINA_MMI_OTP_AWB_FLAG)

static st_hi553_jsl_selina_otp_info hi553_jsl_selina_otp_info = {0};
static uint16_t  hi553_jsl_selina_otp_flag   = 0;
static struct msm_camera_i2c_reg_setting st_hi553_jsl_selina_otp_init;
/****************************************************************************
 * FunctionName: hi553_jsl_selina_otp_write_i2c;
 * Description : write otp info via i2c;
 ***************************************************************************/
static int32_t hi553_jsl_selina_otp_write_i2c(struct msm_sensor_ctrl_t *s_ctrl, int32_t addr, uint16_t data)
{
	int32_t rc = 0;
	int32_t i = 0;

	for(i = 0; i < HI553_JSL_SELINA_I2C_RETRY_TIMES; i++){
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
 		s_ctrl->sensor_i2c_client,
 		addr,
 		data,
 		MSM_CAMERA_I2C_BYTE_DATA);

		if (rc < 0) {
			CMR_LOGE("%s, failed i = %d",__func__,i);
			continue;
		}
		break;
	}

	if ( rc < 0 ){
		CMR_LOGE("%s fail, rc = %d! addr = 0x%x, data = 0x%x\n", __func__, rc, addr, data);
#ifdef CONFIG_HUAWEI_DSM
		camera_report_dsm_err_otp(s_ctrl, DSM_CAMERA_OTP_I2C_ERR, addr, OTP_WRITE_I2C_ERR);
#endif
	}

	return rc;
}

/****************************************************************************
 * FunctionName: hi553_jsl_selina_otp_write_i2c_table;
 * Description : write otp info via i2c;
 ***************************************************************************/
static int32_t hi553_jsl_selina_otp_write_i2c_table(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_setting *write_setting)
{
	int32_t rc = 0;
	int32_t i = 0;

	if(NULL == write_setting){
	CMR_LOGE("%s fail,noting to write i2c \n", __func__);
	return -1;
	}
	for(i = 0; i < HI553_JSL_SELINA_I2C_RETRY_TIMES; i++){
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write_table(
 		s_ctrl->sensor_i2c_client,write_setting);

		if (rc < 0){
			CMR_LOGE("%s, failed i = %d",__func__,i);
			continue;
		}
		break;
	}

	if ( rc < 0 ){
	CMR_LOGE("%s fail, rc = %d! \n", __func__, rc);
#ifdef CONFIG_HUAWEI_DSM
		camera_report_dsm_err_otp(s_ctrl, DSM_CAMERA_OTP_I2C_ERR, 0, OTP_WRITE_I2C_ERR);
#endif
	}
	return rc;
}

/****************************************************************************
 * FunctionName: hi553_jsl_selina_otp_read_i2c;
 * Description : read otp info via i2c;
 ***************************************************************************/
static int32_t hi553_jsl_selina_otp_read_i2c(struct msm_sensor_ctrl_t *s_ctrl,uint32_t addr, uint16_t *data)
{
	int32_t rc = 0;

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,
				addr,
				data,
				MSM_CAMERA_I2C_BYTE_DATA);
	if ( rc < 0 ){
		CMR_LOGE("%s fail, rc = %d! addr = 0x%x, data = 0x%x\n", __func__, rc, addr, *data);
#ifdef CONFIG_HUAWEI_DSM
		camera_report_dsm_err_otp(s_ctrl, DSM_CAMERA_OTP_I2C_ERR, addr, OTP_READ_I2C_ERR);
#endif
	}
	return rc;
}

static int32_t hi553_jsl_selina_otp_single_read(struct msm_sensor_ctrl_t *s_ctrl,uint32_t addr, uint16_t *data)
{
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl, 0x010a, (addr>>8)&0xFF));
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl, 0x010b, addr&0xFF));
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl, 0x0102, 0x01));
	loge_if_ret(hi553_jsl_selina_otp_read_i2c(s_ctrl,0x0108,data));
	return 0;
}
static uint16_t hi553_jsl_selina_group_flag = 0xff;
//hi553_jsl_selina_otp_get_group(s_ctrl, GROUP_TYPE_MODULE,  HI553_JSL_SELINA_MODULE_REG_NO, &group);
//hi553_jsl_selina_module_info_otp_read_addr  int32_t addr
static int hi553_jsl_selina_otp_get_group(struct msm_sensor_ctrl_t* s_ctrl, enum_hi553_jsl_selina_type group_type, enum_hi553_jsl_selina_groups *group_sel)
{
	//uint16_t group_flag = 0xff;
	uint32_t i;  
	int32_t rc;
	uint16_t addr;

	for(i=0; i< ARRAY_SIZE(hi553_jsl_selina_module_info_otp_read_addr); i++)
	{
		addr=hi553_jsl_selina_module_info_otp_read_addr[i][group_type];
		if(!addr)
		{
			CMR_LOGE("addr == 0, group is null. continue.\n");
			continue;
		}
		rc = hi553_jsl_selina_otp_single_read(s_ctrl,addr,&hi553_jsl_selina_group_flag);
		CMR_LOGE("%s read group addr=0x%x, group_type=0x%x\n",__func__,addr, group_type);
		if(rc < 0){
			CMR_LOGE("%s read group addr=0x%x\n",__func__,addr);
			return -1;
		}
		if(hi553_jsl_selina_group_flag == 0x01 || hi553_jsl_selina_group_flag == 0x13 ||hi553_jsl_selina_group_flag == 0x37)
		{
			*group_sel = i;
			CMR_LOGE("%s type:0x%x group_sel =0x%x  hi553_jsl_selina_group_flag =0x%x\n",__func__,group_type, *group_sel, hi553_jsl_selina_group_flag);
			return 0;
		}
		CMR_LOGE("%s [group:0x%x type:0x%x hi553_jsl_selina_group_flag:0x%x] continue\n",__func__,i,group_type,hi553_jsl_selina_group_flag);
	}

	CMR_LOGE("%s [group_type:0x%x error\n",__func__,group_type);
	return -1;
}

static int hi553_jsl_selina_otp_continuous_read(struct msm_sensor_ctrl_t* s_ctrl,uint16_t addr,int rd_num,
						uint16_t * rd_buf,int *sum, int *checksum_otp)
{
	int rc = 0;
	int i=0;
	int sum1=0;

	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl, 0x010a, (addr)>>8&0xFF));//continus single read mode
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl, 0x010b, (addr)&0xFF));
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl, 0x0102, 0x01));

	for(i = 0; i < rd_num; i++)
	{
	    rc = hi553_jsl_selina_otp_read_i2c(s_ctrl, 0x0108, &rd_buf[i]);
		if(rc < 0){
			CMR_LOGE("%s ,%d,fail hi553_jsl_selina_otp_read_i2c,i=%d,rc=%d\n", __func__,__LINE__,i,rc);
			return -1;
		}
		sum1 += rd_buf[i];
	}
	hi553_jsl_selina_otp_read_i2c(s_ctrl, 0x0108, checksum_otp);

	if(sum != NULL){
		*sum= sum1;
	}
	return rc;
}
//zhang
static int hi553_jsl_selina_otp_get_module_info(struct msm_sensor_ctrl_t* s_ctrl,uint16_t module_addr)
{
	uint16_t buf[HI553_JSL_SELINA_MODULE_REG_NO] = {0};
	int rc = 0;
	int sum_module=0;
	uint16_t group_checksum_module= 0;
	uint16_t checksum= 0;
	if (hi553_jsl_selina_group_flag == 0x01)
		{
		module_addr = module_addr + 1;
		}
	else if (hi553_jsl_selina_group_flag == 0x13)
		{
		module_addr = module_addr + 18;
		}
	else if (hi553_jsl_selina_group_flag == 0x37)
		{
		module_addr = module_addr + 35;
		}
	else 
		{
		CMR_LOGE("%s ,read hi553_jsl_selina_group_flag fail \n", __func__);
		}
	
	CMR_LOGE("%s   hi553_jsl_selina_group_flag =0x%x  module_addr =0x%x\n",__func__,hi553_jsl_selina_group_flag,module_addr);

	rc = hi553_jsl_selina_otp_continuous_read(s_ctrl,module_addr ,HI553_JSL_SELINA_MODULE_REG_NO,
								buf,&sum_module, &group_checksum_module);
	if(rc < 0){
		CMR_LOGE("%s ,fail read hi553_jsl_selina module info \n", __func__);
		return -1;
	}
	CMR_LOGE("%s vendor id = 0x%x module info: year = 20%02d month = %d day = %d. \n",
			__func__, buf[0], buf[2], buf[3], buf[4]);

	CMR_LOGE("%s module info: Module code= %d Module version = %d software version = %d AWB station number=%d. \n",
			__func__, buf[9], buf[10], buf[11], buf[12]);

	checksum = sum_module%255+1;
	if(checksum != group_checksum_module){
		CMR_LOGE("%s ,failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,module_addr,group_checksum_module);
		return -1;
   	 }
	
	hi553_jsl_selina_otp_info.vendor_id = buf[0];
	s_ctrl->vendor_otp_info.vendor_id = hi553_jsl_selina_otp_info.vendor_id;
   	CMR_LOGE("%s ,module_info success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,module_addr,group_checksum_module);
	
	return rc;
}
static int hi553_jsl_selina_otp_get_awb(struct msm_sensor_ctrl_t* s_ctrl,uint16_t  awb_addr)
{
	uint16_t buf[HI553_JSL_SELINA_AWB_REG_NO] = {0};
	int rc = 0;
    	int sum_awb=0;
	uint16_t group_checksum_awb= 0;
	uint16_t checksum= 0;
	uint16_t  gloden_rg =0 ;
	uint16_t  gloden_bg =0 ;
	uint16_t  gloden_gg =0 ;

	if (hi553_jsl_selina_group_flag == 0x01)
		{
		awb_addr = awb_addr + 1;
		}
	else if (hi553_jsl_selina_group_flag == 0x13)
		{
		awb_addr = awb_addr + 31;
		}
	else if (hi553_jsl_selina_group_flag == 0x37)
		{
		awb_addr = awb_addr + 61;
		}
	else 
		{
		CMR_LOGE("%s ,read hi553_jsl_selina_group_flag fail \n", __func__);
		}
	CMR_LOGE("%s   hi553_jsl_selina_group_flag =0x%x  awb_addr =0x%x\n",__func__,hi553_jsl_selina_group_flag,awb_addr);
	
	rc = hi553_jsl_selina_otp_continuous_read(s_ctrl,awb_addr , HI553_JSL_SELINA_AWB_REG_NO,
								buf,&sum_awb,&group_checksum_awb);
	if(rc < 0){
		CMR_LOGE("%s ,fail hi553_jsl_selina_otp_read_i2c\n", __func__);
		return rc;
	}
	checksum =sum_awb%255+1;
	if(checksum != group_checksum_awb){
		CMR_LOGE("%s ,awb failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,awb_addr,group_checksum_awb);
		return -1;
    }
    CMR_LOGE("%s ,awb success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,awb_addr,group_checksum_awb);
	
	hi553_jsl_selina_otp_info.rg_ratio = (buf[0]  << 8) | buf[1];
	hi553_jsl_selina_otp_info.bg_ratio = (buf[2]  << 8) | buf[3];
	hi553_jsl_selina_otp_info.gb_gr_ratio = (buf[4] << 8) | buf[5];

	gloden_rg = (buf[6]  << 8) | buf[7];
	gloden_bg = (buf[8]  << 8) | buf[9];
	gloden_gg = (buf[10]  << 8) | buf[11];
	

	CMR_LOGE("%s OTP data are rg_ratio=0x%x, bg_ratio=0x%x, gb_gr_ratio=0x%x\n", __func__,hi553_jsl_selina_otp_info.rg_ratio, hi553_jsl_selina_otp_info.bg_ratio, hi553_jsl_selina_otp_info.gb_gr_ratio);
	CMR_LOGE("%s OTP data are gloden_rg=0x%x, gloden_bg=0x%x, gloden_gg=0x%x\n", __func__,gloden_rg,gloden_bg,gloden_gg);

	if (0 == hi553_jsl_selina_otp_info.rg_ratio || 0 == hi553_jsl_selina_otp_info.bg_ratio || 0 == hi553_jsl_selina_otp_info.gb_gr_ratio){
		//if awb value read is error for zero, abnormal branch deal
		CMR_LOGE("%s OTP awb is wrong!!!\n", __func__);
		return -1;
	}
	return 0;
}
#if 0
static int hi553_jsl_selina_otp_get_af(struct msm_sensor_ctrl_t* s_ctrl,uint16_t  af_addr)
{
	uint16_t buf[HI553_JSL_SELINA_AF_REG_NO] = {0};
	int rc = 0;
	int sum_af =0;
	uint16_t checksum = 0;
	uint16_t group_checksum_af = 0;
	uint16_t vcm_offset = HI553_JSL_SELINA_QTECH_MODULE_VCM_OFFSET;

	rc = hi553_jsl_selina_otp_continuous_read(s_ctrl,af_addr+HI553_JSL_SELINA_GROUP_FLAG_NO,HI553_JSL_SELINA_AF_REG_NO,
								buf, &sum_af, &group_checksum_af);
	if(rc < 0){
		CMR_LOGE("%s ,fail hi553_jsl_selina_otp_read_i2c\n", __func__);
		return rc;
	}

	checksum = sum_af%255+1;
	if(checksum != group_checksum_af){
		CMR_LOGE("%s ,af failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,af_addr,group_checksum_af);
		return -1;
    	}
	hi553_jsl_selina_otp_info.af_macro_dac = (buf[1]  << 8)  | buf[0];
	hi553_jsl_selina_otp_info.af_infinity_dac = (buf[3]  << 8) | buf[2];
	CMR_LOGE("hi553_jsl_selina otp [af_macro_dac:%#x][af_infinity_dac:%#x]", hi553_jsl_selina_otp_info.af_macro_dac , hi553_jsl_selina_otp_info.af_infinity_dac );
	if(hi553_jsl_selina_otp_info.af_infinity_dac == 0 || hi553_jsl_selina_otp_info.af_macro_dac == 0){
		CMR_LOGE("%s OTP af is wrong!!!\n", __func__);
		return -1;
	}
	if(s_ctrl->vendor_otp_info.vendor_id  == QTECH_VENDOR_ID)
		vcm_offset = HI553_JSL_SELINA_QTECH_MODULE_VCM_OFFSET;
	if(s_ctrl->vendor_otp_info.vendor_id  == CHICONY_VENDOR_ID)
		vcm_offset = HI553_JSL_SELINA_CHICONY_MODULE_VCM_OFFSET;

	hi553_jsl_selina_otp_info.af_macro_dac += vcm_offset;
	if(hi553_jsl_selina_otp_info.af_macro_dac >= 1023)
		hi553_jsl_selina_otp_info.af_macro_dac=1023;

	if(hi553_jsl_selina_otp_info.af_infinity_dac <= vcm_offset)
		hi553_jsl_selina_otp_info.af_infinity_dac = 0;
	else
		hi553_jsl_selina_otp_info.af_infinity_dac -= vcm_offset;

	s_ctrl->afc_otp_info.macro_dac= hi553_jsl_selina_otp_info.af_macro_dac;
	s_ctrl->afc_otp_info.infinity_dac = hi553_jsl_selina_otp_info.af_infinity_dac;
	s_ctrl->afc_otp_info.starting_dac = hi553_jsl_selina_otp_info.af_infinity_dac;
	CMR_LOGE("hi553_jsl_selina otp include offset[af_macro_dac:%#x][af_infinity_dac:%#x]", hi553_jsl_selina_otp_info.af_macro_dac , hi553_jsl_selina_otp_info.af_infinity_dac );
	CMR_LOGE("%s ,af success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,af_addr,group_checksum_af);
	return 0;
}

static int hi553_jsl_selina_otp_get_lsc(struct msm_sensor_ctrl_t* s_ctrl,uint16_t  lsc_addr)
{
	uint16_t buf[HI553_JSL_SELINA_LSC_REG_NO] = {0};
	int i,j;
	int rc = 0;
	int sum_lsc=0;
	uint16_t rggb_h =0;
	uint16_t offset = 0;
	uint16_t checksum= 0;
	uint16_t group_checksum_lsc= 0;
	rc = hi553_jsl_selina_otp_continuous_read(s_ctrl,lsc_addr+HI553_JSL_SELINA_GROUP_FLAG_NO,HI553_JSL_SELINA_LSC_REG_NO,
								buf, &sum_lsc, &group_checksum_lsc);
	if(rc < 0){
		CMR_LOGE("%s ,fail hi553_jsl_selina_otp_read_i2c\n", __func__);
		return rc;
	}

	checksum = sum_lsc%255+1;
	if(checksum != group_checksum_lsc){
		CMR_LOGE("%s ,lsc failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,lsc_addr,group_checksum_lsc);
		return -1;
    }

	for(j=0; j<MSM_ROLLOFF_MAX_LIGHT; j++)
	{
		for(i=0,offset=0; i<221; i++,offset+=5)
		{
			rggb_h = offset+4;
			s_ctrl->lsc_otp_info.lsc_otp[j].r_gain[i] =  (buf[rggb_h]<<2 & 0x300) | buf[offset];
			s_ctrl->lsc_otp_info.lsc_otp[j].gr_gain[i] = (buf[rggb_h]<<4 & 0x300) | buf[offset+1];
			s_ctrl->lsc_otp_info.lsc_otp[j].gb_gain[i] = (buf[rggb_h]<<6 & 0x300) | buf[offset+2];
			s_ctrl->lsc_otp_info.lsc_otp[j].b_gain[i] =  (buf[rggb_h]<<8 & 0x300) | buf[offset+3];
		}
	}
	CMR_LOGE("%s ,lsc success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,lsc_addr,group_checksum_lsc);
	return 0;
}
#endif
static int hi553_jsl_selina_otp_init_set(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;

	uint16_t val;

	st_hi553_jsl_selina_otp_init.reg_setting =hi553_jsl_selina_otp_init_setting;
	st_hi553_jsl_selina_otp_init.size = ARRAY_SIZE(hi553_jsl_selina_otp_init_setting);
	st_hi553_jsl_selina_otp_init.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	st_hi553_jsl_selina_otp_init.data_type = MSM_CAMERA_I2C_WORD_DATA;
	st_hi553_jsl_selina_otp_init.delay = 0;

	hi553_jsl_selina_otp_read_i2c(s_ctrl,0x0f16,&val);
	rc = hi553_jsl_selina_otp_write_i2c_table(s_ctrl,&st_hi553_jsl_selina_otp_init);
	if(rc < 0){
		CMR_LOGE("%s:%d failed set otp init setting.\n", __func__, __LINE__);
		return rc;
	}

	CMR_LOGW("set  hi553_jsl_selina_otp_init_set OK\n");
	return rc;
}

static int hi553_jsl_selina_otp_display_mode(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint i = 0;
	int32_t addr = 0;
	uint16_t data = 0;
	int rc = 0;

	struct msm_camera_i2c_reg_array hi553_jsl_selina_otp_enable_setting[]=
	{
		#if 0
		{0x0f02, 0x00, 0x00},/*pll diszble*/
		{0x071a, 0x01, 0x00},/*CP TRIM_H*/
		{0x071b, 0x09, 0x00},/*IPGM TRIM_H*/
		{0x0d04, 0x01, 0x00},/*fync(otp busy)output enable*/
		{0x0d00, 0x07, 0x00},/*fync(otp busy)output drivability*/
		{0x003e, 0x10, 0x00},/*otp r/w mode*/
		{0x0a00, 0x01, 0x00},/*stand by off*/
		#endif
		{0x0a02, 0x01, 0x00},    //Fast sleep on
		{0x0a00, 0x00,  100},  // stand by on
		{0x0f02, 0x00, 0x00},    // pll disable
		{0x011a, 0x01, 0x00},    // CP TRIM_H
		{0x011b, 0x09, 0x00},    // IPGM TRIM_H
		{0x0d04, 0x01, 0x00},    // Fsync(OTP busy) Output Enable
		{0x0d00, 0x07, 0x00},    // Fsync(OTP busy) Output Drivability
		{0x003f, 0x10, 0x00},    // OTP R/W mode
		{0x0a00, 0x01, 0x10},    // stand by off
	};

	for( i = 0; i < ARRAY_SIZE(hi553_jsl_selina_otp_enable_setting); i++ ){
		addr = hi553_jsl_selina_otp_enable_setting[i].reg_addr;
		data = hi553_jsl_selina_otp_enable_setting[i].reg_data;
		rc = hi553_jsl_selina_otp_write_i2c(s_ctrl,addr, data);
		if(rc < 0){
			CMR_LOGE("%s: failed. reg:0x%x, data:0x%x\n", __func__, addr, data);
			return rc;
		}
	}
	return rc;
}

static int hi553_jsl_selina_otp_init(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	/*write init setting*/
	rc = hi553_jsl_selina_otp_init_set(s_ctrl);
	if(rc < 0){
		CMR_LOGE("%s: failed to hi553_jsl_selina otp init setting.\n", __func__);
		return rc;
	}
	/*enter display mode*/
	rc = hi553_jsl_selina_otp_display_mode(s_ctrl);
	if(rc < 0){
		CMR_LOGE("%s: failed hi553_jsl_selina_otp_display_mode.\n", __func__);
	}
	return rc;
}

static int hi553_jsl_selina_otp_exit_read(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	/*stand by on*/
	rc = hi553_jsl_selina_otp_write_i2c(s_ctrl,0x0a00, 0x00);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x0a00 stream off\n", __func__);
		return rc;
	}
	/*Fourth delay 10ms*/
	msleep(10);
	/*display mode*/
	rc = hi553_jsl_selina_otp_write_i2c(s_ctrl,0x003f, 0x00);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x003e display mode\n", __func__);
		return rc;
	}
	/*stand by off*/
	rc = hi553_jsl_selina_otp_write_i2c(s_ctrl,0x0a00, 0x01);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x0a00 stream on\n", __func__);
		return rc;
	}
	return rc;
}

static int hi553_jsl_selina_read_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t tmp_mmi_otp_flag = HI553_JSL_SELINA_MMI_OTP_FAIL;
	int rc = 0;
	enum_hi553_jsl_selina_groups group = GROUP_1;

	CMR_LOGW("%s enter\n", __func__);

	if (HI553_JSL_SELINA_OTP_FAIL_FLAG == (hi553_jsl_selina_otp_flag & HI553_JSL_SELINA_OTP_FAIL_FLAG)){
		CMR_LOGE("%s, HI553_JSL_SELINA_OTP_FAIL_FLAG\n", __func__);
		return -1;
	} else if (HI553_JSL_SELINA_OTP_SUCCESS == hi553_jsl_selina_otp_flag){
		CMR_LOGE("%s, HI553_JSL_SELINA_OTP_COMPLETE\n", __func__);
		return 0;
	}
	//initial global parameters.
	hi553_jsl_selina_otp_flag = 0;
	memset(&hi553_jsl_selina_otp_info, 0 , sizeof(hi553_jsl_selina_otp_info));
	/*init before read otp*/
	rc = hi553_jsl_selina_otp_init(s_ctrl);
	if(rc < 0){
		CMR_LOGE("faild hi553_jsl_selina_otp_init\n");
		goto GET_OTP_FAIL;
	}

	/*read module info group*/
	rc = hi553_jsl_selina_otp_get_group(s_ctrl, GROUP_TYPE_MODULE, &group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina otp get MODULE group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
	/*read module info*/
	rc =hi553_jsl_selina_otp_get_module_info(s_ctrl,hi553_jsl_selina_module_info_otp_read_addr[group][GROUP_TYPE_MODULE]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina_otp_init_read\n", __func__);
		goto GET_OTP_FAIL;
	}

	hi553_jsl_selina_otp_flag |= HI553_JSL_SELINA_OTP_MODULE_INFO_READ;
	tmp_mmi_otp_flag &= ~HI553_JSL_SELINA_MMI_OTP_MODULE_INFO_FLAG;

#if 0
	/*read af  group*/
	rc = hi553_jsl_selina_otp_get_group(s_ctrl, GROUP_TYPE_AF, &group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina otp get af group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
	/*read af info*/
	rc = hi553_jsl_selina_otp_get_af(s_ctrl,hi553_jsl_selina_module_info_otp_read_addr[group][GROUP_TYPE_AF]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina_otp_get_af\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi553_jsl_selina_otp_flag |= HI553_JSL_SELINA_OTP_AF_READ;
	tmp_mmi_otp_flag &= ~HI553_JSL_SELINA_MMI_OTP_AF_FLAG;
#endif
	/*read awb  group*/
	rc = hi553_jsl_selina_otp_get_group(s_ctrl, GROUP_TYPE_AWB, &group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina otp get awb group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
	/*read awb info*/
	rc = hi553_jsl_selina_otp_get_awb(s_ctrl,hi553_jsl_selina_module_info_otp_read_addr[group][GROUP_TYPE_AWB]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina_otp_get_awb\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi553_jsl_selina_otp_flag |= HI553_JSL_SELINA_OTP_AWB_READ;
	tmp_mmi_otp_flag &= ~HI553_JSL_SELINA_MMI_OTP_AWB_FLAG;

#if 0
	/*read lsc  group*/
	rc = hi553_jsl_selina_otp_get_group(s_ctrl, GROUP_TYPE_LSC, &group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina otp get LSC group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
	/*read lsc info*/
	rc = hi553_jsl_selina_otp_get_lsc(s_ctrl,hi553_jsl_selina_module_info_otp_read_addr[group][GROUP_TYPE_LSC]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi553_jsl_selina_otp_get_lsc\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi553_jsl_selina_otp_flag |= HI553_JSL_SELINA_OTP_LSC_READ;
	tmp_mmi_otp_flag &= ~HI553_JSL_SELINA_MMI_OTP_LSC_FLAG;
#endif

    goto EXIT_OTP_READ;

GET_OTP_FAIL:
	hi553_jsl_selina_otp_flag |= HI553_JSL_SELINA_OTP_FAIL_FLAG;

EXIT_OTP_READ:
	//exit hi553_jsl_selina otp read
	rc = hi553_jsl_selina_otp_exit_read(s_ctrl);
	if( rc < 0 ){
		CMR_LOGE("%s, failed hi553_jsl_selina_otp_exit_read\n",__func__);
		hi553_jsl_selina_otp_flag |= HI553_JSL_SELINA_OTP_FAIL_FLAG;
		tmp_mmi_otp_flag = HI553_JSL_SELINA_MMI_OTP_FAIL;
		rc = -1;
	}

	s_ctrl->hw_otp_check_flag.mmi_otp_check_flag  = tmp_mmi_otp_flag;
	pr_err("%s exit hi553_jsl_selina_mmi_otp_flag = 0x%x ret:%d\n",__func__, s_ctrl->hw_otp_check_flag.mmi_otp_check_flag, rc);
	return rc;
}

/****************************************************************************
 * FunctionName: hi553_jsl_selina_update_awb_gain;
 * Description : write R_gain,G_gain,B_gain to otp;
 * 0x200 =1x Gain
 * 0 means write AWB info succeed.
 * -1 means write AWB info failed.
 ***************************************************************************/
static int hi553_jsl_selina_update_awb_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t R_gain=1, G_gain=1 ,B_gain=1, Base_gain=1;
	if(hi553_jsl_selina_otp_info.rg_ratio == 0 || hi553_jsl_selina_otp_info.bg_ratio == 0){
		CMR_LOGE("%s: rg_ratio=%d bg_ratio=%d fail\n",__func__,hi553_jsl_selina_otp_info.rg_ratio,hi553_jsl_selina_otp_info.bg_ratio);
		return -1;
	}

	R_gain = (RG_Ratio_Typical*1000)/hi553_jsl_selina_otp_info.rg_ratio;
	B_gain = (BG_Ratio_Typical*1000)/hi553_jsl_selina_otp_info.bg_ratio;
	G_gain = 1000;
	if(R_gain < 1000 || ( B_gain < 1000 ) ){
		if(R_gain < B_gain )
			Base_gain = R_gain;
		else
			Base_gain = B_gain;
	}
	else
	{
		Base_gain = G_gain;
	}
	R_gain = 0x100*R_gain/Base_gain;
	B_gain = 0x100*B_gain/Base_gain;
	G_gain = 0x100*G_gain/Base_gain;
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x0126, G_gain>>8));       /*otp_gain_gr_h*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x0127, G_gain & 0x00ff)); /*otp_gain_gr_l*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x0128, G_gain>>8));       /*otp_gain_gb_h*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x0129, G_gain & 0x00ff)); /*otp_gain_gb_l*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x012a, R_gain>>8));       /*otp_gain_r_h*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x012b, R_gain & 0x00ff)); /*otp_gain_r_l*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x012c, B_gain>>8));       /*otp_gain_b_h*/
	loge_if_ret(hi553_jsl_selina_otp_write_i2c(s_ctrl,0x012d, B_gain & 0x00ff)); /*otp_gain_b_l*/
	pr_err("%s: hi553_jsl_selina otp R_gain=0x%x, G_gain=0x%x, B_gain=0x%x Base_gain=0x%x \n",__func__,R_gain,G_gain,B_gain,Base_gain);
	return 0;
}

/****************************************************************************
 * FunctionName: hi553_jsl_selina_set_otp_info;
 * Description : set otp data to sensor;
 ***************************************************************************/
int hi553_jsl_selina_otp_func(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
	int rc = 0;
	if(otp_function_lists[index].rg_ratio_typical){
		RG_Ratio_Typical = otp_function_lists[index].rg_ratio_typical;
	}

	if(otp_function_lists[index].bg_ratio_typical){
		BG_Ratio_Typical = otp_function_lists[index].bg_ratio_typical;
	}
	CMR_LOGW("%s, rg_ratio_typical=0x%04x,bg_ratio_typical=0x%04x\n", __func__,RG_Ratio_Typical,BG_Ratio_Typical );
	//Get otp info on the first time
	rc = hi553_jsl_selina_read_otp(s_ctrl);
	if ( rc < 0 ){
		CMR_LOGE("%s:%d otp read failed.\n", __func__, __LINE__);
	return -1;
	}

	hi553_jsl_selina_update_awb_otp(s_ctrl);
	CMR_LOGW("%s exit\n", __func__);
	return rc;
}
