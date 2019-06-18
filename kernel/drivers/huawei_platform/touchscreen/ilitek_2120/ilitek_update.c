#include "ilitek_ts.h"
#include <linux/firmware.h>
#include <linux/vmalloc.h>

#define ILITEK_FW_FILENAME	"ilitek_i2c.bin"

extern struct i2c_data i2c;
static char touch_info[50] = {0};
static unsigned char * CTPM_FW = NULL;

int inwrite(unsigned int address)
{
	uint8_t outbuff[64];
	int data, ret;
	outbuff[0] = 0x25;
	outbuff[1] = (char)((address & 0x000000FF) >> 0);
	outbuff[2] = (char)((address & 0x0000FF00) >> 8);
	outbuff[3] = (char)((address & 0x00FF0000) >> 16);
	ret = ilitek_i2c_write(i2c.client, outbuff, 4);
	udelay(10);
	ret = ilitek_i2c_read(i2c.client, outbuff, 4);
	data = (outbuff[0] + outbuff[1] * 256 + outbuff[2] * 256 * 256 + outbuff[3] * 256 * 256 * 256);
	tp_log_debug("%s, data=0x%x, outbuff[0]=%x, outbuff[1]=%x, outbuff[2]=%x, outbuff[3]=%x\n", __func__, data, outbuff[0], outbuff[1], outbuff[2], outbuff[3]);
	return data;
}

int outwrite(unsigned int address, unsigned int data, int size)
{
	int ret, i;
	char outbuff[64];
	outbuff[0] = 0x25;
	outbuff[1] = (char)((address & 0x000000FF) >> 0);
	outbuff[2] = (char)((address & 0x0000FF00) >> 8);
	outbuff[3] = (char)((address & 0x00FF0000) >> 16);
	for(i = 0; i < size; i++)
	{
		outbuff[i + 4] = (char)(data >> (8 * i));
	}
	ret = ilitek_i2c_write(i2c.client, outbuff, size + 4);
	return ret;
}

static int ilitek_set_appinfo(unsigned char firmware_ver[4])
{
	int ret = -1;
	snprintf(touch_info, sizeof(touch_info),
		"ilitek_2120_%x.%x.%02x",
		firmware_ver[1],
		firmware_ver[2],
		firmware_ver[3]);
	ret = app_info_set("touch_panel", touch_info);
	if (ret < 0) {
		tp_log_err("%s(line %d): error,ret=%d\n",__func__,__LINE__,ret);
	}
	return ret;
}

int ilitek_upgrade_firmware(void)
{
	int ret = 0, upgrade_status = 0, i = 0, j = 0, k = 0, ap_len = 0, df_len = 0;
	unsigned char buf[64] = {0};
	unsigned long ap_startaddr = 0, df_startaddr = 0, ap_endaddr = 0, df_endaddr = 0, ap_checksum = 0, df_checksum = 0, temp = 0;
	unsigned char firmware_ver[4];
	int retry = 0;
	const struct firmware *fw;
	int set_appinfo_ret;
	CTPM_FW = vmalloc(64 * 1024 + 1);		
	
	if (!(*CTPM_FW)) {
		tp_log_info("ilitek CTPM_FW alloctation memory failed\n");
	}
	ap_startaddr = ( CTPM_FW[0] << 16 ) + ( CTPM_FW[1] << 8 ) + CTPM_FW[2];
	ap_endaddr = ( CTPM_FW[3] << 16 ) + ( CTPM_FW[4] << 8 ) + CTPM_FW[5];
	ap_checksum = ( CTPM_FW[6] << 16 ) + ( CTPM_FW[7] << 8 ) + CTPM_FW[8];
	df_startaddr = ( CTPM_FW[9] << 16 ) + ( CTPM_FW[10] << 8 ) + CTPM_FW[11];
	df_endaddr = ( CTPM_FW[12] << 16 ) + ( CTPM_FW[13] << 8 ) + CTPM_FW[14];
	df_checksum = ( CTPM_FW[15] << 16 ) + ( CTPM_FW[16] << 8 ) + CTPM_FW[17];
	firmware_ver[0] = CTPM_FW[18];
	firmware_ver[1] = CTPM_FW[19];
	firmware_ver[2] = CTPM_FW[20];
	firmware_ver[3] = CTPM_FW[21];
	df_len = ( CTPM_FW[22] << 16 ) + ( CTPM_FW[23] << 8 ) + CTPM_FW[24];
	ap_len = ( CTPM_FW[25] << 16 ) + ( CTPM_FW[26] << 8 ) + CTPM_FW[27];
	ap_startaddr = 0x0;
	ap_endaddr = 0xDFFF;
	tp_log_info("ap_startaddr=0x%lX, ap_endaddr=0x%lX,\n", ap_startaddr, ap_endaddr);
	tp_log_debug("df_startaddr=0x%lX, df_endaddr=0x%lX, df_checksum=0x%lX\n", df_startaddr, df_endaddr, df_checksum);
	ret = request_firmware(&fw, ILITEK_FW_FILENAME, &i2c.client->dev);
	if (ret) {
		dev_err(&i2c.client->dev, "[ILITEK] failed to request firmware %s: %d\n",
			ILITEK_FW_FILENAME, ret);
		return ret;
	}
	tp_log_info("ilitek fw->size = %d\n", (int)fw->size);
	for (ret = 0; ret < fw->size; ret++) {
		CTPM_FW[ret + 32] = fw->data[ret];
	}
	firmware_ver[0] = 0x0;
	firmware_ver[1] = CTPM_FW[0xD100+ 32];
	firmware_ver[2] = CTPM_FW[0xD101+ 32];
	firmware_ver[3] = CTPM_FW[0xD102+ 32];
	release_firmware(fw);

	tp_log_info("firmware_ver[0] = %d, firmware_ver[1] = %d firmware_ver[2]=%d firmware_ver[3]=%d\n",firmware_ver[0], firmware_ver[1], firmware_ver[2], firmware_ver[3]);

	for(i = 0; i < 4; i++)
	{
		tp_log_info("i2c.firmware_ver[%d] = %d, firmware_ver[%d] = %d\n", i, i2c.firmware_ver[i], i, firmware_ver[i ]);
		if((i2c.firmware_ver[i] > firmware_ver[i ]) || ((i == 3) && (i2c.firmware_ver[3] == firmware_ver[3])))
		{
			ret = 1;
			set_appinfo_ret = ilitek_set_appinfo(i2c.firmware_ver);
			if (set_appinfo_ret) {
				tp_log_info("%s:%d set app info err\n",__FUNCTION__,__LINE__);
			}
			tp_log_info("ilitek_upgrade_firmware Do not need update\n"); 
			return 1;
		}
		else if(i2c.firmware_ver[i] < firmware_ver[i ])
		{
			break;
		}
	}

	if (ic2120) {
Retry:
		ret = outwrite(0x181062, 0x0, 0);
		tp_log_debug("%s, release Power Down Release mode\n", __func__);
		ret = outwrite(0x041000, 0xab, 1);
		ret = outwrite(0x041004, 0x66aa5500, 4);
		ret = outwrite(0x04100d, 0x00, 1);
		msleep(5);
		for (i = 0; i <= 0xd000;i += 0x1000) {
			tp_log_debug("%s, i = %X\n", __func__, i);
			ret = outwrite(0x041000, 0x06, 1);
			msleep(1);
			ret = outwrite(0x041004, 0x66aa5500, 4);
			msleep(1);
			temp = (i << 8) + 0x20;
			ret = outwrite(0x041000, temp, 4);
			msleep(1);
			ret = outwrite(0x041004, 0x66aa5500, 4);
			msleep(15);
			for (j = 0; j < 50; j++) {
				ret = outwrite(0x041000, 0x05, 1);
				ret = outwrite(0x041004, 0x66aa5500, 4);
				msleep(1);
				buf[0] = inwrite(0x041013);
				tp_log_debug("%s, buf[0] = %X\n", __func__, buf[0]);
				if (buf[0] == 0) {
					break;
				}
				else {
					msleep(2);
				};
			}
		}
		msleep(100);
		
		for(i = ap_startaddr; i < ap_endaddr; i += 0x20) {
			tp_log_debug("%s, i = %X\n", __func__, i);
			ret = outwrite(0x041000, 0x06, 1);
			ret = outwrite(0x041004, 0x66aa5500, 4);
			temp = (i << 8) + 0x02;
			ret = outwrite(0x041000, temp, 4);
			ret = outwrite(0x041004, 0x66aa551f, 4);
			buf[0] = 0x25;
			buf[3] = (char)((0x041020  & 0x00FF0000) >> 16);
			buf[2] = (char)((0x041020  & 0x0000FF00) >> 8);
			buf[1] = (char)((0x041020  & 0x000000FF));
			for(k = 0; k < 32; k++)
			{
				buf[4 + k] = CTPM_FW[i + 32 + k];
			}

			if(ilitek_i2c_write(i2c.client, buf, 36) < 0) {
				tp_log_info("%s, write data error, address = 0x%X, start_addr = 0x%X, end_addr = 0x%X\n", __func__, (int)i, (int)ap_startaddr, (int)ap_endaddr);
				return 3;
			}
			upgrade_status = (i * 100) / ap_len;
			mdelay(3);
		}
		buf[0] = 0x1b;
		buf[1] = 0x62;
		buf[2] = 0x10;
		buf[3] = 0x18;
		ilitek_i2c_write(i2c.client, buf, 4);
		ilitek_reset(i2c.reset_gpio);
		for (ret = 0; ret < 30; ret++ ) {
			j = ilitek_poll_int();
			tp_log_info("ilitek int status = %d\n", j);
			if (j == 0) {
				break;
			}
			else {
				mdelay(5);
			}
		}
		if (ret >= 30) {
			tp_log_err("ilitek reset but int not pull low so retry\n");
			if (retry < 2) {
				retry++;
				goto Retry;
			}
		}
		else {
			tp_log_info("ilitek reset  int  pull low  write 0x10 cmd\n");
		}
		buf[0] = 0x10;
		ilitek_i2c_write(i2c.client, buf, 1);
		ret = ilitek_i2c_read(i2c.client, buf, 3);
		tp_log_info("%s, buf = %X, %X, %X\n", __func__, buf[0], buf[1], buf[2]);
		if (buf[1] >= 0x80) {
			tp_log_info("upgrade ok ok \n");
			set_appinfo_ret = ilitek_set_appinfo(firmware_ver);
			if (set_appinfo_ret) {
				tp_log_info("%s:%d set app info err\n",__FUNCTION__,__LINE__);
			}
		}else {
			if (retry < 2) {
				retry++;
				goto Retry;
			}
		}
	}
	msleep(10);
	if (CTPM_FW) {
		vfree(CTPM_FW);
		CTPM_FW = NULL;
	}
	return 2;
}

