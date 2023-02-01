#define HW_CMR_LOG_TAG "sensor_otp_s5k5e8_blx_selina"
#include <linux/hw_camera_common.h>
#include "msm_sensor.h"
#include "sensor_otp_common_if.h"

//Golden sensor typical ratio
#define IS_HELITECH_MODULE 0

#if IS_HELITECH_MODULE
#define RG_Ratio_Typical  0x2a7
#define BG_Ratio_Typical  0x289
//static int GbGr_Ratio_Typical = 0x3fc;
#define S5K5E8_MODULE_REG_NO   8
#define S5K5E8_AWB_REG_NO      12
#else
#define RG_Ratio_Typical  0x140
#define BG_Ratio_Typical  0x152
//static int GbGr_Ratio_Typical = 0x3fc;
#define S5K5E8_MODULE_REG_NO   7
#define S5K5E8_AWB_REG_NO      6
#endif

#define S5K5E8_AF_REG_NO       4
#define S5K5E8_LSC_REG_NO     360
#define S5K5E8_LSC_PAGE4_REG_NO     16
#define S5K5E8_I2C_RETRY_TIMES 3

#define INFINITY_MARGIN    30
#define MACRO_MARGIN       25
#define OTP_FLAG		0x0a04
#define AWBCHECKSUM		0x0A14
#define AWBINFO			0x0A0D
#define MODULEINFO		0x0A05
#define OTP_CHECKSUM		0x0A13


#define loge_if_ret(x) \
{\
	if (x<0) \
	{\
		CMR_LOGE("'%s' failed", #x); \
		return -1; \
	} \
}

typedef enum s5k5e8_blx_selina_groups_type{
        MODULEINF_START = 0,
        AWB_START,
        AF_START,
        LSC_START,
        GTYPE_MAX
}enum_s5k5e8_blx_selina_type;


//OTP info struct
typedef struct s5k5e8_blx_selina_otp_info {
	uint8_t  vendor_id;
	uint8_t  year;
	uint8_t  month;
	uint8_t  day;
	uint8_t  lens_id;
	uint8_t  vcm_id;
	uint8_t  driver_ic_id;
	uint8_t  sensor_id;

	uint16_t rg_ratio;
	uint16_t bg_ratio;
	uint16_t gb_gr_ratio;

	uint16_t rg_typical_ratio;
	uint16_t bg_typical_ratio;
	uint16_t gb_gr_typical_ratio;

	uint16_t af_macro_dac;
	uint16_t af_infinity_dac;
}st_s5k5e8_blx_selina_otp_info;


/*OTP READ STATUS*/
#define S5K5E8_OTP_MODULE_INFO_READ   (1 << 0)
#define S5K5E8_OTP_AWB_READ           (1 << 1)
//#define S5K5E8_OTP_AF_READ            (1 << 2)
//#define S5K5E8_OTP_LSC_READ           (1 << 3)
#define S5K5E8_OTP_FAIL_FLAG          (1 << 4)

#define S5K5E8_OTP_SUCCESS (S5K5E8_OTP_MODULE_INFO_READ|S5K5E8_OTP_AWB_READ)  //(S5K5E8_OTP_MODULE_INFO_READ|S5K5E8_OTP_AWB_READ|S5K5E8_OTP_AF_READ|S5K5E8_OTP_LSC_READ)

#define S5K5E8_MMI_OTP_MODULE_INFO_FLAG  (1 << 0)
#define S5K5E8_MMI_OTP_AWB_FLAG          (1 << 1)
//#define S5K5E8_MMI_OTP_AF_FLAG           (1 << 2)
//#define S5K5E8_MMI_OTP_LSC_FLAG          (1 << 3)
#define S5K5E8_MMI_OTP_FAIL (S5K5E8_MMI_OTP_MODULE_INFO_FLAG|S5K5E8_MMI_OTP_AWB_FLAG)  //(S5K5E8_MMI_OTP_MODULE_INFO_FLAG|S5K5E8_MMI_OTP_AWB_FLAG|S5K5E8_MMI_OTP_AF_FLAG|S5K5E8_OTP_LSC_READ)

static st_s5k5e8_blx_selina_otp_info s5k5e8_blx_selina_otp_info = {0};
static uint16_t  s5k5e8_blx_selina_otp_flag   = 0;

/****************************************************************************
 * FunctionName: s5k5e8_blx_selina_otp_write_i2c;
 * Description : write otp info via i2c;
 ***************************************************************************/
static int32_t s5k5e8_blx_selina_otp_write_i2c(struct msm_sensor_ctrl_t *s_ctrl, int32_t addr, uint16_t data)
{
	int32_t rc = 0;
	int32_t i = 0;

	for(i = 0; i < S5K5E8_I2C_RETRY_TIMES; i++){
		rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_write(
 		s_ctrl->sensor_i2c_client,
 		addr,
 		data,
 		MSM_CAMERA_I2C_BYTE_DATA);

		if (rc < 0) {
			pr_err("%s, failed i = %d",__func__,i);
			continue;
		}
		break;
	}

	if ( rc < 0 ){
		pr_err("%s fail, rc = %d! addr = 0x%x, data = 0x%x\n", __func__, rc, addr, data);
#ifdef CONFIG_HUAWEI_DSM
		camera_report_dsm_err_otp(s_ctrl, DSM_CAMERA_OTP_I2C_ERR, addr, OTP_WRITE_I2C_ERR);
#endif
	}

	return rc;
}

/****************************************************************************
 * FunctionName: s5k5e8_blx_selina_otp_read_i2c;
 * Description : read otp info via i2c;
 ***************************************************************************/
static int32_t s5k5e8_blx_selina_otp_read_i2c(struct msm_sensor_ctrl_t *s_ctrl,uint32_t addr, uint16_t *data)
{
	int32_t rc = 0;

	rc = s_ctrl->sensor_i2c_client->i2c_func_tbl->i2c_read(s_ctrl->sensor_i2c_client,
				addr,
				data,
				MSM_CAMERA_I2C_BYTE_DATA);
	if ( rc < 0 ){
		pr_err("%s fail, rc = %d! addr = 0x%x, data = 0x%x\n", __func__, rc, addr, *data);
#ifdef CONFIG_HUAWEI_DSM
		camera_report_dsm_err_otp(s_ctrl, DSM_CAMERA_OTP_I2C_ERR, addr, OTP_READ_I2C_ERR);
#endif
	}
	return rc;
}
#if 0    //zhangqiangqiang  del
static int s5k5e8_blx_selina_otp_set_page(struct msm_sensor_ctrl_t* s_ctrl, uint32_t page_no)
{

	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0a00,0x04));
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0a02,page_no));
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0a00,0x01));
	return 0;
}
#endif
static int s5k5e8_blx_selina_read_otp_checksum(struct msm_sensor_ctrl_t* s_ctrl, uint16_t addr, uint16_t *checksum_otp)
{
	int32_t  rc =-1;

	if(!addr)
	{
		pr_err("addr == 0, checksum_otp is null.\n");
		return -1;
	}
	rc = s5k5e8_blx_selina_otp_read_i2c(s_ctrl,addr,checksum_otp);
	if(rc < 0){
		pr_err("%s read checksum from addr=0x%x\n",__func__,addr);
		return -1;
	}
	return rc;
}
static int s5k5e8_blx_selina_otp_continuous_read(struct msm_sensor_ctrl_t* s_ctrl,uint16_t addr,int rd_num,
						uint16_t * rd_buf,int *sum)
{
	int rc = 0;
	int i=0;
	int sum1=0;
	
	for(i = 0; i < rd_num; i++)
	{
	    rc = s5k5e8_blx_selina_otp_read_i2c(s_ctrl, addr, &rd_buf[i]);
		addr++;
		if(rc < 0){
			pr_err("%s ,%d,fail s5k5e8_blx_selina_otp_read_i2c,i=%d,rc=%d\n", __func__,__LINE__,i,rc);
			return -1;
		}
		sum1 += rd_buf[i];
	}

	if(sum != NULL){
		*sum= sum1;
	}
	return rc;
}

static int s5k5e8_blx_selina_module_info_sum_module=0;//zhangqiangqiang  add
static int s5k5e8_blx_selina_read_otp_module_info(struct msm_sensor_ctrl_t* s_ctrl,uint16_t module_addr,int checksum_otp)
{
	uint16_t buf[S5K5E8_MODULE_REG_NO] = {0};
	int rc = 0;
	//int sum_module=0;
	//uint16_t checksum= 0;//zhangqiangqiang del

	rc = s5k5e8_blx_selina_otp_continuous_read(s_ctrl,module_addr,S5K5E8_MODULE_REG_NO,buf,&s5k5e8_blx_selina_module_info_sum_module);//zhangqiangqiang
	if(rc < 0){
		pr_err("%s ,fail read s5k5e8_blx_selina module info \n", __func__);
		return -1;
	}
	CMR_LOGW("%s year 20%d month %d day %d module code:0x%x module version:0x%x software version:0x%x Supplier code:0x%x\n",
			__func__, buf[4],buf[5], buf[6], buf[1], buf[2], buf[3], buf[0]);
	return rc;
}
static int s5k5e8_blx_selina_read_otp_awb(struct msm_sensor_ctrl_t* s_ctrl,uint16_t  awb_addr,int checksum_otp)
{
	uint16_t buf[S5K5E8_AWB_REG_NO] = {0};
	int rc = -1;
    	int sum_awb=0;
	uint16_t checksum= 0;
	rc = s5k5e8_blx_selina_otp_continuous_read(s_ctrl,awb_addr,S5K5E8_AWB_REG_NO,buf,&sum_awb);
	if(rc < 0){
		pr_err("%s ,fail s5k5e8_blx_selina_otp_read_i2c\n", __func__);
		return rc;
	}

	checksum =sum_awb%255+1; // modfied for HELITECH

	if(checksum != checksum_otp){
		pr_err("%s ,awb failed verify checksum ,checksum=0x%x,checksum_reg=0x%x\n",
					__func__, checksum,checksum_otp);
		return -1;
	}
	CMR_LOGW("%s ,awb success verify checksum ,checksum=0x%x,checksum_reg=0x%x\n",
					__func__, checksum,checksum_otp);
	s5k5e8_blx_selina_otp_info.rg_ratio = (buf[0]  << 8) | buf[1];
	s5k5e8_blx_selina_otp_info.bg_ratio = (buf[2]  << 8) | buf[3];
	s5k5e8_blx_selina_otp_info.gb_gr_ratio = (buf[4] << 8) | buf[5];
	CMR_LOGW("%s OTP data is rg_ratio=0x%x, bg_ratio=0x%x, gb_gr_ratio=0x%x\n", __func__,s5k5e8_blx_selina_otp_info.rg_ratio, s5k5e8_blx_selina_otp_info.bg_ratio, s5k5e8_blx_selina_otp_info.gb_gr_ratio);
	if (0 == s5k5e8_blx_selina_otp_info.rg_ratio || 0 == s5k5e8_blx_selina_otp_info.bg_ratio || 0 == s5k5e8_blx_selina_otp_info.gb_gr_ratio){
		pr_err("%s OTP awb is wrong!!!\n", __func__);
		return -1;
	}
	return 0;
}

#if 0  //zhangqiangqiang
static int s5k5e8_blx_selina_read_otp_af(struct msm_sensor_ctrl_t* s_ctrl,uint16_t  af_addr,int checksum_otp)
{
	uint16_t buf[S5K5E8_AF_REG_NO] = {0};
	int rc = 0;
	int sum_af =0;
	uint16_t checksum = 0;
	uint16_t otp_step_bound = 0;

	rc = s5k5e8_blx_selina_otp_continuous_read(s_ctrl,af_addr,S5K5E8_AF_REG_NO,
								buf, &sum_af);
	if(rc < 0){
		pr_err("%s ,fail s5k5e8_blx_selina_otp_read_i2c\n", __func__);
		return rc;
	}

	checksum = sum_af%255+1;
	if(checksum != checksum_otp){
		pr_err("%s ,af failed verify checksum ,checksum=0x%x,group_checksum_addr=0x%x,group_checksum=0x%x\n",
					__func__, checksum,af_addr,checksum_otp);
		return -1;
    	}
	s5k5e8_blx_selina_otp_info.af_macro_dac = (buf[0]  << 8)  | buf[1];
	s5k5e8_blx_selina_otp_info.af_infinity_dac = (buf[2]  << 8) | buf[3];
	CMR_LOGW("s5k5e8_blx_selina otp [af_macro_dac:%#x][af_infinity_dac:%#x]", s5k5e8_blx_selina_otp_info.af_macro_dac , s5k5e8_blx_selina_otp_info.af_infinity_dac );
	if(s5k5e8_blx_selina_otp_info.af_infinity_dac == 0 || s5k5e8_blx_selina_otp_info.af_macro_dac == 0){
		pr_err("%s OTP af is wrong!!!\n", __func__);
		return -1;
	}

    	/* adjust af_driver_ptr */
	otp_step_bound = s5k5e8_blx_selina_otp_info.af_macro_dac - s5k5e8_blx_selina_otp_info.af_infinity_dac;
	s5k5e8_blx_selina_otp_info.af_macro_dac = s5k5e8_blx_selina_otp_info.af_macro_dac + ((MACRO_MARGIN* otp_step_bound)/100);
	if(s5k5e8_blx_selina_otp_info.af_macro_dac >= 1023)
		s5k5e8_blx_selina_otp_info.af_macro_dac=1023;

	if(s5k5e8_blx_selina_otp_info.af_infinity_dac <= ((INFINITY_MARGIN* otp_step_bound)/100))
		s5k5e8_blx_selina_otp_info.af_infinity_dac = 0;
	else
	s5k5e8_blx_selina_otp_info.af_infinity_dac = s5k5e8_blx_selina_otp_info.af_infinity_dac - ((INFINITY_MARGIN* otp_step_bound)/100);

	s_ctrl->afc_otp_info.macro_dac= s5k5e8_blx_selina_otp_info.af_macro_dac;
	s_ctrl->afc_otp_info.infinity_dac = s5k5e8_blx_selina_otp_info.af_infinity_dac;
	s_ctrl->afc_otp_info.starting_dac = s5k5e8_blx_selina_otp_info.af_infinity_dac;

	CMR_LOGW("s5k5e8_blx_selina otp aftre [af_macro_dac:%#x][af_infinity_dac:%#x]", s5k5e8_blx_selina_otp_info.af_macro_dac , s5k5e8_blx_selina_otp_info.af_infinity_dac );

	CMR_LOGW("%s ,af success verify checksum ,checksum=0x%x,checksum_otp=0x%x\n",
					__func__, checksum,checksum_otp);
	return 0;
}

static int s5k5e8_blx_selina_read_otp_lsc(struct msm_sensor_ctrl_t* s_ctrl,uint16_t  lsc_addr,int checksum_otp)
{
	uint16_t buf[S5K5E8_LSC_REG_NO] = {0};
	int i;
	int rc = 0;
	int sum_lsc=0;
	int sum_lsc1=0;

	uint16_t checksum= 0;
	uint16_t satrt_addr = 0x0A04;
	uint16_t page_no = 5;
	uint16_t num_read = 64;
	
	rc = s5k5e8_blx_selina_otp_continuous_read(s_ctrl,lsc_addr,S5K5E8_LSC_PAGE4_REG_NO,
								buf, &sum_lsc);
	sum_lsc1 = sum_lsc;
	for(i=0;i<6;i++)
	{
		memset(buf, 0 , sizeof(buf));
		s5k5e8_blx_selina_otp_set_page(s_ctrl,page_no++);
		s5k5e8_blx_selina_otp_continuous_read(s_ctrl,satrt_addr,num_read,
								buf, &sum_lsc);
		sum_lsc1 += sum_lsc;
	}
	if(rc < 0){
		pr_err("%s ,fail s5k5e8_blx_selina_otp_read_i2c\n", __func__);
		return rc;
	}
	
	checksum = sum_lsc1%255+1;
	if(checksum != checksum_otp){
		pr_err("%s ,lsc failed verify checksum ,checksum=0x%x,OTP_checksum=0x%x\n",
					__func__, checksum,checksum_otp);
		return -1;
    }
	return 0;
}
#endif

static int s5k5e8_blx_selina_start_read_otp(struct msm_sensor_ctrl_t* s_ctrl, uint32_t page_no)
{
	CMR_LOGW("s5k5e8_start_read_otp page = 0x%02x.\n",page_no);
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0A00, 0x04));//make initial state
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0A02, page_no));   //Select the page to write by writing to 0xD0000A02 0x00~0x0F
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0A00, 0x01));   //Enter read mode by writing 01h to 0xD0000A00
	mdelay(2);//wait time > 47us
	return 1;
}

static int s5k5e8_blx_selina_stop_read_otp(struct msm_sensor_ctrl_t* s_ctrl)
{
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0A00,0x04));//make initial state
	loge_if_ret(s5k5e8_blx_selina_otp_write_i2c(s_ctrl,0x0a00,0x00));//Disable NVM controller
	return 1;
}

static int s5k5e8_blx_selina_read_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t tmp_mmi_otp_flag = S5K5E8_MMI_OTP_FAIL;
	uint16_t otp_flag = 0;
	int rc = -1;
	uint16_t checksum_otp= 0;
	uint16_t page_no = 4;
	uint16_t otp_addr_offset= 0;
	CMR_LOGW("%s enter\n", __func__);
	if (S5K5E8_OTP_FAIL_FLAG == (s5k5e8_blx_selina_otp_flag & S5K5E8_OTP_FAIL_FLAG)){
		pr_err("%s, S5K5E8_OTP_FAIL_FLAG\n", __func__);
		return -1;
	} else  if (S5K5E8_OTP_SUCCESS == s5k5e8_blx_selina_otp_flag){
		pr_err("%s, S5K5E8_OTP_COMPLETE\n", __func__);
		return 0;
	}
	//initial global parameters.
	s5k5e8_blx_selina_otp_flag = 0;
	memset(&s5k5e8_blx_selina_otp_info, 0 , sizeof(s5k5e8_blx_selina_otp_info));
	//enable page 4
	s5k5e8_blx_selina_start_read_otp(s_ctrl,page_no);

/*zhangqiangqiang  add start */

#if IS_HELITECH_MODULE
	s5k5e8_blx_selina_otp_read_i2c(s_ctrl,0x0a1b,&otp_flag);
	pr_err("group2  otp_flag =0x%x\n",otp_flag);
		if(otp_flag == 1 )
			{
			/*read module info check_sum*/
			rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, 0x0A31, &checksum_otp);
			if ( rc < 0 ){
				pr_err("%s,faild s5k5e8_blx_selina otp get MODULE checksum,checksum_otp:%d,\n", __func__,checksum_otp);
				goto GET_OTP_FAIL;
				}

			/*read module info*/
			rc =s5k5e8_blx_selina_read_otp_module_info(s_ctrl,0x0A1C,checksum_otp);
			if ( rc < 0 ){
				pr_err("%s,faild s5k5e8_blx_selina_otp_init_read\n", __func__);
				goto GET_OTP_FAIL;
				}

			s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_MODULE_INFO_READ;
			tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_MODULE_INFO_FLAG;

			/*read awb check_sum*/
			rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, 0x0A31, &checksum_otp);//zhangqiangqiang 0x0A1A
			if ( rc < 0 ){
				pr_err("%s,faild s5k5e8_blx_selina otp get AWB checksum,checksum_otp %d\n", __func__,checksum_otp);
				goto GET_OTP_FAIL;
				}
			/*read awb info*/
			rc = s5k5e8_blx_selina_read_otp_awb(s_ctrl,0x0A25,checksum_otp);//zhangqiangqiang 0x0A1A
			if ( rc < 0 ){
				pr_err("%s,faild s5k5e8_blx_selina_otp_get_awb\n", __func__);
				goto GET_OTP_FAIL;
				}
			s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_AWB_READ;
			tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_AWB_FLAG;

			}
		 else{

/*zhangqiangqiang  add end */

		//chekc flag
		s5k5e8_blx_selina_otp_read_i2c(s_ctrl,0x0a04,&otp_flag);
		pr_err("group1  otp_flag =0x%x\n",otp_flag);
		if(otp_flag != 1 ){
			pr_err("%s,faild read s5k5e8_blx_selina otp the otp flag is:%d,\n", __func__,otp_flag);
			goto GET_OTP_FAIL;
			}	
		/*read module info check_sum*/
		rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, 0x0A1A, &checksum_otp);
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina otp get MODULE checksum,checksum_otp:%d,\n", __func__,checksum_otp);
			goto GET_OTP_FAIL;
		}

		/*read module info*/
		rc =s5k5e8_blx_selina_read_otp_module_info(s_ctrl,0x0A05,checksum_otp);
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina_otp_init_read\n", __func__);
			goto GET_OTP_FAIL;
		}

		s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_MODULE_INFO_READ;
		tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_MODULE_INFO_FLAG;

		/*read awb check_sum*/
		rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, 0x0A1A, &checksum_otp);//zhangqiangqiang 0x0A1A
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina otp get AWB checksum,checksum_otp %d\n", __func__,checksum_otp);
			goto GET_OTP_FAIL;
		}
		/*read awb info*/
		rc = s5k5e8_blx_selina_read_otp_awb(s_ctrl,0x0A0E,checksum_otp);//zhangqiangqiang 0x0A1A
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina_otp_get_awb\n", __func__);
			goto GET_OTP_FAIL;
		}
		s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_AWB_READ;
		tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_AWB_FLAG;
	}
#else
	//yujinyun add for HELITECH
		s5k5e8_blx_selina_otp_read_i2c(s_ctrl,OTP_FLAG,&otp_flag);
		if ( otp_flag == 0x00 ){
			pr_err("The AWB otp is empty\n");
			goto GET_OTP_FAIL;
		} else if ( otp_flag == 0x01 ){
			CMR_LOGW("The AWB otp group1 Valid\n");
			otp_addr_offset = 0;
		} else if ( otp_flag == 0x11 ) {
			CMR_LOGW("The AWB otp group2 Valid\n");
			otp_addr_offset = 0x10;
		} else {
			pr_err("The otp flag is invalid\n");
			goto GET_OTP_FAIL;
		}
		rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, OTP_CHECKSUM + otp_addr_offset, &checksum_otp);
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina otp get MODULE checksum,checksum_otp:%d,\n", __func__,checksum_otp);
			goto GET_OTP_FAIL;
		}
		/*read module info*/
		rc =s5k5e8_blx_selina_read_otp_module_info(s_ctrl,MODULEINFO + otp_addr_offset,checksum_otp);
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina_otp_init_read\n", __func__);
			goto GET_OTP_FAIL;
		}
		s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_MODULE_INFO_READ;
		tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_MODULE_INFO_FLAG;
		/*read awb check_sum*/
		rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, AWBCHECKSUM + otp_addr_offset, &checksum_otp);
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina otp get AWB checksum,checksum_otp %d\n", __func__,checksum_otp);
			goto GET_OTP_FAIL;
		}
		/*read awb info*/
		rc = s5k5e8_blx_selina_read_otp_awb(s_ctrl,AWBINFO + otp_addr_offset,checksum_otp);
		if ( rc < 0 ){
			pr_err("%s,faild s5k5e8_blx_selina_otp_get_awb\n", __func__);
			goto GET_OTP_FAIL;
		}
		s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_AWB_READ;
		tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_AWB_FLAG;
#endif
#if 0    //zhangqiangqiang
	/*read af check_sum*/
	rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, 0x0A19, &checksum_otp);
	if ( rc < 0 ){
		pr_err("%s,faild s5k5e8_blx_selina otp get AF checksum,checksum_otp %d\n", __func__,checksum_otp);
		goto GET_OTP_FAIL;
	}

	/*read af info*/
	rc = s5k5e8_blx_selina_read_otp_af(s_ctrl,0x0A13,checksum_otp);
	if ( rc < 0 ){
		pr_err("%s,faild s5k5e8_blx_selina_otp_get_af\n", __func__);
		goto GET_OTP_FAIL;
	}
	s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_AF_READ;
	tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_AF_FLAG;

	/*read lsc check_sum*/
	rc = s5k5e8_blx_selina_read_otp_checksum(s_ctrl, 0x0A1A, &checksum_otp);
	if ( rc < 0 ){
		pr_err("%s,faild s5k5e8_blx_selina otp get LSC checksum,checksum_otp %d\n", __func__,checksum_otp);
		goto GET_OTP_FAIL;
	}
	/*read lsc info*/
	rc = s5k5e8_blx_selina_read_otp_lsc(s_ctrl,0x0A34,checksum_otp);
	if ( rc < 0 ){
		pr_err("%s,faild s5k5e8_blx_selina_otp_get_lsc\n", __func__);
		goto GET_OTP_FAIL;
	}
	s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_LSC_READ;
	tmp_mmi_otp_flag &= ~S5K5E8_MMI_OTP_LSC_FLAG;
#endif
	
		goto EXIT_OTP_READ;


GET_OTP_FAIL:
	s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_FAIL_FLAG;
	
EXIT_OTP_READ:
	//exit s5k5e8 otp read
	rc = s5k5e8_blx_selina_stop_read_otp(s_ctrl);
	if( rc < 0 ){
		pr_err("%s, failed s5k5e8_otp_exit_read\n",__func__);
		s5k5e8_blx_selina_otp_flag |= S5K5E8_OTP_FAIL_FLAG;
		tmp_mmi_otp_flag = S5K5E8_MMI_OTP_FAIL;
		rc = -1;
		}
	
	s_ctrl->hw_otp_check_flag.mmi_otp_check_flag  = tmp_mmi_otp_flag;
	pr_err("%s exit hi553_mmi_otp_flag = 0x%x ret:%d\n",__func__, s_ctrl->hw_otp_check_flag.mmi_otp_check_flag, rc);
	return rc;
}


/****************************************************************************
 * FunctionName: s5k5e8_blx_selina_update_awb_gain;
 * Description : write R_gain,G_gain,B_gain to otp;
 * 0x200 =1x Gain
 * 0 means write AWB info succeed.
 * -1 means write AWB info failed.
 ***************************************************************************/
static void s5k5e8_blx_selina_update_awb_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	int rg, bg, R_gain, G_gain, B_gain, Base_gain;
	
	rg = s5k5e8_blx_selina_otp_info.rg_ratio;	
	bg = s5k5e8_blx_selina_otp_info.bg_ratio;
	
	//calculate G gain
	R_gain= RG_Ratio_Typical* 256 / rg;
	B_gain= BG_Ratio_Typical* 256 / bg;
	G_gain= 256;
	
	if (R_gain< 256 || B_gain< 256)
	{
	if (R_gain< B_gain)
	Base_gain= R_gain;
	else
	Base_gain= B_gain;
	}
	else
	{
	Base_gain= G_gain;
	}
	
	R_gain= 256 * R_gain/ (Base_gain);
	B_gain= 256 * B_gain/ (Base_gain);
	G_gain= 256 * G_gain/ (Base_gain);
	CMR_LOGW("%s  is R_gain=0x%x, B_gain=0x%x, G_gain=0x%x\n", __func__,R_gain, B_gain, G_gain);
	// update sensor WB gain
	if (R_gain>256) {
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x210, R_gain>> 8);
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x211, R_gain& 0x00ff);
	}
	if (G_gain>256) {
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x20e, G_gain>> 8);
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x20f, G_gain& 0x00ff);
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x214, G_gain>> 8);
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x215, G_gain& 0x00ff);
	}
	if (B_gain>256) {
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x212, B_gain>> 8);
	s5k5e8_blx_selina_otp_write_i2c(s_ctrl, 0x213, B_gain& 0x00ff);
	}
	CMR_LOGW("%s  is  end\n", __func__);
	
}

/****************************************************************************
 * FunctionName: s5k5e8_blx_selina_set_otp_info;
 * Description : set otp data to sensor;
 ***************************************************************************/
int s5k5e8_blx_selina_otp_func(struct msm_sensor_ctrl_t *s_ctrl,int index)
{
	int rc = 0;
	//Get otp info on the first time
	pr_err("%s:%d start.\n", __func__, __LINE__);
	rc = s5k5e8_blx_selina_read_otp(s_ctrl);
	if ( rc < 0 ){
		pr_err("%s:%d otp read failed.\n", __func__, __LINE__);
	return -1;
	}

	s5k5e8_blx_selina_update_awb_otp(s_ctrl);
	pr_err("%s exit\n", __func__);
	return rc;
}
