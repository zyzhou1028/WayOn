#ifndef CONTROL_3DMACHINE_H
#define CONTROL_3DMACHINE_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>
#include "dataprocess.h"
#include "Modbus.h"

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
    bool relativeDisX();
    bool relativeDisY();
    bool relativeDisZ();

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
    bool getSubdivisionResponse();
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

    bool relativeDisXSet(float fDis);
    bool relativeDisYSet(float fDis);
    bool relativeDisZSet(float fDis);
    bool setMachineSpeedX(float fSpeed);
    bool setMachineSpeedY(float fSpeed);
    bool setMachineSpeedZ(float fSpeed);

    bool setMachinePitchX(float fPitch);
    bool setMachinePitchY(float fPitch);
    bool setMachinePitchZ(float fPitch);

    bool setMachineSubdivisionX(short iSubdivision);
    bool setMachineSubdivisionY(short iSubdivision);
    bool setMachineSubdivisionZ(short iSubdivision);

    bool setMachineAccX(short iAccX);
    bool setMachineAccY(short iAccY);
    bool setMachineAccZ(short iAccZ);

    bool setMachineDecX(short iDecX);
    bool setMachineDecY(short iDecY);
    bool setMachineDecZ(short iDecZ);

    bool setMachineStartSpeedX(float fStartS);
    bool setMachineStartSpeedY(float fStartS);
    bool setMachineStartSpeedZ(float fStartS);

    void setStartSpeedX();
    void setStartSpeedY();
    void setStartSpeedZ();

    bool runStep(unsigned int iStepIndex);
    bool runFile();

    dataprocess dataProcess;
    Modbus modbusPro;
};

#endif // CONTROL_3DMACHINE_H
