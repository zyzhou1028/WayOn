#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include "aboutform.h"
#include "ui_aboutform.h"
#include "control_3dmachine.h"
#include "ui_control_3dmachine.h"
#include "modbus_protl.h"

#include<unistd.h>

Control_3DMachine::Control_3DMachine(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Control_3DMachine)
{
  qDebug("System Start:%s!", m_timeTag.currentTime().toString().toStdString().c_str());
  m_bIfResp = true;
  ui->setupUi(this);
  comModbus = new QSerialPort(this);
  connect(comModbus, SIGNAL(readyRead()), this, SLOT(readData()));
  if(openSerialPort()){
      //getControlerParams();
      QTime time;
      time.start();
      clearMotorX();
      while(time.elapsed() < 300)
        {
          QCoreApplication::processEvents();
        }
      time.restart();
      clearMotorY();
      while(time.elapsed() < 300)
        {
          QCoreApplication::processEvents();
        }
      time.restart();
      clearMotorZ();
      while(time.elapsed() < 300)
        {
          QCoreApplication::processEvents();
        }
      modbusPro.fAbsPosX = 0;
      modbusPro.fAbsPosY = 0;
      modbusPro.fAbsPosZ = 0;
    }
}

Control_3DMachine::~Control_3DMachine()
{
  closeSerialPort();
  delete ui;
}

void Control_3DMachine::on_pushButton_released()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open control file..."),
                                                  "/home", tr("Machine control File(*.3dm)"));

  if(!(fileName.isNull())){
      qDebug("Open control file:%s!", fileName.toStdString().c_str());

      QFile fMachine (fileName);
      if( fMachine.open(QIODevice::ReadOnly)){
          dataProc.dataList.clear();
          dataProc.shiftList.clear();
          QTextStream tStream(&fMachine);
          QString strTmp;
          strTmp = tStream.readLine();
          qDebug("Check file head:%s!", strTmp.toStdString().c_str());
          if( !strTmp.compare("#wayon 3D machine control data")){
              strTmp=tStream.readLine();
              if( strTmp.contains("TimeInter="))
                {
                  dataProc.iTimeInterval = strTmp.right(strTmp.length() - strlen("TimeInter=")).toInt();
                  qDebug("TimeInterval:%s!iTimeInterval=%d!", strTmp.right(strTmp.length() - strlen("TimeInter=")).toStdString().c_str(), dataProc.iTimeInterval);
                  if(dataProc.parse(tStream) > 0){
                      ui->lineEdit->setText(fileName);
                      ui->label_2->setText("文件已加载");
                    }
                }
            }else{
              qDebug("Cann't find 3dm file head!");
            }
          fMachine.close();
        }
    }
}

void Control_3DMachine::on_action_triggered()
{
  aboutForm *pAboutForm = new aboutForm;
  pAboutForm->setAttribute(Qt::WA_DeleteOnClose);
  pAboutForm->setWindowModality(Qt::ApplicationModal);
  pAboutForm->show();
}

bool Control_3DMachine::openSerialPort()
{
  QString strCom = "/dev/ttyUSB0";
  QList<QSerialPortInfo> serialList = QSerialPortInfo::availablePorts();
  for(int i=0; i<serialList.size(); i++)
    {
      qDebug("Available Serial[%d]:%s!", i, serialList[i].portName().toStdString().c_str());
      if((serialList[i].portName().contains("ttyUSB"))||(serialList[i].portName().contains("COM")))
        {
          strCom = serialList[i].portName();
          break;
        }
    }
  comModbus->setPortName(strCom);
  qDebug("Serial name:%s!", strCom.toStdString().c_str());
  if (comModbus->open(QIODevice::ReadWrite)) {
      qDebug("Serial openned!");
      if (comModbus->setBaudRate(QSerialPort::Baud9600)
          && comModbus->setDataBits(QSerialPort::Data8)
          && comModbus->setParity(QSerialPort::NoParity)
          && comModbus->setStopBits(QSerialPort::OneStop)
          && comModbus->setFlowControl(QSerialPort::NoFlowControl)) {
          ui->label->setText("设备已连接");
          qDebug("Serial params set ok!");
          return true;
        } else {
          comModbus->close();
          ui->label->setText("设置参数失败");
          qDebug("Serial params set fail!");
        }
    } else {
      ui->label->setText("设备连接失败");
      qDebug("Serial open fail!");
    }
  return false;
}

void Control_3DMachine::closeSerialPort()
{
  if(comModbus->isOpen()){
      comModbus->close();
    }
}

int Control_3DMachine::writeData(const char* data)
{
  return  comModbus->write(data, sizeof(data));
}

int Control_3DMachine::readData()
{
  char szBuf[65] = "";
  int iComDataLen = comModbus->read(szBuf, 64);
  std::string strTmp;
  strTmp.assign(szBuf, iComDataLen);
  modbusPro.m_strReadBuf.append(strTmp);
  //qDebug("Com data Read %d/%d Bytes!", iComDataLen, modbusPro.m_strReadBuf.length());
  return iComDataLen;
}

bool Control_3DMachine::queryCoefficients()
{
  modbusPro.m_strReadBuf.clear();
  const char szMode[] = {0xCC, 0x03, 0x00, 0x01, 0x00, 0x03, 0x44, 0x16};
  if (comModbus->write(szMode, 10))
    {
      //qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed() < ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getSpeedResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::sendQuery(const char* szCMD, unsigned int iLen)
{
  char szCmd[iLen + 3];
  modbusPro.buildPacket(szCmd, szCMD, iLen);
  //    qDebug("Try Cmd szCmd:");
  //    for(int i=0; i<iLen+2;i++){
  //        qDebug("%02x", (char)szCmd[i]);
  //    }
  int iCount = 0;
  if ((iCount = comModbus->write(szCmd, iLen+2)))
    {
      //qDebug("Write cmd success Len:%d -- %d!", iLen, iCount);
      return true;
    }
  qDebug("Write cmd fail Len:%d -- %d!", iLen, iCount);
  return false;
}

bool Control_3DMachine::getResponse()
{
  if(modbusPro.checkResponse())
    {
      return true;
    }else
    {
      qDebug("Something wrong with response data!");
    }
  return false;
}

bool Control_3DMachine::testQuery()
{
  //    bool bFlag = sendQuery(0XCC, MODBUS_CODE_3, (short)DM_USER_DATA, (short)2);
  //    QElapsedTimer t;
  //    t.start();
  //    while(t.elapsed()<ELAPSED_INTERVAL)
  //        QCoreApplication::processEvents();
  //    return bFlag;
}

void Control_3DMachine::getControlerParams()
{
  if(getParamsSpeed()){
      qDebug("Get Speed Success!");
    }else{
      qDebug("Get Speed Fail!");
    }

  if(getParamsPitch()){
      qDebug("Get Pitch Success!");
    }else{
      qDebug("Get Pitch Fail!");
    }

  if(getParamsSubdivision()){
      qDebug("Get Subdivision Success!");
    }else{
      qDebug("Get Subdivision Fail!");
    }
}

bool Control_3DMachine::getParamsPitch()
{
  modbusPro.m_strReadBuf.clear();
  //    const char szMode[] = {0xCC, 0x03, 0x00, 0x0D, 0x00, 0x06, 0x44, 0x16};
  const char szPitchQuery[] = {0xCC, 0x03, 0x00, 0x0D, 0x00, 0x06};
  if (sendQuery(szPitchQuery, 6))
    {
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getPitchResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getPitchResponse()
{
  if(modbusPro.checkResponse())
    {
      setPitchParams();
      return true;
    }else
    {
      qDebug("Something wrong with Pitch response data!");
    }
  return false;
}

void Control_3DMachine::setPitchParams()
{
  unsigned char szTmp[5];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iPitchX = modbusPro.getFloat(szTmp);

  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 7,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iPitchY = modbusPro.getFloat(szTmp);
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 11,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iPitchZ = modbusPro.getFloat(szTmp);
  qDebug("Pitchs:%.02f %.02f %.02f!", modbusPro.iPitchX, modbusPro.iPitchY, modbusPro.iPitchZ);
}

bool Control_3DMachine::getParamsSubdivision()
{
  modbusPro.m_strReadBuf.clear();
  //    const char szMode[] = {0xCC, 0x03, 0x00, 0x13, 0x00, 0x03, 0xE4, 0x13};
  const char szSubdivQuery[] = {0xCC, 0x03, 0x00, 0x13, 0x00, 0x03};
  if (sendQuery(szSubdivQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getSubdivisionResp())
            {
              qDebug("GetResponse OK!");
              unsigned char szTmp[3];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
              for(int i=0; i<2; i++){
                  qDebug("%02x", szTmp[i]);
                }
              modbusPro.getReg(szTmp);
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 5,  2);
              for(int i=0; i<2; i++){
                  qDebug("%02x", szTmp[i]);
                }
              modbusPro.getReg(szTmp);
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 7,  2);
              for(int i=0; i<2; i++){
                  qDebug("%02x", szTmp[i]);
                }
              modbusPro.getReg(szTmp);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getSubdivisionResp()
{
  if(modbusPro.checkResponse())
    {
      setSubdivisionParams();
      return true;
    }else
    {
      qDebug("Something wrong with Subdivision response data!");
    }
  return false;
}

void Control_3DMachine::setSubdivisionParams()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iSubdivX = modbusPro.getReg(szTmp);

  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 5,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iSubdivY = modbusPro.getReg(szTmp);
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 7,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iSubdivZ = modbusPro.getReg(szTmp);

}

bool Control_3DMachine::getParamsSpeed()
{
  modbusPro.m_strReadBuf.clear();
  //    const char szMode[] = {0xCC, 0x03, 0x00, 0x01, 0x00, 0x03, 0x44, 0x16};
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x07, 0x00, 0x06};
  if (sendQuery(szCoeQuery, 6))
    {
      //qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getSpeedResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamMotorAccX()
{
  modbusPro.m_strReadBuf.clear();
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x01, 0x00, 0x01};
  if (sendQuery(szCoeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getAccXResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamMotorAccY()
{
  modbusPro.m_strReadBuf.clear();
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x02, 0x00, 0x01};
  if (sendQuery(szCoeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getAccYResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamMotorAccZ()
{
  modbusPro.m_strReadBuf.clear();
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x03, 0x00, 0x01};
  if (sendQuery(szCoeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getAccZResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamMotorDecX()
{
  modbusPro.m_strReadBuf.clear();
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x04, 0x00, 0x02};
  if (sendQuery(szCoeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getDecXResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamMotorDecY()
{
  modbusPro.m_strReadBuf.clear();
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x05, 0x00, 0x02};
  if (sendQuery(szCoeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getDecYResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamMotorDecZ()
{
  modbusPro.m_strReadBuf.clear();
  const char szCoeQuery[] = {0xCC, 0x03, 0x00, 0x06, 0x00, 0x02};
  if (sendQuery(szCoeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getDecZResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamStartSpeedX()
{
  modbusPro.m_strReadBuf.clear();
  const char szStartSpeed[] = {0xCC, 0x03, 0x00, 0x5E, 0x00, 0x02};
  if (sendQuery(szStartSpeed, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getStartSpeedXResp())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamStartSpeedY()
{
  modbusPro.m_strReadBuf.clear();
  const char szStartSpeed[] = {0xCC, 0x03, 0x00, 0x60, 0x00, 0x02};
  if (sendQuery(szStartSpeed, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getStartSpeedYResp())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getParamStartSpeedZ()
{
  modbusPro.m_strReadBuf.clear();
  const char szStartSpeed[] = {0xCC, 0x03, 0x00, 0x62, 0x00, 0x02};
  if (sendQuery(szStartSpeed, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getStartSpeedZResp())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setMachineSpeedX(float fSpeed)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x07, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fSpeed);
  if (sendQuery(szRelativeQuery, 11))
    {
      //      qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse(8))
            {
              //qDebug("GetResponse Set Speed X OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Speed X Set Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::setMachineSpeedY(float fSpeed)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x09, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fSpeed);
  if (sendQuery(szRelativeQuery, 11))
    {
      //      qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse(8))
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Speed Y Set Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::setMachineSpeedZ(float fSpeed)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x0B, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fSpeed);
  if (sendQuery(szRelativeQuery, 11))
    {
      //      qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse(8))
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Speed Z Set Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::setMachinePitchX(float fPitch)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x0D, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fPitch);
  if (sendQuery(szRelativeQuery, 11))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setMachinePitchY(float fPitch)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x0F, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fPitch);
  if (sendQuery(szRelativeQuery, 11))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setMachinePitchZ(float fPitch)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x11, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fPitch);
  if (sendQuery(szRelativeQuery, 11))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getSpeedResponse()
{
  if(modbusPro.checkResponse())
    {
      setSpeedParams();
      return true;
    }else
    {
      qDebug("Something wrong with Speed response data!");
    }
  return false;
}

bool Control_3DMachine::getAccXResponse()
{
  if(modbusPro.checkResponse())
    {
      setMotorAccX();
      return true;
    }else
    {
      qDebug("Something wrong with ACC X response data!");
    }
  return false;
}

bool Control_3DMachine::getAccYResponse()
{
  if(modbusPro.checkResponse())
    {
      setMotorAccY();
      return true;
    }else
    {
      qDebug("Something wrong with ACC Y response data!");
    }
  return false;
}

bool Control_3DMachine::getAccZResponse()
{
  if(modbusPro.checkResponse())
    {
      setMotorAccZ();
      return true;
    }else
    {
      qDebug("Something wrong with ACC Z response data!");
    }
  return false;
}

bool Control_3DMachine::getDecXResponse()
{
  if(modbusPro.checkResponse())
    {
      setMotorDecX();
      return true;
    }else
    {
      qDebug("Something wrong with Dec X response data!");
    }
  return false;
}

bool Control_3DMachine::getDecYResponse()
{
  if(modbusPro.checkResponse())
    {
      setMotorDecY();
      return true;
    }else
    {
      qDebug("Something wrong with Dec Y response data!");
    }
  return false;
}

bool Control_3DMachine::getDecZResponse()
{
  if(modbusPro.checkResponse())
    {
      setMotorDecZ();
      return true;
    }else
    {
      qDebug("Something wrong with Dec Z response data!");
    }
  return false;
}

bool Control_3DMachine::getStartSpeedXResp()
{
  if(modbusPro.checkResponse())
    {
      recordStartSpeedX();
      return true;
    }else
    {
      qDebug("Something wrong with StartSpeed X response data!");
    }
  return false;
}

bool Control_3DMachine::getStartSpeedYResp()
{
  if(modbusPro.checkResponse())
    {
      recordStartSpeedY();
      return true;
    }else
    {
      qDebug("Something wrong with StartSpeed Y response data!");
    }
  return false;
}

bool Control_3DMachine::getStartSpeedZResp()
{
  if(modbusPro.checkResponse())
    {
      recordStartSpeedZ();
      return true;
    }else
    {
      qDebug("Something wrong with StartSpeed Z response data!");
    }
  return false;
}

void Control_3DMachine::setSpeedParams()
{
  unsigned char szTmp[5];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iSpeedX = modbusPro.getFloat(szTmp);
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 7,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iSpeedY =modbusPro.getFloat(szTmp);
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 11,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iSpeedZ =modbusPro.getFloat(szTmp);
}

void Control_3DMachine::setMotorAccX()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iMotorAccX = modbusPro.getReg(szTmp);
}

void Control_3DMachine::setMotorAccY()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iMotorAccY = modbusPro.getReg(szTmp);
}

void Control_3DMachine::setMotorAccZ()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iMotorAccZ = modbusPro.getReg(szTmp);
}

void Control_3DMachine::setMotorDecX()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iMotorDecX = modbusPro.getReg(szTmp);
}

void Control_3DMachine::setMotorDecY()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iMotorDecY = modbusPro.getReg(szTmp);
}

void Control_3DMachine::setMotorDecZ()
{
  unsigned char szTmp[3];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  2);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.iMotorDecZ = modbusPro.getReg(szTmp);
}

void Control_3DMachine::recordStartSpeedX()
{
  unsigned char szTmp[5];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.fStartSpeedX = modbusPro.getFloat(szTmp);
}

void Control_3DMachine::recordStartSpeedY()
{
  unsigned char szTmp[5];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.fStartSpeedY = modbusPro.getFloat(szTmp);
}

void Control_3DMachine::recordStartSpeedZ()
{
  unsigned char szTmp[5];
  memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
  //    for(int i=0; i<4; i++){
  //        qDebug("%02x", szTmp[i]);
  //    }
  modbusPro.fStartSpeedZ = modbusPro.getFloat(szTmp);
}

void Control_3DMachine::on_pushButton_3_released()
{
  closeSerialPort();
  openSerialPort();
}

bool Control_3DMachine::moveMachine()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x20, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (getMoveResponse())
            {
              qDebug("GetResponse OK!");
              break;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }

  qDebug("Start:%s!", QTime::currentTime().toString().toStdString().c_str());
  QElapsedTimer tt;
  tt.start();
  while(tt.elapsed() < 200){
      QCoreApplication::processEvents();
    }
  qDebug("Stop:%s!", QTime::currentTime().toString().toStdString().c_str());

  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x20, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::getMoveResponse()
{
  if(modbusPro.checkResponse())
    {
      return true;
    }else
    {
      qDebug("Something wrong with Move cmd response data!");
    }
  return false;
}

bool Control_3DMachine::moveXLeft()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x1E, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::stopXLeft()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x1E, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::moveXRight()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x1F, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::stopXRight()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x1F, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::moveYLeft()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x20, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::stopYLeft()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x20, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::moveYRight()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x21, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::stopYRight()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x21, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::moveZLeft()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x22, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::stopZLeft()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x22, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::moveZRight()
{
  modbusPro.m_strReadBuf.clear();
  const char szMoveQuery[] = {0xCC, 0x05, 0x00, 0x23, 0xFF, 0x00};
  if (sendQuery(szMoveQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::stopZRight()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x23, 0x00, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::clearMotorX()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x24, 0xFF, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else
            {
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::clearMotorY()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x25, 0xFF, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::clearMotorZ()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x05, 0x00, 0x26, 0xFF, 0x00};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::dirXYZQuery()
{
  modbusPro.m_strReadBuf.clear();
  const char szStopQuery[] = {0xCC, 0x01, 0x00, 0x15, 0x00, 0x03};
  if (sendQuery(szStopQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES)
        {
          t.start();
          while(t.elapsed() < ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse XYZ Dir OK!Value:%02x!", modbusPro.m_strReadBuf.c_str()[3]);
              setXYZDir(modbusPro.m_strReadBuf.c_str()[3]);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

void Control_3DMachine::setXYZDir(char szDir)
{
  if((szDir & 0x01) > 0){
      modbusPro.bDirX = true;
    }else{
      modbusPro.bDirX = false;
    }
  if((szDir & 0x02) > 0){
      modbusPro.bDirY = true;
    }else{
      modbusPro.bDirY = false;
    }
  if((szDir & 0x04) > 0){
      modbusPro.bDirZ = true;
    }else{
      modbusPro.bDirZ = false;
    }
}

void Control_3DMachine::on_pushButton_12_released()
{

}

void Control_3DMachine::on_pushButton_4_released()
{
  //    moveMachine();
  moveXLeft();
}

void Control_3DMachine::on_pushButton_15_released()
{
  stopXLeft();
}

void Control_3DMachine::on_pushButton_5_released()
{
  //relativeDisY();
  moveYLeft();
}

void Control_3DMachine::on_pushButton_13_released()
{
  stopYLeft();
}

void Control_3DMachine::on_pushButton_8_released()
{
  moveYRight();
}

void Control_3DMachine::on_pushButton_11_released()
{
  stopYRight();
}

void Control_3DMachine::on_pushButton_6_released()
{
  moveZLeft();
}

void Control_3DMachine::on_pushButton_14_released()
{
  stopZLeft();
}

void Control_3DMachine::on_pushButton_7_released()
{
  moveZRight();
}

void Control_3DMachine::on_pushButton_10_released()
{
  stopZRight();
}

bool Control_3DMachine::absoluteDisXQuery()
{
  modbusPro.m_strReadBuf.clear();
  const char szCmd[] = {0xCC, 0x03, 0x00, 0x16, 0x00, 0x02};
  if (sendQuery(szCmd, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse POS_X OK!");
              unsigned char szTmp[5];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
              //              for(int i=0; i<4; i++){
              //                  qDebug("%02x", szTmp[i]);
              //                }
              modbusPro.fAbsPosX = modbusPro.getFloat(szTmp);
              qDebug("POS_X:%f!", modbusPro.fAbsPosX);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::absoluteDisYQuery()
{
  modbusPro.m_strReadBuf.clear();
  const char szCmd[] = {0xCC, 0x03, 0x00, 0x18, 0x00, 0x02};
  if (sendQuery(szCmd, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              unsigned char szTmp[5];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
              //              for(int i=0; i<4; i++){
              //                  qDebug("%02x", szTmp[i]);
              //                }
              modbusPro.fAbsPosY = modbusPro.getFloat(szTmp);
              qDebug("POS_Y:%f!", modbusPro.fAbsPosY);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::absoluteDisZQuery()
{
  modbusPro.m_strReadBuf.clear();
  const char szCmd[] = {0xCC, 0x03, 0x00, 0x1A, 0x00, 0x02};
  if (sendQuery(szCmd, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse POS_Z OK!");
              unsigned char szTmp[5];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
              //              for(int i=0; i<4; i++){
              //                  qDebug("%02x", szTmp[i]);
              //                }
              modbusPro.fAbsPosZ = modbusPro.getFloat(szTmp);
              qDebug("POS_Z:%f!", modbusPro.fAbsPosZ);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::relativeDisXQuery()
{
  modbusPro.m_strReadBuf.clear();
  // CC 03 00 09 00 02 04 14
  // CC 10 00 18 00 02 04 00 00 41 20 05 22
  //const char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x1C, 0x00, 0x02, 0x04, 0x00, 0x00, 0x41, 0x20};
  const char szRelativeQuery[] = {0xCC, 0x03, 0x00, 0x1C, 0x00, 0x02};
  if (sendQuery(szRelativeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              unsigned char szTmp[5];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
              //    for(int i=0; i<4; i++){
              //        qDebug("%02x", szTmp[i]);
              //    }
              modbusPro.getFloat(szTmp);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::relativeDisYQuery()
{
  modbusPro.m_strReadBuf.clear();
  const char szRelativeQuery[] = {0xCC, 0x03, 0x00, 0x1E, 0x00, 0x02};
  if (sendQuery(szRelativeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              unsigned char szTmp[5];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
              //    for(int i=0; i<4; i++){
              //        qDebug("%02x", szTmp[i]);
              //    }
              modbusPro.getFloat(szTmp);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::moveXRelative(float fDis)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x1C, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fDis);
  if (sendQuery(szRelativeQuery, 11))
    {
      //qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES)
        {
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL)
            {
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Move X Relative Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::moveYRelative(float fDis)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x1E, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fDis);
  if (sendQuery(szRelativeQuery, 11))
    {
      //qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Move Y Relative Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::moveZRelative(float fDis)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x20, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fDis);
  if (sendQuery(szRelativeQuery, 11))
    {
      //qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Move Z Relative Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::moveXAbsolute(float fDis)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x16, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fDis);
  if (sendQuery(szRelativeQuery, 11))
    {
      //qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Move X Absolute Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::moveYAbsolute(float fDis)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x18, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fDis);
  if (sendQuery(szRelativeQuery, 11))
    {
      //qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Move Y Absolute Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::moveZAbsolute(float fDis)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x1A, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fDis);
  if (sendQuery(szRelativeQuery, 11))
    {
      //qDebug("Write Query OK!");
      if(!m_bIfResp){
          return true;
        }
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }else{
      qDebug("Write Move Z Absolute Cmd Fail!");
    }
  return false;
}

bool Control_3DMachine::relativeDisZQuery()
{
  modbusPro.m_strReadBuf.clear();
  // CC 03 00 09 00 02 04 14
  // CC 10 00 18 00 02 04 00 00 41 20 05 22
  //const char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x20, 0x00, 0x02, 0x04, 0x00, 0x00, 0x41, 0x20};
  const char szRelativeQuery[] = {0xCC, 0x03, 0x00, 0x20, 0x00, 0x02};
  if (sendQuery(szRelativeQuery, 6))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              unsigned char szTmp[5];
              memcpy(szTmp, modbusPro.m_strReadBuf.c_str() + 3,  4);
              //    for(int i=0; i<4; i++){
              //        qDebug("%02x", szTmp[i]);
              //    }
              modbusPro.getFloat(szTmp);
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

void Control_3DMachine::on_pushButton_16_released()
{
  if(!(ui->lineEdit_2->text().isEmpty()))
    {
      moveXRelative(ui->lineEdit_2->text().toFloat());
      //      moveXAbsolute(ui->lineEdit_2->text().toFloat());
    }
}

void Control_3DMachine::on_pushButton_17_released()
{
  if(!(ui->lineEdit_3->text().isEmpty()))
    {
      moveYRelative(ui->lineEdit_3->text().toFloat());
    }
}

void Control_3DMachine::on_pushButton_18_released()
{
  if(!(ui->lineEdit_4->text().isEmpty()))
    {
      moveZRelative(ui->lineEdit_4->text().toFloat());
      //moveZAbsolute(ui->lineEdit_4->text().toFloat());
    }
}

bool Control_3DMachine::setMachineSubdivX(short iSubdivision)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x13, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iSubdivision);
  if (sendQuery(szRelativeQuery, 9))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setMachineSubdivY(short iSubdivision)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x14, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iSubdivision);
  if (sendQuery(szRelativeQuery, 9))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setMachineSubdivZ(short iSubdivision)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x15, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iSubdivision);
  if (sendQuery(szRelativeQuery, 9))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setMachineAccX(short iAccX)
{
  qDebug("AccX value to set:%d!", iAccX);
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iAccX);
  if (sendQuery(szRelativeQuery, 9))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //                qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  qDebug("Write Query fail!");
  return false;
}

bool Control_3DMachine::setMachineAccY(short iAccY)
{
  qDebug("AccY value to set:%d!", iAccY);
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x02, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iAccY);
  if (sendQuery(szRelativeQuery, 9))
    {
      //        qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //                qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  qDebug("Write Query fail!");
  return false;
}

bool Control_3DMachine::setMachineAccZ(short iAccZ)
{
  qDebug("AccZ value to set:%d!", iAccZ);
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x03, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iAccZ);
  if (sendQuery(szRelativeQuery, 9))
    {
      //        qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //                qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  qDebug("Write Query fail!");
  return false;
}

bool Control_3DMachine::setMachineDecX(short iDecX)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x04, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iDecX);
  if (sendQuery(szRelativeQuery, 9))
    {
      //        qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //                qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  qDebug("Write Query fail!");
  return false;
}

bool Control_3DMachine::setMachineDecY(short iDecY)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x05, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iDecY);
  if (sendQuery(szRelativeQuery, 9))
    {
      //        qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //                qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  qDebug("Write Query fail!");
  return false;
}

bool Control_3DMachine::setMachineDecZ(short iDecZ)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x06, 0x00, 0x01, 0x02, 0x00, 0x00};
  modbusPro.setReg((unsigned char*)szRelativeQuery + 7, iDecZ);
  if (sendQuery(szRelativeQuery, 9))
    {
      //        qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              //                qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  qDebug("Write Query fail!");
  return false;
}

bool Control_3DMachine::setStartSpeedX(float fStartS)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x5E, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fStartS);
  if (sendQuery(szRelativeQuery, 11))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setStartSpeedY(float fStartS)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x60, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fStartS);
  if (sendQuery(szRelativeQuery, 11))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

bool Control_3DMachine::setStartSpeedZ(float fStartS)
{
  modbusPro.m_strReadBuf.clear();
  char szRelativeQuery[] = {0xCC, 0x10, 0x00, 0x62, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00};
  modbusPro.setFloat((unsigned char*)szRelativeQuery + 7, fStartS);
  if (sendQuery(szRelativeQuery, 11))
    {
      qDebug("Write Query OK!");
      QElapsedTimer t;
      unsigned int iCounter = 0;
      while(iCounter < ELAPSED_TIMES){
          t.start();
          while(t.elapsed()<ELAPSED_INTERVAL){
              QCoreApplication::processEvents();
            }
          if (modbusPro.checkResponse())
            {
              qDebug("GetResponse OK!");
              return true;
            }else{
              qDebug("GetResponse fail!iCounter=%d!", iCounter);
            }
          iCounter++;
        }
    }
  return false;
}

void Control_3DMachine::on_pushButton_19_toggled(bool checked)
{

}

void Control_3DMachine::on_pushButton_19_released()
{
  qDebug("Choice:%d %d!", ui->comboBox->currentIndex(), ui->comboBox_2->currentIndex());
  QString strTmp("none");
  switch(ui->comboBox->currentIndex())
    {
    case 0:
      if(getParamsSpeed())
        {
          switch(ui->comboBox_2->currentIndex()){
            case 0:
              strTmp = QString::number(modbusPro.iSpeedX, 'f', 2);
              break;
            case 1:
              strTmp = QString::number(modbusPro.iSpeedY, 'f', 2);
              break;
            case 2:
              strTmp = QString::number(modbusPro.iSpeedZ, 'f', 2);
              break;
            default:
              qDebug("No this Index!");
              break;
            }
        }
      break;
    case 1:
      if(getParamsSubdivision())
        {
          switch(ui->comboBox_2->currentIndex()){
            case 0:
              strTmp = QString::number(modbusPro.iSubdivX);
              break;
            case 1:
              strTmp = QString::number(modbusPro.iSubdivY);
              break;
            case 2:
              strTmp = QString::number(modbusPro.iSubdivZ);
              break;
            default:
              qDebug("No this Index!");
              break;
            }
        }
      break;
    case 2:
      if(getParamsPitch())
        {
          switch(ui->comboBox_2->currentIndex()){
            case 0:
              qDebug("Show PitchX:%.02f!", modbusPro.iPitchX);
              strTmp = QString::number(modbusPro.iPitchX, 'f', 2);
              break;
            case 1:
              qDebug("Show PitchY:%.02f!", modbusPro.iPitchY);
              strTmp = QString::number(modbusPro.iPitchY, 'f', 2);
              break;
            case 2:
              qDebug("Show PitchZ:%.02f!", modbusPro.iPitchZ);
              strTmp = QString::number(modbusPro.iPitchZ, 'f', 2);
              break;
            default:
              qDebug("No this Index!");
              break;
            }
        }
      break;
    case 3:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          if(getParamMotorAccX())
            {
              qDebug("Show AccX:%d!", modbusPro.iMotorAccX);
              strTmp = QString::number(modbusPro.iMotorAccX);
            }
          break;
        case 1:
          if(getParamMotorAccY())
            {
              qDebug("Show AccY:%d!", modbusPro.iMotorAccY);
              strTmp = QString::number(modbusPro.iMotorAccY);
            }
          break;
        case 2:
          if(getParamMotorAccZ())
            {
              qDebug("Show AccZ:%d!", modbusPro.iMotorAccZ);
              strTmp = QString::number(modbusPro.iMotorAccZ);
            }
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    case 4:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          if(getParamMotorDecX())
            {
              qDebug("Show DecX:%d!", modbusPro.iMotorDecX);
              strTmp = QString::number(modbusPro.iMotorDecX);
            }
          break;
        case 1:
          if(getParamMotorDecY())
            {
              qDebug("Show DecY:%d!", modbusPro.iMotorDecY);
              strTmp = QString::number(modbusPro.iMotorDecY);
            }
          break;
        case 2:
          if(getParamMotorDecZ())
            {
              qDebug("Show DecZ:%d!", modbusPro.iMotorDecZ);
              strTmp = QString::number(modbusPro.iMotorDecZ);
            }
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    case 5:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          if(getParamStartSpeedX())
            {
              qDebug("Show DecX:%f!", modbusPro.fStartSpeedX);
              strTmp = QString::number(modbusPro.fStartSpeedX);
            }
          break;
        case 1:
          if(getParamStartSpeedY())
            {
              qDebug("Show DecY:%f!", modbusPro.fStartSpeedY);
              strTmp = QString::number(modbusPro.fStartSpeedY);
            }
          break;
        case 2:
          if(getParamStartSpeedZ())
            {
              qDebug("Show DecZ:%f!", modbusPro.fStartSpeedZ);
              strTmp = QString::number(modbusPro.fStartSpeedZ);
            }
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    default:
      qDebug("No this Index!");
      break;
    }
  ui->lineEdit_5->setText(strTmp);
}

void Control_3DMachine::on_pushButton_20_released()
{
  qDebug("Choice:%d %d!", ui->comboBox->currentIndex(), ui->comboBox_2->currentIndex());
  if(ui->lineEdit_5->text().isEmpty())
    {
      qDebug("Set Value is empty!");
      return;
    }
  switch(ui->comboBox->currentIndex())
    {
    case 0:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          qDebug("SpeedX want to set:%.02f!", ui->lineEdit_5->text().toFloat());
          setMachineSpeedX(ui->lineEdit_5->text().toFloat());
          break;
        case 1:
          setMachineSpeedY(ui->lineEdit_5->text().toFloat());
          break;
        case 2:
          setMachineSpeedZ(ui->lineEdit_5->text().toFloat());
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    case 1:
      switch(ui->comboBox_2->currentIndex()){
        case 0:
          setMachineSubdivX(ui->lineEdit_5->text().toInt());
          break;
        case 1:
          setMachineSubdivY(ui->lineEdit_5->text().toInt());
          break;
        case 2:
          setMachineSubdivZ(ui->lineEdit_5->text().toInt());
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    case 2:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          qDebug("PitchX want to set:%.02f!", ui->lineEdit_5->text().toFloat());
          setMachinePitchX(ui->lineEdit_5->text().toFloat());
          break;
        case 1:
          setMachinePitchY(ui->lineEdit_5->text().toFloat());
          break;
        case 2:
          setMachinePitchZ(ui->lineEdit_5->text().toFloat());
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    case 3:
      if(ui->lineEdit_5->text().toInt() > 0)
        {
          qDebug("Acc Set to:%d!", ui->lineEdit_5->text().toInt());
          switch(ui->comboBox_2->currentIndex()){
            case 0:
              setMachineAccX(ui->lineEdit_5->text().toInt());
              break;
            case 1:
              setMachineAccY(ui->lineEdit_5->text().toInt());
              break;
            case 2:
              setMachineAccZ(ui->lineEdit_5->text().toInt());
              break;
            default:
              qDebug("No this Index!");
              break;
            }
        }
      break;
    case 4:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          setMachineDecX(ui->lineEdit_5->text().toInt());
          break;
        case 1:
          setMachineDecY(ui->lineEdit_5->text().toInt());
          break;
        case 2:
          setMachineDecZ(ui->lineEdit_5->text().toInt());
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    case 5:
      switch(ui->comboBox_2->currentIndex())
        {
        case 0:
          qDebug("Start SpeedX want to set:%.02f!", ui->lineEdit_5->text().toFloat());
          setStartSpeedX(ui->lineEdit_5->text().toFloat());
          break;
        case 1:
          setStartSpeedY(ui->lineEdit_5->text().toFloat());
          break;
        case 2:
          setStartSpeedZ(ui->lineEdit_5->text().toFloat());
          break;
        default:
          qDebug("No this Index!");
          break;
        }
      break;
    default:
      break;
    }
}

void Control_3DMachine::on_pushButton_2_released()
{
  qDebug("Go to release?");
}

bool Control_3DMachine::runRelativeFile()
{
  int iMSecs = dataProc.iTimeInterval;
  if(iMSecs <= 0)
    {
      iMSecs = 1000;
    }
  QTime timeStart;
  for(unsigned int i=0; i<dataProc.dataList.size(); i++)
    {
      timeStart.restart();
      //qDebug("TimeStart:%s!Step:%d!", timeStart.toString().toStdString().c_str(), i);
      runRelativeStep(i);
      while(timeStart.elapsed() < iMSecs)
        {
          QCoreApplication::processEvents();
        }
      //qDebug("TimeEnd:%s!\n", QTime::currentTime().toString().toStdString().c_str());
    }
  qDebug("File Ended!");
  return true;
}

bool Control_3DMachine::runAbsoluteFile()
{
  //int iMSecs = dataProc.iTimeInterval;
  //  if(iMSecs <= 0)
  //    {
  //      iMSecs = 200;
  //    }
  if(dataProc.iTimeInterval <= 0)
    {
      dataProc.iTimeInterval = 200;
    }
  dataProc.preProcessAbs();
  //  QTime timeStart;
  //  timeStart.start();
  for(unsigned int i=0; i<dataProc.dataList.size(); i++)
    {
      qDebug("\nTimeNow:%s!StepIndex:%d!", QTime::currentTime().toString().toStdString().c_str(), i);
      runAbsoluteStep(i);
      //      while(m_timeTag.elapsed() < (iMSecs*(i+1)))
      //        {
      //          QCoreApplication::processEvents();
      //        }
      while(m_timeTag.elapsed() < dataProc.iTimeInterval)
        {
          QCoreApplication::processEvents();
        }
    }
  qDebug("File Ended!");
  return true;
}

bool Control_3DMachine::runFileShifting()
{
  int iMSecs = dataProc.iTimeInterval;
  if(iMSecs <= 0)
    {
      iMSecs = 1000;
    }
  dataProc.preProcessShift();
  if(dataProc.shiftList.size() <= 0)
    {
      return false;
    }
  ShiftPre shiftTmp = {0.0, 0, 1, 0.0, 0, 1, 0.0, 0, 1};
  dataProc.shiftList.append(shiftTmp);
  qDebug(" Shift data list Length:%d!", dataProc.shiftList.size());
  QTime timeStart;
  timeStart.start();
  for(unsigned int i=0; i<dataProc.shiftList.size(); i++)
    {
      qDebug("\nTimeNow:%s!Counter:%d!", QTime::currentTime().toString().toStdString().c_str(), i);
      set3AxleStatus(i);
      while(timeStart.elapsed() < (iMSecs*(i+1)))
        {
          QCoreApplication::processEvents();
        }
    }

  qDebug("File Ended!");
  return true;
}

bool Control_3DMachine::set3AxleStatus(int iIndex)
{
  //  if(!setAxleXStat(iIndex))
  //    {
  //      qDebug("Wrong with Set Axle X!");
  //      return false;
  //    }

  //  if(!setAxleYStat(iIndex))
  //    {
  //      qDebug("Wrong with Set Axle Y!");
  //      return false;
  //    }

  if(!setAxleZStat(iIndex))
    {
      qDebug("Wrong with Set Axle Z!");
      return false;
    }

  return true;
}

bool Control_3DMachine::setAxleXStat(int iIndex)
{
  if(true == dataProc.shiftList[iIndex].bChangX)
    {
      if(iIndex > 0)
        {
          switch(dataProc.shiftList[iIndex-1].iDirX)
            {
            case 1:
              stopXRight();
              break;
            case 0:
              //qDebug("Warning Axle X is Static!");
              break;
            case -1:
              stopXLeft();
              break;
            default:
              qDebug("Error Axle X no this case!");
              break;
            }
          switch(dataProc.shiftList[iIndex].iDirX)
            {
            case 1:
              moveXRight();
              break;
            case 0:
              //qDebug("Warning Axle X is Static!");
              break;
            case -1:
              moveXLeft();
              break;
            default:
              qDebug("Warning Axle X no this case!");
              break;
            }
        }else
        {
          switch(dataProc.shiftList[iIndex].iDirX)
            {
            case 1:
              moveXRight();
              break;
            case 0:
              //qDebug("Warning Axle X is Static!");
              break;
            case -1:
              moveXLeft();
              break;
            default:
              qDebug("Warning Axle X no this case!");
              break;
            }
        }
    }
  if(!((iIndex>0)&&(dataProc.shiftList[iIndex].fSpeedX == dataProc.shiftList[iIndex-1].fSpeedX)&&(dataProc.shiftList[iIndex-1].iDirX == dataProc.shiftList[iIndex].iDirX)))
    {
      qDebug("Set X speed:%f!", dataProc.shiftList[iIndex].fSpeedX);
      if(!setMachineSpeedX(dataProc.shiftList[iIndex].fSpeedX))
        {
          qDebug("Set Speed X fail");
          //return false;
        }
    }
  return true;
}

bool Control_3DMachine::setAxleYStat(int iIndex)
{
  if(true == dataProc.shiftList[iIndex].bChangY)
    {
      if(iIndex > 0)
        {
          switch(dataProc.shiftList[iIndex-1].iDirY)
            {
            case 1:
              stopYRight();
              break;
            case 0:
              //qDebug("Warning Axle Y is Static!");
              break;
            case -1:
              stopYLeft();
              break;
            default:
              qDebug("Error Axle Y no this case!");
              break;
            }
          switch(dataProc.shiftList[iIndex].iDirY)
            {
            case 1:
              moveYRight();
              break;
            case 0:
              //qDebug("Warning Axle Y is Static!");
              break;
            case -1:
              moveYLeft();
              break;
            default:
              qDebug("Warning Axle Y no this case!");
              break;
            }
        }else
        {
          switch(dataProc.shiftList[iIndex].iDirY)
            {
            case 1:
              moveYRight();
              break;
            case 0:
              //qDebug("Warning Axle Y is Static!");
              break;
            case -1:
              moveYLeft();
              break;
            default:
              qDebug("Warning Axle Y no this case!");
              break;
            }
        }
    }
  if(!((iIndex>0)&&(dataProc.shiftList[iIndex].fSpeedY == dataProc.shiftList[iIndex-1].fSpeedY)&&(dataProc.shiftList[iIndex-1].iDirY == dataProc.shiftList[iIndex].iDirY)))
    {
      qDebug("Set Y speed:%f!", dataProc.shiftList[iIndex].fSpeedY);
      if(!setMachineSpeedY(dataProc.shiftList[iIndex].fSpeedY))
        {
          qDebug("Set Speed Y fail");
          //return false;
        }
    }
  return true;
}

bool Control_3DMachine::setAxleZStat(int iIndex)
{
  if(true == dataProc.shiftList[iIndex].bChangZ)
    {
      QTime timeNow;
      if(iIndex > 0)
        {
          switch(dataProc.shiftList[iIndex-1].iDirZ)
            {
            case 1:
              qDebug("Stop Axle Z Right!");
              stopZRight();
              qDebug("timeStart:%s!", timeNow.currentTime().toString().toStdString().c_str());
              timeNow.start();
              while(timeNow.elapsed() < 2000){
                  QCoreApplication::processEvents();
                };
              qDebug("timeEnd:%s!", timeNow.currentTime().toString().toStdString().c_str());
              break;
            case 0:
              //qDebug("Warning Axle Z is Static!");
              break;
            case -1:
              qDebug("Stop Axle Z Left!");
              stopZLeft();
              qDebug("timeStart:%s!", timeNow.currentTime().toString().toStdString().c_str());
              timeNow.start();
              while(timeNow.elapsed() < 2000){
                  QCoreApplication::processEvents();
                };
              qDebug("timeEnd:%s!", timeNow.currentTime().toString().toStdString().c_str());
              break;
            default:
              qDebug("Error Axle Z no this case!");
              break;
            }
          if(!setMachineSpeedZ(dataProc.shiftList[iIndex].fSpeedZ))
            {
              qDebug("Set Speed Z fail");
              //return false;
            }else{
              qDebug("Set Speed Z %f OK!", dataProc.shiftList[iIndex].fSpeedZ);
            }
          switch(dataProc.shiftList[iIndex].iDirZ)
            {
            case 1:
              qDebug("Move Axle Z Right!");
              moveZRight();
              break;
            case 0:
              //qDebug("Warning Axle Z is Static!");
              break;
            case -1:
              qDebug("Move Axle Z Left!");
              moveZLeft();
              break;
            default:
              qDebug("Warning Axle Z no this case!");
              break;
            }
        }else
        {
          if(dataProc.shiftList[iIndex].fSpeedZ != 0)
            {
              if(!setMachineSpeedZ(dataProc.shiftList[iIndex].fSpeedZ))
                {
                  qDebug("Set Speed Z fail");
                  //return false;
                }
            }
          switch(dataProc.shiftList[iIndex].iDirZ)
            {
            case 1:
              qDebug("Move Axle Z Right!");
              moveZRight();
              break;
            case 0:
              //qDebug("Warning Axle Z is Static!");
              break;
            case -1:
              qDebug("Move Axle Z Right!");
              moveZLeft();
              break;
            default:
              qDebug("Warning Axle Z no this case!");
              break;
            }
        }
    }else
    {
      if(dataProc.shiftList[iIndex].fSpeedZ != dataProc.shiftList[iIndex-1].fSpeedZ)
        {
          if(!setMachineSpeedZ(dataProc.shiftList[iIndex].fSpeedZ))
            {
              qDebug("Set Speed Z fail");
              //return false;
            }else
            {
              qDebug("Set Speed Z %f OK!", dataProc.shiftList[iIndex].fSpeedZ);
            }
        }
    }

  return true;
}

bool Control_3DMachine::runAbsoluteStep(unsigned int iStepIndex)
{
  bool bFlag = true;

  if(dataProc.absList[iStepIndex].fOffsetX != 0)
    {
      if((iStepIndex == 0)||(dataProc.absList[iStepIndex].fSpeedX != dataProc.absList[iStepIndex-1].fSpeedX))
        {
          setMachineSpeedX(dataProc.absList[iStepIndex].fSpeedX);
          qDebug("Set X Speed %f!", dataProc.absList[iStepIndex].fSpeedX);
        }
      if(!moveXAbsolute(dataProc.absList[iStepIndex].fPosX))
        {
          qDebug("Move X fail!");
          bFlag = false;
        }else
        {
          qDebug("Move X %f!", dataProc.absList[iStepIndex].fPosX);
        }
    }

  if(dataProc.absList[iStepIndex].fOffsetY != 0)
    {
      if((iStepIndex == 0)||(dataProc.absList[iStepIndex].fSpeedY != dataProc.absList[iStepIndex-1].fSpeedY))
        {
          setMachineSpeedY(dataProc.absList[iStepIndex].fSpeedY);
          qDebug("Set Y Speed %f!", dataProc.absList[iStepIndex].fSpeedY);
        }
      if(!moveYAbsolute(dataProc.absList[iStepIndex].fPosY))
        {
          qDebug("Move Y fail!");
          bFlag = false;
        }else
        {
          qDebug("Move Y %f!", dataProc.absList[iStepIndex].fPosY);
        }
    }

  if(dataProc.absList[iStepIndex].fOffsetZ != 0)
    {
      if(iStepIndex > 0)
        {
          if(dataProc.absList[iStepIndex].fSpeedZ != dataProc.absList[iStepIndex-1].fSpeedZ)
            {
              qDebug("Speed not equal:%f != %f!", dataProc.absList[iStepIndex].fSpeedZ, dataProc.absList[iStepIndex-1].fSpeedZ);
              setMachineSpeedZ(dataProc.absList[iStepIndex].fSpeedZ);
              qDebug("Set Z Speed[%d] %f!", iStepIndex, dataProc.absList[iStepIndex].fSpeedZ);
            }
        }else if(iStepIndex == 0)
        {
          setMachineSpeedZ(dataProc.absList[iStepIndex].fSpeedZ);
          qDebug("Set Z Speed %f!First!", dataProc.absList[iStepIndex].fSpeedZ);
        }
      if(!moveZAbsolute(dataProc.absList[iStepIndex].fPosZ))
        {
          qDebug("Move Z fail!");
          bFlag = false;
        }else{
          qDebug("Move Z[%d] %f!", iStepIndex, dataProc.absList[iStepIndex].fPosZ);
        }
    }

  m_timeTag.restart();

  return bFlag;
}

bool Control_3DMachine::runRelativeStep(unsigned int iStepIndex)
{
  bool bFlag = true;
  if(!moveXRelative(dataProc.dataList[iStepIndex].fDistanceX))
    {
      qDebug("Move X fail!");
      bFlag = false;
    }
  if(!moveYRelative(dataProc.dataList[iStepIndex].fDistanceY))
    {
      qDebug("Move Y fail!");
      bFlag = false;
    }
  if(!moveZRelative(dataProc.dataList[iStepIndex].fDistanceZ))
    {
      qDebug("Move Z fail!");
      bFlag = false;
    }
  return bFlag;
}

void Control_3DMachine::on_pushButton_2_pressed()
{
  qDebug("Steps:%d!Press!", dataProc.dataList.size());
  if(dataProc.dataList.size() > 3)
    {
      //runRelativeFile();
      //runAbsoluteFile();
      runFileShifting();
    }
}
