/******************** (C) COPYRIGHT 2012-2010 ADHON ********************
* File Name          : modbus_protl.h
* Author             : WangJiangZhu
* Version            : V1.0.1
* Date               : 08/05/2012
* Description        : Modbus标准协议接入
*******************************************************************************/

#ifndef __MODBUS_PROTL_H__
#define __MODBUS_PROTL_H__

/* 用户支持的指令 */
#define  MODBUS_CODE_1      0x01  /* 读取一组逻辑线圈状态 */
#define  MODBUS_CODE_2      0x02  /* 读取一组开关输入状态 */
#define  MODBUS_CODE_3      0x03  /* 查询保持寄存器 */
#define  MODBUS_CODE_4      0x04  /* 查询输入寄存器 */
#define  MODBUS_CODE_5      0x05  /* 写位数据 */
#define  MODBUS_CODE_6      0x06  /* 写多字节 */
#define  MODBUS_CODE_15     0x0F  /* 写位数据 */
#define  MODBUS_CODE_16     0x10  /* 写多字节 */

//Coil相关宏定义
#define  COIL_X0_ADDR       1   /* 对应控制器X0输入口电平 */
#define  COIL_X1_ADDR       2
#define  COIL_X2_ADDR       3
#define  COIL_X3_ADDR       4
#define  COIL_X4_ADDR       5
#define  COIL_X5_ADDR       6
#define  COIL_IN1_ADDR      7   /* 对应控制器IN1输入口电平 */
#define  COIL_IN2_ADDR      8
#define  COIL_IN3_ADDR      9
#define  COIL_IN4_ADDR      10
#define  COIL_IN5_ADDR      11
#define  COIL_IN6_ADDR      12
#define  COIL_Y0_ADDR       13   /* 对应控制器Y0输出口电平 */
#define  COIL_Y1_ADDR       14
#define  COIL_Y2_ADDR       15
#define  COIL_Y3_ADDR       16
#define  COIL_Y4_ADDR       17
#define  COIL_Y5_ADDR       18
#define  COIL_Y6_ADDR       19
#define  COIL_Y7_ADDR       20
#define  COIL_MOTOR_X_DIR   21  //0标识向左，1标识向右
#define  COIL_MOTOR_Y_DIR   22  //0标识向左，1标识向右
#define  COIL_MOTOR_Z_DIR   23  //0标识向左，1标识向右
#define  COIL_MOTOR_X_S     24  //0标识电机停止,1标识电机在运行
#define  COIL_MOTOR_Y_S     25  //0标识电机停止,1标识电机在运行
#define  COIL_MOTOR_Z_S     26  //0标识电机停止,1标识电机在运行
#define  COIL_MOTOR_LEFT_X  30  //启动电机手动向左运行
#define  COIL_MOTOR_RIGHT_X 31  //启动电机手动向右运行
#define  COIL_MOTOR_LEFT_Y  32  //启动电机手动向左运行
#define  COIL_MOTOR_RIGHT_Y 33  //启动电机手动向右运行
#define  COIL_MOTOR_LEFT_Z  34  //启动电机手动向左运行
#define  COIL_MOTOR_RIGHT_Z 35  //启动电机手动向右运行
#define  COIL_MOTOR_CLEAR_X 36  //X轴位置清零
#define  COIL_MOTOR_CLEAR_Y 37  //Y轴位置清零
#define  COIL_MOTOR_CLEAR_Z 38  //Z轴位置清零
#define  COIL_SYS_PAUSE     39  //暂停
#define  COIL_SYS_CONTINUE  40  //继续运行
#define  COIL_SYS_RESET     41  //复位
#define  COIL_SAVE_DATA_XY  42  //保存XY坐标
#define  COIL_SAVE_COMPLETE_1  43 //保存点胶机有几个点,不规则保存
#define  COIL_SAVE_COMPLETE_2  44 //保存点胶机有几个点,规则数据保存
#define  COIL_SAVE_START_POINT 45 //保存点胶机参考位置信息
#define  COIL_SAVE_DATA_XYZ    46 //保存XYZ三个点坐标
#define  COIL_SAVE_DATA_XY_O   47 //保存XY与输出口状态
#define  COIL_SAVE_DATA_XYZ_O  48 //保存XYZ与输出口状态
#define  COIL_SET_STEP_MODE    50 //设置为单步工作模式
#define  COIL_RUN_STEP_CODE    51 //单步运行一行
#define  COIL_EARSE_DM_DATA    52 //将用户区的DM值全部清空，赋值为0
#define  COIL_MOTOR_CUSTOM     80 //用户自定义指令

/* 常用DM数据区表 */
#define  DM_MOTOR_ACC_X  1   //SHORT 设置X轴电机加速系数
#define  DM_MOTOR_ACC_Y  2   //SHORT
#define  DM_MOTOR_ACC_Z  3   //SHORT
#define  DM_MOTOR_DEC_X  4   //SHORT 设置X轴电机减速系数
#define  DM_MOTOR_DEC_Y  5   //SHORT
#define  DM_MOTOR_DEC_Z  6   //SHORT
#define  DM_MOTOR_MAX_SPD_X  7   //FLOAT   设置X轴电机运行最大速度
#define  DM_MOTOR_MAX_SPD_Y  9   //FLOAT
#define  DM_MOTOR_MAX_SPD_Z  11  //FLOAT
#define  DM_MOTOR_PITH_X 13  //FLOAT   设置X轴螺距
#define  DM_MOTOR_PITH_Y 15  //FLOAT
#define  DM_MOTOR_PITH_Z 17  //FLOAT
#define  DM_MOTOR_DIV_X  19  //SHORT   设置X轴驱动器系分数
#define  DM_MOTOR_DIV_Y  20  //SHORT
#define  DM_MOTOR_DIV_Z  21  //SHORT
#define  DM_MOTOR_POS_X  22  //FLOAT   设置X轴运行的目标地址
#define  DM_MOTOR_POS_Y  24  //FLOAT
#define  DM_MOTOR_POS_Z  26  //FLOAT
#define  DM_MOTOR_MOVE_X 28  //FLOAT   设置X轴运行相对位移，正负为方向
#define  DM_MOTOR_MOVE_Y 30  //FLOAT
#define  DM_MOTOR_MOVE_Z 32  //FLOAT
#define  DM_PROGRAM_NO   34  //SHORT
#define  DM_WORK_STATUS  35  //SHORT
#define  DM_USER_COUNTER 36  //SHORT
#define  DM_DATA_AD0     37  //SHORT   /* 读取外部AD的输入值 */
#define  DM_DATA_AD1     38  //SHORT
#define  DM_DATA_POS_CNT 39  //SHORT  保存点胶机有几个点
#define  DM_CODE_LINE    40  //SHORT  读取代码运行行号

//用户自定义数字
#define  DM_USER_PAR_0   41  //FLOAT
#define  DM_USER_PAR_1   43  //FLOAT
#define  DM_USER_PAR_2   45  //FLOAT
#define  DM_USER_PAR_3   47  //FLOAT
#define  DM_USER_PAR_4   49  //FLOAT
#define  DM_USER_PAR_5   51  //FLOAT
#define  DM_USER_PAR_6   53  //FLOAT
#define  DM_USER_PAR_7   55  //FLOAT
#define  DM_USER_PAR_8   57  //FLOAT
#define  DM_USER_PAR_9   59  //FLOAT
#define  DM_USER_PASSWD  61  //LONG
#define  DM_USE_TIMES    63  //SHORT
#define  DM_START_POINT_X 64 //FLOAT
#define  DM_START_POINT_Y 66 //FLOAT
#define  DM_VER_NUM       68  //SHORT
#define  DM_VER_DIST      69  //FLOAT
#define  DM_HER_NUM       71  //SHORT
#define  DM_HER_DIST      72  //FLOAT
#define  DM_ENCODER_SCALE_1 74 //FLOAT
#define  DM_ENCODER_SCALE_2 76 //FLOAT
#define  DM_AD_MIN_0        78 //FLOAT
#define  DM_AD_MAX_0        80 //FLOAT
#define  DM_AD_MIN_1        82 //FLOAT
#define  DM_AD_MAX_1        84 //FLOAT
#define  DM_AD_SCALE_0      86 //FLOAT 通过最小值与最大值转换后的值
#define  DM_AD_SCALE_1      88 //FLOAT 转换成实际参数的值转换后的值
#define  DM_ENCODE_1_POS    90 //FLOAT 编码器位置信息
#define  DM_ENCODE_2_POS    92 //FLOAT 编码器位置信息
#define  DM_MOTOR_MIN_SPD_X 94 //FLOAT X电机最小速度
#define  DM_MOTOR_MIN_SPD_Y 96 //FLOAT Y电机最小速度
#define  DM_MOTOR_MIN_SPD_Z 98 //FLOAT Z电机最小速度
#define  DM_USER_DATA       100 //USER DEFINE MEMORY 用户编程自定义参数
#define  DM_DIAN_JIAO_DATA  150 //点胶点临时存放位置

#endif /* __MODBUS_PROTL_H__ */

