
#define HW_CMR_LOG_TAG "sensor_otp_hi846_synology_1p0j0_pad"

#include <linux/hw_camera_common.h>
#include "msm_sensor.h"
#include "sensor_otp_common_if.h"
//Golden sensor typical ratio
static int RG_Ratio_Typical = 0x00DD;
static int BG_Ratio_Typical = 0x012E;
#define HI846_SYNOLOGY_1P0J0_PAD_MODULE_REG_NO   7
#define HI846_SYNOLOGY_1P0J0_PAD_AWB_REG_NO      12
#define HI846_SYNOLOGY_1P0J0_PAD_LSC_REG_NO      858
#define HI846_SYNOLOGY_1P0J0_PAD_AF_REG_NO      5
#define HI846_I2C_RETRY_TIMES 3
#define HI846_SYNOLOGY_1P0J0_PAD_FLAG_GROUP1  0x01
#define HI846_SYNOLOGY_1P0J0_PAD_FLAG_GROUP2  0x13
#define HI846_SYNOLOGY_1P0J0_PAD_FLAG_GROUP3  0x37
#define HI846_SYNOLOGY_1P0J0_PAD_OTP_VCM_OFFSET_VALUE (50)
#define HI846_SYNOLOGY_1P0J0_PAD_MACRO_MAX 1023
#define HI846_SYNOLOGY_1P0J0_PAD_DEBUG_OTP_DATA
#define loge_if_ret(x) \
{\
	if (x<0) \
	{\
		CMR_LOGE("'%s' failed", #x); \
		return -1; \
	} \
}
//OTP info struct
typedef struct hi846_synology_1p0j0_pad_otp_info {
	uint8_t  year;
	uint8_t  month;
	uint8_t  day;
	uint8_t  module_code;
	uint8_t  supplier_code;
	uint8_t  version;
	uint16_t rg_ratio;
	uint16_t bg_ratio;
	uint16_t gb_gr_ratio;
	uint16_t starting_dac;
	uint16_t infinity_dac;
	uint16_t macro_dac;
}st_hi846_synology_1p0j0_pad_otp_info;

typedef struct hi846_synology_1p0j0_pad_otp_reg_addr {
	uint16_t start_addr;
	uint16_t checksum_module_addr;
	uint16_t checksum_lsc_addr;
	uint16_t checksum_awb_addr;
	uint16_t checksum_af_addr;
}st_reg_addr;

//hi846_synology_1p0j0_pad has three groups: [1,2,3]
typedef enum hi846_synology_1p0j0_pad_groups_count{
	GROUP_1 = 0,
	GROUP_2,
	GROUP_3,
	GROUP_MAX
}enum_hi846_synology_1p0j0_pad_groups;

static uint16_t group_checksum_module = 0;
static uint16_t group_checksum_lsc = 0;
static uint16_t group_checksum_awb = 0;
static uint16_t group_checksum_af = 0;
static st_reg_addr hi846_synology_1p0j0_pad_module_info_otp_read_addr[] = {
	{0x0202,0x0212,0x0598,0x0C7D,0x0CC2},
	{0x0213,0x0223,0x08FB,0x0C9B,0x0CCA},
	{0x0224,0x0234,0x0C5E,0x0CB9,0x0CD2},
};

static st_reg_addr hi846_synology_1p0j0_pad_awb_otp_read_addr[] = {
	{0x0C60,0x0212,0x0598,0x0C7D,0x0CC2},
	{0x0C7E,0x0223,0x08FB,0x0C9B,0x0CCA},
	{0x0C9C,0x0234,0x0C5E,0x0CB9,0x0CD2},
};
static st_reg_addr hi846_synology_1p0j0_pad_lsc_otp_read_addr[] = {
	{0x0236,0x0212,0x0598,0x0C7D,0x0CC2},
	{0x0599,0x0223,0x08FB,0x0C9B,0x0CCA},
	{0x08FC,0x0234,0x0C5E,0x0CB9,0x0CD2},
};
static st_reg_addr hi846_synology_1p0j0_pad_af_otp_read_addr[] = {
	{0x0CBB,0x0212,0x0598,0x0C7D,0x0CC2},
	{0x0CC3,0x0223,0x08FB,0x0C9B,0x0CCA},
	{0x0CCB,0x0234,0x0C5E,0x0CB9,0x0CD2},
};
static struct msm_camera_i2c_reg_array hi846_synology_1p0j0_pad_otp_init_setting[]=
{
//Sensor Information////////////////////////////
//Sensor		  : Hi-846B
//Date		  : 2016-11-24
//Customer          : Huawei
////////////////////////////////////////////////

  //{0x0118, 0x0000}, //sleep On
  //{0x0000, 0x0300}, // image_orient / null

  {0x0a00, 0x0000},	//sleep On
  {0x2000, 0x100A},
  {0x2002, 0x00FF},
  {0x2004, 0x0007},
  {0x2006, 0x3FFF},
  {0x2008, 0x3FFF},
  {0x200A, 0xC216},
  {0x200C, 0x1292},
  {0x200E, 0xC01A},
  {0x2010, 0x403D},
  {0x2012, 0x000E},
  {0x2014, 0x403E},
  {0x2016, 0x0B80},
  {0x2018, 0x403F},
  {0x201A, 0x82AE},
  {0x201C, 0x1292},
  {0x201E, 0xC00C},
  {0x2020, 0x4130},
  {0x2022, 0x43E2},
  {0x2024, 0x0180},
  {0x2026, 0x4130},
  {0x2028, 0x7400},
  {0x202A, 0x5000},
  {0x202C, 0x0253},
  {0x202E, 0x0AD1},
  {0x2030, 0x2360},
  {0x2032, 0x0009},
  {0x2034, 0x5020},
  {0x2036, 0x000B},
  {0x2038, 0x0002},
  {0x203A, 0x0044},
  {0x203C, 0x0016},
  {0x203E, 0x1792},
  {0x2040, 0x7002},
  {0x2042, 0x154F},
  {0x2044, 0x00D5},
  {0x2046, 0x000B},
  {0x2048, 0x0019},
  {0x204A, 0x1698},
  {0x204C, 0x000E},
  {0x204E, 0x099A},
  {0x2050, 0x0058},
  {0x2052, 0x7000},
  {0x2054, 0x1799},
  {0x2056, 0x0310},
  {0x2058, 0x03C3},
  {0x205A, 0x004C},
  {0x205C, 0x064A},
  {0x205E, 0x0001},
  {0x2060, 0x0007},
  {0x2062, 0x0BC7},
  {0x2064, 0x0055},
  {0x2066, 0x7000},
  {0x2068, 0x1550},
  {0x206A, 0x158A},
  {0x206C, 0x0004},
  {0x206E, 0x1488},
  {0x2070, 0x7010},
  {0x2072, 0x1508},
  {0x2074, 0x0004},
  {0x2076, 0x0016},
  {0x2078, 0x03D5},
  {0x207A, 0x0055},
  {0x207C, 0x08CA},
  {0x207E, 0x2019},
  {0x2080, 0x0007},
  {0x2082, 0x7057},
  {0x2084, 0x0FC7},
  {0x2086, 0x5041},
  {0x2088, 0x12C8},
  {0x208A, 0x5060},
  {0x208C, 0x5080},
  {0x208E, 0x2084},
  {0x2090, 0x12C8},
  {0x2092, 0x7800},
  {0x2094, 0x0802},
  {0x2096, 0x040F},
  {0x2098, 0x1007},
  {0x209A, 0x0803},
  {0x209C, 0x080B},
  {0x209E, 0x3803},
  {0x20A0, 0x0807},
  {0x20A2, 0x0404},
  {0x20A4, 0x0400},
  {0x20A6, 0xFFFF},
  {0x20A8, 0xF0B2},
  {0x20AA, 0xFFEF},
  {0x20AC, 0x0A84},
  {0x20AE, 0x1292},
  {0x20B0, 0xC02E},
  {0x20B2, 0x4130},
  {0x23FE, 0xC056},
  {0x3232, 0xFC0C},
  {0x3236, 0xFC22},
  {0x3248, 0xFCA8},
  {0x326A, 0x8302},
  {0x326C, 0x830A},
  {0x326E, 0x0000},
  {0x32CA, 0xFC28},
  {0x32CC, 0xC3BC},
  {0x32CE, 0xC34C},
  {0x32D0, 0xC35A},
  {0x32D2, 0xC368},
  {0x32D4, 0xC376},
  {0x32D6, 0xC3C2},
  {0x32D8, 0xC3E6},
  {0x32DA, 0x0003},
  {0x32DC, 0x0003},
  {0x32DE, 0x00C7},
  {0x32E0, 0x0031},
  {0x32E2, 0x0031},
  {0x32E4, 0x0031},
  {0x32E6, 0xFC28},
  {0x32E8, 0xC3BC},
  {0x32EA, 0xC384},
  {0x32EC, 0xC392},
  {0x32EE, 0xC3A0},
  {0x32F0, 0xC3AE},
  {0x32F2, 0xC3C4},
  {0x32F4, 0xC3E6},
  {0x32F6, 0x0003},
  {0x32F8, 0x0003},
  {0x32FA, 0x00C7},
  {0x32FC, 0x0031},
  {0x32FE, 0x0031},
  {0x3300, 0x0031},
  {0x3302, 0x82CA},
  {0x3304, 0xC164},
  {0x3306, 0x82E6},
  {0x3308, 0xC19C},
  {0x330A, 0x001F},
  {0x330C, 0x001A},
  {0x330E, 0x0034},
  {0x3310, 0x0000},
  {0x3312, 0x0000},
  {0x3314, 0xFC94},
  {0x3316, 0xC3D8},
  {0x0A00, 0x0000},
  {0x0E04, 0x0012},
  {0x002E, 0x1111},
  {0x0032, 0x1111},
  {0x0022, 0x0008},
  {0x0026, 0x0040},
  {0x0028, 0x0017},
  {0x002C, 0x09CF},
  {0x005C, 0x2101},
  {0x0006, 0x09BC},
  {0x0008, 0x0ED8},
  {0x000E, 0x0100},
  {0x000C, 0x0022},
  {0x0A22, 0x0000},
  {0x0A24, 0x0000},
  {0x0804, 0x0000},
  {0x0A12, 0x0CC0},
  {0x0A14, 0x0990},
  {0x0074, 0x09B6},
  {0x0076, 0x0000},
  {0x051E, 0x0000},
  {0x0200, 0x0400},
  {0x0A1A, 0x0C00},
  {0x0A0C, 0x0010},
  {0x0A1E, 0x0CCF},
  {0x0402, 0x0110},
  {0x0404, 0x00F4},
  {0x0408, 0x0000},
  {0x0410, 0x008D},
  {0x0412, 0x011A},
  {0x0414, 0x864C},
  {0x021C, 0x0003},
  {0x0C00, 0x9150},
  {0x0C06, 0x0021},
  {0x0C10, 0x0040},
  {0x0C12, 0x0040},
  {0x0C14, 0x0040},
  {0x0C16, 0x0040},
  {0x0A02, 0x0100},
  {0x0A04, 0x015A},
  {0x0418, 0x0000},
  {0x012A, 0x03B4},
  {0x0120, 0x0046},
  {0x0122, 0x0376},
  {0x0B02, 0xE04D},
  {0x0B10, 0x6821},
  {0x0B12, 0x0120},
  {0x0B14, 0x0001},
  {0x2008, 0x38FD},
  {0x326E, 0x0000},
  {0x0900, 0x0300},
  {0x0902, 0xC319},
  {0x0914, 0xC109},
  {0x0916, 0x061A},
  {0x0918, 0x0407},
  {0x091A, 0x0A0B},
  {0x091C, 0x0E08},
  {0x091E, 0x0A00},
  {0x090C, 0x0427},
  {0x090E, 0x0069},
  {0x0954, 0x0089},
  {0x0956, 0x0000},
  {0x0958, 0xCA80},
  {0x095A, 0x9240},
  {0x0F08, 0x2F04},
  {0x0F30, 0x001F},
  {0x0F36, 0x001F},
  {0x0F04, 0x3A00},
  {0x0F32, 0x025A},
  {0x0F38, 0x025A},
  {0x0F2A, 0x4124},
  {0x006A, 0x0100},
  {0x004C, 0x0100},
  {0x0A00, 0x0100},//sleep Off

};
/*OTP READ STATUS*/
#define HI846_SYNOLOGY_1P0J0_PAD_OTP_MODULE_INFO_READ   (1 << 0)
#define HI846_SYNOLOGY_1P0J0_PAD_OTP_AWB_READ           (1 << 1)
#define HI846_SYNOLOGY_1P0J0_PAD_OTP_LSC_READ           (1 << 2)
#define HI846_SYNOLOGY_1P0J0_PAD_OTP_AF_READ          (1 << 3)
#define HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG          (1 << 4)

#define HI846_SYNOLOGY_1P0J0_PAD_OTP_SUCCESS (HI846_SYNOLOGY_1P0J0_PAD_OTP_MODULE_INFO_READ|HI846_SYNOLOGY_1P0J0_PAD_OTP_AWB_READ|HI846_SYNOLOGY_1P0J0_PAD_OTP_LSC_READ|HI846_SYNOLOGY_1P0J0_PAD_OTP_AF_READ)

#define HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_MODULE_INFO_FLAG  (1 << 0)
#define HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_AWB_FLAG          (1 << 1)
#define HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_LSC_FLAG          (1 << 2)
#define HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_AF_FLAG          (1 << 3)
#define HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_FAIL (HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_MODULE_INFO_FLAG|HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_AWB_FLAG|HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_LSC_FLAG|HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_AF_FLAG) 

#define MODULE_FLAG_ADDR 0x0201
#define AF_FLAG_ADDR  0x0CBA
#define AWB_FLAG_ADDR 0x0C5F
#define LSC_FLAG_ADDR 0x0235

static st_hi846_synology_1p0j0_pad_otp_info hi846_synology_1p0j0_pad_otp_info = {0};
static uint16_t  hi846_synology_1p0j0_pad_otp_flag   = 0;
static struct msm_camera_i2c_reg_setting st_hi846_synology_1p0j0_pad_otp_init;
/****************************************************************************
 * FunctionName: hi846_synology_1p0j0_pad_otp_write_i2c;
 * Description : write otp info via i2c;
 ***************************************************************************/
static int32_t hi846_synology_1p0j0_pad_otp_write_i2c(struct msm_sensor_ctrl_t *s_ctrl, int32_t addr, uint16_t data)
{
	int32_t rc = 0;

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(s_ctrl->sensor_i2c_client,
		addr,
		data,
		MSM_CAMERA_I2C_BYTE_DATA);

	if ( rc < 0 ){
		CMR_LOGE("%s fail, rc = %d! addr = 0x%x, data = 0x%x\n", __func__, rc, addr, data);
#ifdef CONFIG_HUAWEI_DSM
		camera_report_dsm_err_otp(s_ctrl, DSM_CAMERA_OTP_I2C_ERR, addr, OTP_WRITE_I2C_ERR);
#endif
	}

	return rc;
}

/****************************************************************************
 * FunctionName: hi846_synology_1p0j0_pad_otp_write_i2c_table;
 * Description : write otp info via i2c;
 ***************************************************************************/
static int32_t hi846_synology_1p0j0_pad_otp_write_i2c_table(struct msm_sensor_ctrl_t *s_ctrl, struct msm_camera_i2c_reg_setting *write_setting)
{
	int32_t rc = 0;
        int32_t i = 0;

	if(NULL == write_setting){
	CMR_LOGE("%s fail,noting to write i2c \n", __func__);
	return -1;
	}
	for(i = 0; i < HI846_I2C_RETRY_TIMES; i++){
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
 * FunctionName: hi846_synology_1p0j0_pad_otp_read_i2c;
 * Description : read otp info via i2c;
 ***************************************************************************/
static int32_t hi846_synology_1p0j0_pad_otp_read_i2c(struct msm_sensor_ctrl_t *s_ctrl,uint32_t addr, uint16_t *data)
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

static int32_t hi846_synology_1p0j0_pad_otp_single_read(struct msm_sensor_ctrl_t *s_ctrl,uint32_t addr, uint16_t *data)
{
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl, 0x070A, (addr>>8)&0xFF));
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl, 0x070B, addr&0xFF));
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl, 0x0702, 0x01));
	loge_if_ret(hi846_synology_1p0j0_pad_otp_read_i2c(s_ctrl,0x0708,data));
	return 0;
}

static int hi846_synology_1p0j0_pad_otp_get_group(struct msm_sensor_ctrl_t* s_ctrl,int32_t addr,enum_hi846_synology_1p0j0_pad_groups * group_sel)
{
	uint16_t group_num = 0xff;
	int rc = 0;

	rc = hi846_synology_1p0j0_pad_otp_single_read(s_ctrl,addr,&group_num);
	CMR_LOGW("%s read group addr=0x%x, data=0x%x\n",__func__,addr,group_num);
	if(rc < 0){
		CMR_LOGE("%s read group addr=0x%x\n",__func__,addr);
	}
	switch(group_num)
	{
		case HI846_SYNOLOGY_1P0J0_PAD_FLAG_GROUP3:
		{
			*group_sel = GROUP_3;
			break;
		}
		case HI846_SYNOLOGY_1P0J0_PAD_FLAG_GROUP2:
		{
			*group_sel = GROUP_2;
			break;
		}
		case HI846_SYNOLOGY_1P0J0_PAD_FLAG_GROUP1:
		{
			*group_sel = GROUP_1;
			break;
		}
		default:
		{
			CMR_LOGE("%s hi846_synology_1p0j0_pad module burn otp bad\n",__func__);
			rc = -1;
			break;
		}
	}
	return rc;
}

static int hi846_synology_1p0j0_pad_otp_continuous_read(struct msm_sensor_ctrl_t* s_ctrl,st_reg_addr addr,int rd_num,uint16_t * rd_buf,int *sum)
{
	int rc = 0;
	int i=0;
	int sum1=0;
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl, 0x070A, (addr.start_addr)>>8&0xFF));//continus single read mode
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl, 0x070B, (addr.start_addr)&0xFF));
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl, 0x0702, 0x01));
    for(i = 0; i < rd_num; i++)//id part
    {
        rc = hi846_synology_1p0j0_pad_otp_read_i2c(s_ctrl, 0x0708, &rd_buf[i]);
		if(rc < 0){
			CMR_LOGE("%s ,%d,fail hi846_synology_1p0j0_pad_otp_read_i2c,i=%d,rc=%d\n", __func__,__LINE__,i,rc);
			return -1;
		}
		sum1 += rd_buf[i];
		#ifdef HI846_SYNOLOGY_1P0J0_PAD_DEBUG_OTP_DATA
		CMR_LOGE("%s,OTP Info:addr = 0x%x, data = 0x%x\n",__func__,addr.start_addr+i,rd_buf[i]);
		#endif
    }

	if(sum != NULL){
		*sum= sum1;
	}

	return rc;
}
static int hi846_synology_1p0j0_pad_otp_get_module_info(struct msm_sensor_ctrl_t* s_ctrl,st_reg_addr module_addr)
{
	uint16_t buf[HI846_SYNOLOGY_1P0J0_PAD_MODULE_REG_NO] = {0};
	int rc = 0;
	int sum_module=0;
    uint16_t checksum= 0;
	rc = hi846_synology_1p0j0_pad_otp_continuous_read(s_ctrl,module_addr,HI846_SYNOLOGY_1P0J0_PAD_MODULE_REG_NO, buf,&sum_module);
	if(rc < 0){
		CMR_LOGE("%s ,fail read hi846_synology_1p0j0_pad module info \n", __func__);
		return -1;
	}
	CMR_LOGW("%s module info module id %d year 20%02d month %d day %d. lens id 0x%x\n",
			__func__, buf[0], buf[2], buf[3], buf[4], buf[6]);

	checksum = sum_module%255+1;//just use the last part checksum value to set final checksum flag validation
	if(checksum != group_checksum_module){
		CMR_LOGE("%s ,failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,module_addr.checksum_module_addr,group_checksum_module);
		return -1;
    }
    hi846_synology_1p0j0_pad_otp_info.module_code = buf[0];
    hi846_synology_1p0j0_pad_otp_info.year  = buf[2];
    hi846_synology_1p0j0_pad_otp_info.month = buf[3];
    hi846_synology_1p0j0_pad_otp_info.day   = buf[4];
    s_ctrl->vendor_otp_info.vendor_id = hi846_synology_1p0j0_pad_otp_info.module_code;
    CMR_LOGW("%s ,module_info success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
                        __func__, checksum,module_addr.checksum_module_addr,group_checksum_module);
	return rc;
}
static int hi846_synology_1p0j0_pad_otp_get_awb(struct msm_sensor_ctrl_t* s_ctrl,st_reg_addr  awb_addr)
{
	uint16_t buf[HI846_SYNOLOGY_1P0J0_PAD_AWB_REG_NO] = {0};
	int rc = 0;
    int sum_awb=0;
	uint16_t checksum= 0;
	rc = hi846_synology_1p0j0_pad_otp_continuous_read(s_ctrl,awb_addr,HI846_SYNOLOGY_1P0J0_PAD_AWB_REG_NO, buf,&sum_awb);
	if(rc < 0){
		CMR_LOGE("%s ,fail hi846_synology_1p0j0_pad_otp_read_i2c\n", __func__);
		return rc;
	}
	checksum =sum_awb%255+1;//just use the last part checksum value to set final checksum flag validation
	if(checksum != group_checksum_awb){
		CMR_LOGE("%s ,awb failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,awb_addr.checksum_awb_addr,group_checksum_awb);
		return -1;
    }
    CMR_LOGW("%s ,awb success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,awb_addr.checksum_awb_addr,group_checksum_awb);
	hi846_synology_1p0j0_pad_otp_info.rg_ratio = (buf[0]  << 8) | buf[1];
	hi846_synology_1p0j0_pad_otp_info.bg_ratio = (buf[2]  << 8) | buf[3];
	hi846_synology_1p0j0_pad_otp_info.gb_gr_ratio = (buf[4] << 8) | buf[5];

	CMR_LOGW("%s OTP data are rg_ratio=0x%x, bg_ratio=0x%x, gb_gr_ratio=0x%x\n", __func__,hi846_synology_1p0j0_pad_otp_info.rg_ratio, hi846_synology_1p0j0_pad_otp_info.bg_ratio, hi846_synology_1p0j0_pad_otp_info.gb_gr_ratio);

	if (0 == hi846_synology_1p0j0_pad_otp_info.rg_ratio || 0 == hi846_synology_1p0j0_pad_otp_info.bg_ratio || 0 == hi846_synology_1p0j0_pad_otp_info.gb_gr_ratio){
		//if awb value read is error for zero, abnormal branch deal
		CMR_LOGE("%s OTP awb is wrong!!!\n", __func__);
		return -1;
	}
	return 0;
}

static int hi846_synology_1p0j0_pad_otp_get_lsc(struct msm_sensor_ctrl_t* s_ctrl,st_reg_addr  lsc_addr)
{
	uint16_t buf[HI846_SYNOLOGY_1P0J0_PAD_LSC_REG_NO] = {0};
	int rc = 0;
	int sum_lsc=0;
	uint16_t checksum= 0;
	rc = hi846_synology_1p0j0_pad_otp_continuous_read(s_ctrl,lsc_addr,HI846_SYNOLOGY_1P0J0_PAD_LSC_REG_NO, buf,&sum_lsc);
	if(rc < 0){
		CMR_LOGE("%s ,fail hi846_synology_1p0j0_pad_otp_read_i2c\n", __func__);
		return rc;
	}
	checksum = sum_lsc%255+1;//just use the last part checksum value to set final checksum flag validation
	if(checksum != group_checksum_lsc){
		CMR_LOGE("%s ,lsc failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,lsc_addr.checksum_lsc_addr,group_checksum_lsc);
		return -1;
    }
	CMR_LOGW("%s ,lsc success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,lsc_addr.checksum_lsc_addr,group_checksum_lsc);
	return 0;
}

static int hi846_synology_1p0j0_pad_otp_get_af(struct msm_sensor_ctrl_t* s_ctrl,st_reg_addr  af_addr)
{
	uint16_t buf[HI846_SYNOLOGY_1P0J0_PAD_AF_REG_NO] = {0};
	int rc = 0;
	int sum_af=0;
	uint16_t checksum= 0;
	rc = hi846_synology_1p0j0_pad_otp_continuous_read(s_ctrl,af_addr,HI846_SYNOLOGY_1P0J0_PAD_AF_REG_NO, buf,&sum_af);
	if(rc < 0){
		CMR_LOGE("%s ,fail hi846_synology_1p0j0_pad_otp_read_i2c\n", __func__);
		return rc;
	}
	checksum = sum_af%255+1;//just use the last part checksum value to set final checksum flag validation
	if(checksum != group_checksum_af){
		CMR_LOGE("%s ,af failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,af_addr.checksum_af_addr,group_checksum_af);
		return -1;
    }
	CMR_LOGW("%s ,af success verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,af_addr.checksum_af_addr,group_checksum_af);
	hi846_synology_1p0j0_pad_otp_info.infinity_dac = (buf[1]  << 8) | buf[2];
	hi846_synology_1p0j0_pad_otp_info.macro_dac = (buf[3]  << 8) | buf[4];
	if((0 == hi846_synology_1p0j0_pad_otp_info.infinity_dac) || (0 == hi846_synology_1p0j0_pad_otp_info.macro_dac) ||
	    (hi846_synology_1p0j0_pad_otp_info.macro_dac <= hi846_synology_1p0j0_pad_otp_info.infinity_dac))
	{
		CMR_LOGE("hi846_synology_1p0j0_pad_otp_info.infinity_dac = 0x%x\n", hi846_synology_1p0j0_pad_otp_info.infinity_dac);
		CMR_LOGE("hi846_synology_1p0j0_pad_otp_info.macro_dac = 0x%x\n", hi846_synology_1p0j0_pad_otp_info.macro_dac);
	       return -1;
	}
	if(hi846_synology_1p0j0_pad_otp_info.infinity_dac <= HI846_SYNOLOGY_1P0J0_PAD_OTP_VCM_OFFSET_VALUE)
	{
		hi846_synology_1p0j0_pad_otp_info.infinity_dac = 1;
	}
	else
	{
		hi846_synology_1p0j0_pad_otp_info.infinity_dac -= HI846_SYNOLOGY_1P0J0_PAD_OTP_VCM_OFFSET_VALUE;
	}
	hi846_synology_1p0j0_pad_otp_info.macro_dac += HI846_SYNOLOGY_1P0J0_PAD_OTP_VCM_OFFSET_VALUE;
	if(hi846_synology_1p0j0_pad_otp_info.macro_dac >= HI846_SYNOLOGY_1P0J0_PAD_MACRO_MAX)
	{
            hi846_synology_1p0j0_pad_otp_info.macro_dac = HI846_SYNOLOGY_1P0J0_PAD_MACRO_MAX;
	}
	hi846_synology_1p0j0_pad_otp_info.starting_dac = hi846_synology_1p0j0_pad_otp_info.infinity_dac;

	return 0;
}

static int hi846_synology_1p0j0_pad_otp_init_set(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	st_hi846_synology_1p0j0_pad_otp_init.reg_setting =hi846_synology_1p0j0_pad_otp_init_setting;
	st_hi846_synology_1p0j0_pad_otp_init.size = ARRAY_SIZE(hi846_synology_1p0j0_pad_otp_init_setting);
	st_hi846_synology_1p0j0_pad_otp_init.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
	st_hi846_synology_1p0j0_pad_otp_init.data_type = MSM_CAMERA_I2C_WORD_DATA;
	st_hi846_synology_1p0j0_pad_otp_init.delay = 0;
	rc = hi846_synology_1p0j0_pad_otp_write_i2c_table(s_ctrl,&st_hi846_synology_1p0j0_pad_otp_init);
	if(rc < 0){
		CMR_LOGE("%s:%d failed set otp init setting.\n", __func__, __LINE__);
		return rc;
	}

	CMR_LOGW("set  otp init setting to sensor OK\n");
	return rc;
}

static int hi846_synology_1p0j0_pad_otp_display_mode(struct msm_sensor_ctrl_t *s_ctrl)
{
	int i = 0;
	int32_t addr = 0;
	uint16_t data = 0;
	int rc = 0;

	struct msm_camera_i2c_reg_array hi846_synology_1p0j0_pad_otp_enable_setting[]=
	{
		{0x0f02, 0x00, 0x00},/*pll diszble*/
		{0x071a, 0x01, 0x00},/*CP TRIM_H*/
		{0x071b, 0x09, 0x00},/*IPGM TRIM_H*/
		{0x0d04, 0x01, 0x00},/*fync(otp busy)output enable*/
		{0x0d00, 0x07, 0x00},/*fync(otp busy)output drivability*/
		{0x003e, 0x10, 0x00},/*otp r/w mode*/
		{0x070f, 0x05, 0x00},/*otp data rewrite*/
		{0x0a00, 0x01, 0x00},/*stand by off*/
	};

	/*fast sleep on*/
	rc = hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x0a02, 0x01);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x0a02\n", __func__);
		return rc;
	}
	/*stand by on*/
	rc = hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x0a00, 0x00);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x0a02\n", __func__);
		return rc;
	}
	/* delay 10ms*/
	msleep(10);
	for( i = 0; i < ARRAY_SIZE(hi846_synology_1p0j0_pad_otp_enable_setting); i++ ){
		addr = hi846_synology_1p0j0_pad_otp_enable_setting[i].reg_addr;
		data = hi846_synology_1p0j0_pad_otp_enable_setting[i].reg_data;
		rc = hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,addr, data);
		if(rc < 0){
			CMR_LOGE("%s: failed. reg:0x%x, data:0x%x\n", __func__, addr, data);
			return rc;
		}
	}
	return rc;
}
static int hi846_synology_1p0j0_pad_otp_init(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;

	/*write init setting*/
	rc = hi846_synology_1p0j0_pad_otp_init_set(s_ctrl);
	if(rc < 0){
		CMR_LOGE("%s: failed to hi846_synology_1p0j0_pad otp init setting.\n", __func__);
		return rc;
	}

	/*enter display mode*/
	rc = hi846_synology_1p0j0_pad_otp_display_mode(s_ctrl);
	if(rc < 0){
		CMR_LOGE("%s: failed hi846_synology_1p0j0_pad_otp_display_mode.\n", __func__);
	}

	return rc;
}
static int hi846_synology_1p0j0_pad_otp_exit_read(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rc = 0;
	/*stand by on*/
	rc = hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x0a00, 0x00);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x0a00 stream off\n", __func__);
		return rc;
	}
	/*Fourth delay 10ms*/
	msleep(10);
	/*display mode*/
	rc = hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x003e, 0x00);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x003e display mode\n", __func__);
		return rc;
	}
	/*stand by off*/
	rc = hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x0a00, 0x01);
	if(rc < 0){
		CMR_LOGE("%s: failed write 0x0a00 stream on\n", __func__);
		return rc;
	}

	return rc;
}

static int hi846_synology_1p0j0_pad_read_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t tmp_mmi_otp_flag = HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_FAIL;
	int rc = 0;
	enum_hi846_synology_1p0j0_pad_groups group = GROUP_1;

	CMR_LOGW("%s enter\n", __func__);
	if (HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG == (hi846_synology_1p0j0_pad_otp_flag & HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG)){
		CMR_LOGE("%s, HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG\n", __func__);
		return -1;
	} else if (HI846_SYNOLOGY_1P0J0_PAD_OTP_SUCCESS == hi846_synology_1p0j0_pad_otp_flag){
		CMR_LOGW("%s, HI846_SYNOLOGY_1P0J0_PAD_OTP_COMPLETE\n", __func__);
		return 0;
	}
	//initial global parameters.
	hi846_synology_1p0j0_pad_otp_flag = 0;
	memset(&hi846_synology_1p0j0_pad_otp_info, 0 , sizeof(hi846_synology_1p0j0_pad_otp_info));

	/*init before read otp*/
	rc = hi846_synology_1p0j0_pad_otp_init(s_ctrl);
	if(rc < 0){
		CMR_LOGE("faild hi846_synology_1p0j0_pad_otp_init\n");
		goto GET_OTP_FAIL;
	}

	/*read module info group*/
	rc = hi846_synology_1p0j0_pad_otp_get_group(s_ctrl,MODULE_FLAG_ADDR,&group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get MODULE group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
	/*read checksum of module info*/
	rc = hi846_synology_1p0j0_pad_otp_single_read(s_ctrl,hi846_synology_1p0j0_pad_module_info_otp_read_addr[group].checksum_module_addr,&group_checksum_module);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get module checksum. \n", __func__);
		goto GET_OTP_FAIL;
	}
	/*read module info*/
	rc =hi846_synology_1p0j0_pad_otp_get_module_info(s_ctrl,hi846_synology_1p0j0_pad_module_info_otp_read_addr[group]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad_otp_init_read\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi846_synology_1p0j0_pad_otp_flag |= HI846_SYNOLOGY_1P0J0_PAD_OTP_MODULE_INFO_READ;
	tmp_mmi_otp_flag &= ~HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_MODULE_INFO_FLAG;

    /*read lsc  group*/
	rc = hi846_synology_1p0j0_pad_otp_get_group(s_ctrl,LSC_FLAG_ADDR,&group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get lsc group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
    /*read checksum of lsc*/
	rc = hi846_synology_1p0j0_pad_otp_single_read(s_ctrl,hi846_synology_1p0j0_pad_module_info_otp_read_addr[group].checksum_lsc_addr,&group_checksum_lsc);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get lsc checksum. \n", __func__);
		goto GET_OTP_FAIL;
	}
	/*read lsc info*/
	rc = hi846_synology_1p0j0_pad_otp_get_lsc(s_ctrl,hi846_synology_1p0j0_pad_lsc_otp_read_addr[group]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad_otp_get_lsc\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi846_synology_1p0j0_pad_otp_flag |= HI846_SYNOLOGY_1P0J0_PAD_OTP_LSC_READ;
	tmp_mmi_otp_flag &= ~HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_LSC_FLAG;

	/*read awb  group*/
	rc = hi846_synology_1p0j0_pad_otp_get_group(s_ctrl,AWB_FLAG_ADDR,&group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get awb group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
    /*read checksum of awb*/
	rc = hi846_synology_1p0j0_pad_otp_single_read(s_ctrl,hi846_synology_1p0j0_pad_module_info_otp_read_addr[group].checksum_awb_addr,&group_checksum_awb);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get awb checksum. \n", __func__);
		goto GET_OTP_FAIL;
	}
	/*read awb info*/
	rc = hi846_synology_1p0j0_pad_otp_get_awb(s_ctrl,hi846_synology_1p0j0_pad_awb_otp_read_addr[group]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad_otp_get_awb\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi846_synology_1p0j0_pad_otp_flag |= HI846_SYNOLOGY_1P0J0_PAD_OTP_AWB_READ;
	tmp_mmi_otp_flag &= ~HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_AWB_FLAG;

	/*read af  group*/
	rc = hi846_synology_1p0j0_pad_otp_get_group(s_ctrl,AF_FLAG_ADDR,&group);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get af group,group:%d,\n", __func__,group);
		goto GET_OTP_FAIL;
	}
	/*read checksum of af*/
	rc = hi846_synology_1p0j0_pad_otp_single_read(s_ctrl,hi846_synology_1p0j0_pad_module_info_otp_read_addr[group].checksum_af_addr,&group_checksum_af);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad otp get af checksum. \n", __func__);
		goto GET_OTP_FAIL;
	}

	/*read af info*/
	rc = hi846_synology_1p0j0_pad_otp_get_af(s_ctrl,hi846_synology_1p0j0_pad_af_otp_read_addr[group]);
	if ( rc < 0 ){
		CMR_LOGE("%s,faild hi846_synology_1p0j0_pad_otp_get_af\n", __func__);
		goto GET_OTP_FAIL;
	}
	hi846_synology_1p0j0_pad_otp_flag |= HI846_SYNOLOGY_1P0J0_PAD_OTP_AF_READ;
	tmp_mmi_otp_flag &= ~HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_AF_FLAG;
        goto EXIT_OTP_READ;

GET_OTP_FAIL:
	hi846_synology_1p0j0_pad_otp_flag |= HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG;

EXIT_OTP_READ:
	//exit hi846_synology_1p0j0_pad otp read
	rc = hi846_synology_1p0j0_pad_otp_exit_read(s_ctrl);
	if( rc < 0 ){
		CMR_LOGE("%s, failed hi846_synology_1p0j0_pad_otp_exit_read\n",__func__);
		hi846_synology_1p0j0_pad_otp_flag |= HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG;
		tmp_mmi_otp_flag = HI846_SYNOLOGY_1P0J0_PAD_MMI_OTP_FAIL;
		rc = -1;
	}

	s_ctrl->hw_otp_check_flag.mmi_otp_check_flag  = tmp_mmi_otp_flag;
	CMR_LOGW("%s exit hi846_synology_1p0j0_pad_mmi_otp_flag = 0x%x\n",__func__, s_ctrl->hw_otp_check_flag.mmi_otp_check_flag);
	return rc;
}

/****************************************************************************
 * FunctionName: hi846_synology_1p0j0_pad_update_awb_gain;
 * Description : write R_gain,G_gain,B_gain to otp;
 * 0x200 =1x Gain
 * 0 means write AWB info succeed.
 * -1 means write AWB info failed.
 ***************************************************************************/
static int hi846_synology_1p0j0_pad_update_awb_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t R_gain=0x200, G_gain=0x200 ,B_gain=0x200;
	if(hi846_synology_1p0j0_pad_otp_info.rg_ratio == 0 || hi846_synology_1p0j0_pad_otp_info.bg_ratio == 0){
		CMR_LOGE("%s: rg_ratio=%d bg_ratio=%d fail\n",__func__,hi846_synology_1p0j0_pad_otp_info.rg_ratio,hi846_synology_1p0j0_pad_otp_info.bg_ratio);
		return -1;
	}
	//calculate G gain
	//0x200 = 1x gain
	R_gain = (RG_Ratio_Typical*0x200)/hi846_synology_1p0j0_pad_otp_info.rg_ratio;
	B_gain = (BG_Ratio_Typical*0x200)/hi846_synology_1p0j0_pad_otp_info.bg_ratio;
	G_gain = 0x200;

	if(R_gain < B_gain ){
        if(R_gain < 0x200){
            B_gain = 0x200 * B_gain/R_gain;
            G_gain = 0x200 * G_gain/R_gain;
            R_gain = 0x200;
        }
    }else{
        if(B_gain < 0x200){
            R_gain = 0x200 * R_gain/B_gain;
            G_gain = 0x200 * G_gain/B_gain;
            B_gain = 0x200;
        }
    }

	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x0078,G_gain>>8));       /*otp_gain_gr_h*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x0079,G_gain & 0x00ff)); /*otp_gain_gr_l*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x007A,G_gain>>8));       /*otp_gain_gb_h*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x007B,G_gain & 0x00ff)); /*otp_gain_gb_l*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x007C,R_gain>>8));       /*otp_gain_r_h*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x007D,R_gain & 0x00ff)); /*otp_gain_r_l*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x007E,B_gain>>8));       /*otp_gain_b_h*/
	loge_if_ret(hi846_synology_1p0j0_pad_otp_write_i2c(s_ctrl,0x007F,B_gain & 0x00ff)); /*otp_gain_b_l*/
	CMR_LOGW("%s: R_gain=0x%x, G_gain=0x%x, B_gain=0x%x \n",__func__,R_gain,G_gain,B_gain);
	return 0;
}
/*
**************************************************************************
* FunctionName: hi846_synology_1p0j0_pad_set_af_to_platform;
* Description : set the otp info into sctrl;
* Input         : s_ctrl:the struct of sensor controller;
* Output       : NA;
* ReturnValue:NA;
* Other         : NA;
**************************************************************************
*/
static void hi846_synology_1p0j0_pad_set_af_to_platform(struct msm_sensor_ctrl_t* s_ctrl)
{

	if (HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG == (hi846_synology_1p0j0_pad_otp_flag & HI846_SYNOLOGY_1P0J0_PAD_OTP_FAIL_FLAG))
	{
		CMR_LOGE("%s invalid otp info!\n", __func__);
		return;
	}

	/*set VCM*/
	s_ctrl->afc_otp_info.starting_dac = hi846_synology_1p0j0_pad_otp_info.starting_dac;
	s_ctrl->afc_otp_info.infinity_dac = hi846_synology_1p0j0_pad_otp_info.infinity_dac;
	s_ctrl->afc_otp_info.macro_dac = hi846_synology_1p0j0_pad_otp_info.macro_dac;

	CMR_LOGW("%s, starting_dac=0x%04x,macro_dac=0x%04x\n", __func__,s_ctrl->afc_otp_info.starting_dac,s_ctrl->afc_otp_info.macro_dac );

	return;
}

/****************************************************************************
 * FunctionName: hi846_synology_1p0j0_pad_set_otp_info;
 * Description : set otp data to sensor;
 ***************************************************************************/
int hi846_synology_1p0j0_pad_otp_func(struct msm_sensor_ctrl_t *s_ctrl,int index)
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
	rc = hi846_synology_1p0j0_pad_read_otp(s_ctrl);
	if ( rc < 0 ){
		CMR_LOGE("%s:%d otp read failed.\n", __func__, __LINE__);
	return -1;
	}
	hi846_synology_1p0j0_pad_update_awb_otp(s_ctrl);
	hi846_synology_1p0j0_pad_set_af_to_platform(s_ctrl);
	CMR_LOGW("%s exit\n", __func__);
	return rc;
}

