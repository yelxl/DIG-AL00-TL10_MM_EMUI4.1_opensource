#ifndef _FOCAL_MCAPTESTLIB_H
#define _FOCAL_MCAPTESTLIB_H

#define TX_NUM_MAX 30
#define RX_NUM_MAX 30

//enum boolean {false = 0, true = 1,};
#define boolean unsigned char
#define false 0
#define true  1

typedef int (*FTS_I2c_Read_Function)(unsigned char * , int , unsigned char *, int);
typedef int (*FTS_I2c_Write_Function)(unsigned char * , int );

int Init_I2C_Read_Func(FTS_I2c_Read_Function fpI2C_Read);
int Init_I2C_Write_Func(FTS_I2c_Write_Function fpI2C_Write);
//int InitTestParam(struct Test_ConfigParam_SCap *testparam);

int SetParamData(char * TestParamData);
int focal_save_error_data(char * databuf, int databuflen);
void FreeTestParamData(void);
boolean StartTestTP(void);		//return true pass,else ng

#endif