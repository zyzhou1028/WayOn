#ifndef CONTROL_3DMACHINE_H
#define CONTROL_3DMACHINE_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include "dataprocess.h"
#include "Modbus.h"

#define ELAPSED_TIMES       6
#define ELAPSED_INTERVAL 50

namespace Ui {
class Control_3DMachine;
}

class Control_3DMachine : public QMainWindow
{
    Q_OBJECT

public:
    explicit Control_3DMachine(QWidget *parent = 0);
    ~Control_3DMachine();

private slots:
    void on_pushButton_released();
    void on_action_triggered();
    bool openSerialPort();
    void closeSerialPort();
    int readData();
    int writeData(const char* data);

    void on_pushButton_4_released();

    void on_pushButton_5_released();

    void on_pushButton_3_released();

    void on_pushButton_12_released();

    void on_pushButton_15_released();

    void on_pushButton_13_released();

    void on_pushButton_8_released();

    void on_pushButton_11_released();

    void on_pushButton_6_released();

    void on_pushButton_14_released();

    void on_pushButton_7_released();

    void on_pushButton_10_released();

    void on_pushButton_16_released();

    void on_pushButton_17_released();

    void on_pushButton_18_released();

    void on_pushButton_19_toggled(bool checked);

    void on_pushButton_19_released();

    void on_pushButton_20_released();

    void on_pushButton_2_released();

    void on_pushButton_2_pressed();

private:
    Ui::Control_3DMachine *ui;
    QSerialPort *comModbus;

    bool moveMachine();
    bool moveXLeft();
    bool stopXLeft();
    bool moveXRight();
    bool stopXRight();
    bool moveYLeft();
    bool stopYLeft();
    bool moveYRight();
    bool stopYRight();
    bool moveZLeft();
    bool stopZLeft();
    bool moveZRight();
    bool stopZRight();
    bool clearMotorX();
    bool clearMotorY();
    bool clearMotorZ();
    bool dirXYZQuery();
    void setXYZDir(char szDir);
    bool relativeDisXQuery();
    bool relativeDisYQuery();
    bool relativeDisZQuery();
    bool absoluteDisXQuery();
    bool absoluteDisYQuery();
    bool absoluteDisZQuery();


    bool getMoveResponse();
    void getControlerParams();
    bool getParamsPitch();
    bool getParamsSubdivision();
    bool getParamsSpeed();
    bool getParamMotorAccX();
    bool getParamMotorAccY();
    bool getParamMotorAccZ();
    bool getParamMotorDecX();
    bool getParamMotorDecY();
    bool getParamMotorDecZ();
    bool getParamStartSpeedX();
    bool getParamStartSpeedY();
    bool getParamStartSpeedZ();
    bool getPitchResponse();
    void setPitchParams();
    bool getSubdivisionResp();
    void setSubdivisionParams();
    bool getSpeedResponse();
    bool getAccXResponse();
    bool getAccYResponse();
    bool getAccZResponse();
    bool getDecXResponse();
    bool getDecYResponse();
    bool getDecZResponse();
    bool getStartSpeedXResp();
    bool getStartSpeedYResp();
    bool getStartSpeedZResp();
    void setSpeedParams();
    void setMotorAccX();
    void setMotorAccY();
    void setMotorAccZ();
    void setMotorDecX();
    void setMotorDecY();
    void setMotorDecZ();

    bool sendQuery(const char* szCMD, unsigned int iLen);
    bool getResponse();
    bool testQuery();
    bool queryCoefficients();

    bool moveXRelative(float fDis);
    bool moveYRelative(float fDis);
    bool moveZRelative(float fDis);

    bool moveXAbsolute(float fDis);
    bool moveYAbsolute(float fDis);
    bool moveZAbsolute(float fDis);

    bool setMachineSpeedX(float fSpeed);
    bool setMachineSpeedY(float fSpeed);
    bool setMachineSpeedZ(float fSpeed);

    bool setMachinePitchX(float fPitch);
    bool setMachinePitchY(float fPitch);
    bool setMachinePitchZ(float fPitch);

    bool setMachineSubdivX(short iSubdiv);
    bool setMachineSubdivY(short iSubdiv);
    bool setMachineSubdivZ(short iSubdiv);

    bool setMachineAccX(short iAccX);
    bool setMachineAccY(short iAccY);
    bool setMachineAccZ(short iAccZ);

    bool setMachineDecX(short iDecX);
    bool setMachineDecY(short iDecY);
    bool setMachineDecZ(short iDecZ);

    bool setStartSpeedX(float fStartS);
    bool setStartSpeedY(float fStartS);
    bool setStartSpeedZ(float fStartS);

    void recordStartSpeedX();
    void recordStartSpeedY();
    void recordStartSpeedZ();

    bool runFileShifting();
    bool set3AxleStatus(int iIndex);
    bool setAxleXStat(int iIndex);
    bool setAxleYStat(int iIndex);
    bool setAxleZStat(int iIndex);

    bool runRelativeStep(unsigned int iStepIndex);
    bool runRelativeFile();

    bool runAbsoluteStep(unsigned int iStepIndex);
    bool runAbsoluteFile();

    dataprocess dataProc;
    Modbus modbusPro;
    bool m_bIfResp;
    QTime m_timeTag;
};

#endif // CONTROL_3DMACHINE_H
