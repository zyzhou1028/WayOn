/******************** (C) COPYRIGHT 2012-2010 ADHON ********************
* File Name          : modbus_protl.h
* Author             : WangJiangZhu
* Version            : V1.0.1
* Date               : 08/05/2012
* Description        : Modbus��׼Э�����
*******************************************************************************/

#ifndef __MODBUS_PROTL_H__
#define __MODBUS_PROTL_H__

/* �û�֧�ֵ�ָ�� */
#define  MODBUS_CODE_1      0x01  /* ��ȡһ���߼���Ȧ״̬ */
#define  MODBUS_CODE_2      0x02  /* ��ȡһ�鿪������״̬ */
#define  MODBUS_CODE_3      0x03  /* ��ѯ���ּĴ��� */
#define  MODBUS_CODE_4      0x04  /* ��ѯ����Ĵ��� */
#define  MODBUS_CODE_5      0x05  /* дλ���� */
#define  MODBUS_CODE_6      0x06  /* д���ֽ� */
#define  MODBUS_CODE_15     0x0F  /* дλ���� */
#define  MODBUS_CODE_16     0x10  /* д���ֽ� */

//Coil��غ궨��
#define  COIL_X0_ADDR       1   /* ��Ӧ������X0����ڵ�ƽ */
#define  COIL_X1_ADDR       2
#define  COIL_X2_ADDR       3
#define  COIL_X3_ADDR       4
#define  COIL_X4_ADDR       5
#define  COIL_X5_ADDR       6
#define  COIL_IN1_ADDR      7   /* ��Ӧ������IN1����ڵ�ƽ */
#define  COIL_IN2_ADDR      8
#define  COIL_IN3_ADDR      9
#define  COIL_IN4_ADDR      10
#define  COIL_IN5_ADDR      11
#define  COIL_IN6_ADDR      12
#define  COIL_Y0_ADDR       13   /* ��Ӧ������Y0����ڵ�ƽ */
#define  COIL_Y1_ADDR       14
#define  COIL_Y2_ADDR       15
#define  COIL_Y3_ADDR       16
#define  COIL_Y4_ADDR       17
#define  COIL_Y5_ADDR       18
#define  COIL_Y6_ADDR       19
#define  COIL_Y7_ADDR       20
#define  COIL_MOTOR_X_DIR   21  //0��ʶ����1��ʶ����
#define  COIL_MOTOR_Y_DIR   22  //0��ʶ����1��ʶ����
#define  COIL_MOTOR_Z_DIR   23  //0��ʶ����1��ʶ����
#define  COIL_MOTOR_X_S     24  //0��ʶ���ֹͣ,1��ʶ���������
#define  COIL_MOTOR_Y_S     25  //0��ʶ���ֹͣ,1��ʶ���������
#define  COIL_MOTOR_Z_S     26  //0��ʶ���ֹͣ,1��ʶ���������
#define  COIL_MOTOR_LEFT_X  30  //��������ֶ���������
#define  COIL_MOTOR_RIGHT_X 31  //��������ֶ���������
#define  COIL_MOTOR_LEFT_Y  32  //��������ֶ���������
#define  COIL_MOTOR_RIGHT_Y 33  //��������ֶ���������
#define  COIL_MOTOR_LEFT_Z  34  //��������ֶ���������
#define  COIL_MOTOR_RIGHT_Z 35  //��������ֶ���������
#define  COIL_MOTOR_CLEAR_X 36  //X��λ������
#define  COIL_MOTOR_CLEAR_Y 37  //Y��λ������
#define  COIL_MOTOR_CLEAR_Z 38  //Z��λ������
#define  COIL_SYS_PAUSE     39  //��ͣ
#define  COIL_SYS_CONTINUE  40  //��������
#define  COIL_SYS_RESET     41  //��λ
#define  COIL_SAVE_DATA_XY  42  //����XY����
#define  COIL_SAVE_COMPLETE_1  43 //����㽺���м�����,�����򱣴�
#define  COIL_SAVE_COMPLETE_2  44 //����㽺���м�����,�������ݱ���
#define  COIL_SAVE_START_POINT 45 //����㽺���ο�λ����Ϣ
#define  COIL_SAVE_DATA_XYZ    46 //����XYZ����������
#define  COIL_SAVE_DATA_XY_O   47 //����XY�������״̬
#define  COIL_SAVE_DATA_XYZ_O  48 //����XYZ�������״̬
#define  COIL_SET_STEP_MODE    50 //����Ϊ��������ģʽ
#define  COIL_RUN_STEP_CODE    51 //��������һ��
#define  COIL_EARSE_DM_DATA    52 //���û�����DMֵȫ����գ���ֵΪ0
#define  COIL_MOTOR_CUSTOM     80 //�û��Զ���ָ��

/* ����DM�������� */
#define  DM_MOTOR_ACC_X  1   //SHORT ����X��������ϵ��
#define  DM_MOTOR_ACC_Y  2   //SHORT
#define  DM_MOTOR_ACC_Z  3   //SHORT
#define  DM_MOTOR_DEC_X  4   //SHORT ����X��������ϵ��
#define  DM_MOTOR_DEC_Y  5   //SHORT
#define  DM_MOTOR_DEC_Z  6   //SHORT
#define  DM_MOTOR_MAX_SPD_X  7   //FLOAT   ����X������������ٶ�
#define  DM_MOTOR_MAX_SPD_Y  9   //FLOAT
#define  DM_MOTOR_MAX_SPD_Z  11  //FLOAT
#define  DM_MOTOR_PITH_X 13  //FLOAT   ����X���ݾ�
#define  DM_MOTOR_PITH_Y 15  //FLOAT
#define  DM_MOTOR_PITH_Z 17  //FLOAT
#define  DM_MOTOR_DIV_X  19  //SHORT   ����X��������ϵ����
#define  DM_MOTOR_DIV_Y  20  //SHORT
#define  DM_MOTOR_DIV_Z  21  //SHORT
#define  DM_MOTOR_POS_X  22  //FLOAT   ����X�����е�Ŀ���ַ
#define  DM_MOTOR_POS_Y  24  //FLOAT
#define  DM_MOTOR_POS_Z  26  //FLOAT
#define  DM_MOTOR_MOVE_X 28  //FLOAT   ����X���������λ�ƣ�����Ϊ����
#define  DM_MOTOR_MOVE_Y 30  //FLOAT
#define  DM_MOTOR_MOVE_Z 32  //FLOAT
#define  DM_PROGRAM_NO   34  //SHORT
#define  DM_WORK_STATUS  35  //SHORT
#define  DM_USER_COUNTER 36  //SHORT
#define  DM_DATA_AD0     37  //SHORT   /* ��ȡ�ⲿAD������ֵ */
#define  DM_DATA_AD1     38  //SHORT
#define  DM_DATA_POS_CNT 39  //SHORT  ����㽺���м�����
#define  DM_CODE_LINE    40  //SHORT  ��ȡ���������к�

//�û��Զ�������
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
#define  DM_AD_SCALE_0      86 //FLOAT ͨ����Сֵ�����ֵת�����ֵ
#define  DM_AD_SCALE_1      88 //FLOAT ת����ʵ�ʲ�����ֵת�����ֵ
#define  DM_ENCODE_1_POS    90 //FLOAT ������λ����Ϣ
#define  DM_ENCODE_2_POS    92 //FLOAT ������λ����Ϣ
#define  DM_MOTOR_MIN_SPD_X 94 //FLOAT X�����С�ٶ�
#define  DM_MOTOR_MIN_SPD_Y 96 //FLOAT Y�����С�ٶ�
#define  DM_MOTOR_MIN_SPD_Z 98 //FLOAT Z�����С�ٶ�
#define  DM_USER_DATA       100 //USER DEFINE MEMORY �û�����Զ������
#define  DM_DIAN_JIAO_DATA  150 //�㽺����ʱ���λ��

#endif /* __MODBUS_PROTL_H__ */

