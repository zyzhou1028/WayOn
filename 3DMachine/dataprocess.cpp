
#include<stdio.h>

#include "dataprocess.h"

dataprocess::dataprocess()
{
  m_iSteps = 0;
  iTimeInterval = 1000;
}

int dataprocess::parse(QTextStream& dataStream)
{
  OperateData dataTmp;
  QString strLine;
  m_iSteps = 0;
  while((strLine = dataStream.readLine()).length() >= 7)
    {
      //        qDebug("Line:%s!", strLine.toStdString().c_str());
      if(sscanf(strLine.toStdString().c_str(), "{%f,%f,%f}", &dataTmp.fDistanceX, &dataTmp.fDistanceY, &dataTmp.fDistanceZ) > 0){
          dataList.append(dataTmp);
          m_iSteps++;
        }
    }
  qDebug("Total steps:%d!", m_iSteps);
  for(int i=0; i<dataList.length();i++){
      qDebug("X:%f Y:%f Z:%f", dataList[i].fDistanceX, dataList[i].fDistanceY, dataList[i].fDistanceZ);
    }
  // preProcess();
  return m_iSteps;
}

bool dataprocess::preProcessShift()
{
  qDebug("PreProcess! iSteps:%d!iTimeInterval:%d!", m_iSteps, iTimeInterval);
  if(m_iSteps > 0)
    {
      ShiftPre preTmp;
      for(unsigned short i=0; i<m_iSteps; i++)
        {
          if(i > 0){
              preTmp.fSpeedX = ((dataList[i].fDistanceX - dataList[i-1].fDistanceX)/iTimeInterval)*1000;
              if(dataList[i].fDistanceX > dataList[i-1].fDistanceX){
                  preTmp.iDirX = 1;
                }else if(dataList[i].fDistanceX < dataList[i-1].fDistanceX){
                  preTmp.iDirX = -1;
                }else{
                  preTmp.iDirX = shiftList[i-1].iDirX;
                }
              if(shiftList[i-1].iDirX != preTmp.iDirX){
                  preTmp.bChangX = true;
                }else{
                  preTmp.bChangX = false;
                }

              preTmp.fSpeedY = ((dataList[i].fDistanceY - dataList[i-1].fDistanceY)/iTimeInterval)*1000;
              if(dataList[i].fDistanceY > dataList[i-1].fDistanceY){
                  preTmp.iDirY = 1;
                }else if(dataList[i].fDistanceY < dataList[i-1].fDistanceY){
                  preTmp.iDirY = -1;
                }else{
                  preTmp.iDirY = shiftList[i-1].iDirY;
                }
              if(shiftList[i-1].iDirY != preTmp.iDirY){
                  preTmp.bChangY = true;
                }else{
                  preTmp.bChangY = false;
                }

              preTmp.fSpeedZ = ((dataList[i].fDistanceZ - dataList[i-1].fDistanceZ)/iTimeInterval)*1000;
              if(dataList[i].fDistanceZ > dataList[i-1].fDistanceZ){
                  preTmp.iDirZ = 1;
                }else if(dataList[i].fDistanceZ < dataList[i-1].fDistanceZ){
                  preTmp.iDirZ = -1;
                }else{
                  preTmp.iDirZ = shiftList[i-1].iDirZ;
                }
              if(shiftList[i-1].iDirZ != preTmp.iDirZ){
                  preTmp.bChangZ = true;
                }else{
                  preTmp.bChangZ = false;
                }
            }else{
              preTmp.fSpeedX = (dataList[i].fDistanceX)*(1000/iTimeInterval);
              if(dataList[i].fDistanceX > 0){
                  preTmp.iDirX = 1;
                }else if(dataList[i].fDistanceX < 0){
                  preTmp.iDirX = -1;
                }else{
                  preTmp.iDirX = 0;
                }
              preTmp.bChangX = true;

              preTmp.fSpeedY = (dataList[i].fDistanceY)*(1000/iTimeInterval);
              if(dataList[i].fDistanceY > 0){
                  preTmp.iDirY = 1;
                }else if(dataList[i].fDistanceY < 0){
                  preTmp.iDirY = -1;
                }else{
                  preTmp.iDirY = 0;
                }
              preTmp.bChangY = true;

              preTmp.fSpeedZ = (dataList[i].fDistanceZ)*(1000/iTimeInterval);
              if(dataList[i].fDistanceZ > 0){
                  preTmp.iDirZ = 1;
                }else if(dataList[i].fDistanceZ < 0){
                  preTmp.iDirZ = -1;
                }else{
                  preTmp.iDirZ = 0;
                }
              preTmp.bChangZ = true;
            }
          if(preTmp.fSpeedX < 0){
              preTmp.fSpeedX = preTmp.fSpeedX * (-1);
            }
          if(preTmp.fSpeedY < 0){
              preTmp.fSpeedY = preTmp.fSpeedY * (-1);
            }
          if(preTmp.fSpeedZ < 0){
              preTmp.fSpeedZ = preTmp.fSpeedZ * (-1);
            }
          //qDebug("preTmp: {%f, %d, %s, %f, %d, %s, %f, %d, %s}", preTmp.fSpeedX, preTmp.iDirX, preTmp.bChangX==false?"false":"true", preTmp.fSpeedY, preTmp.iDirY, preTmp.bChangY==false?"false":"true", preTmp.fSpeedZ, preTmp.iDirZ, preTmp.bChangZ==false?"false":"true");
          shiftList.append(preTmp);
          qDebug("shift[%d]: {%f, %d, %s, %f, %d, %s, %f, %d, %s}", i, shiftList[i].fSpeedX, shiftList[i].iDirX, shiftList[i].bChangX==false?"false":"true", shiftList[i].fSpeedY, shiftList[i].iDirY, shiftList[i].bChangY==false?"false":"true", shiftList[i].fSpeedZ, shiftList[i].iDirZ, shiftList[i].bChangZ==false?"false":"true");
        }
    }else{
      return false;
    }
  return true;
}

bool dataprocess::dataprocess::preProcessAbs()
{
  qDebug("preProcessAbs! iSteps:%d!iTimeInterval:%d!", m_iSteps, iTimeInterval);
  absList.clear();
  if(m_iSteps > 0)
    {
      AbsPre preTmp;
      for(unsigned short i=0; i<m_iSteps; i++)
        {
          preTmp.fPosX = dataList[i].fDistanceX;
          preTmp.fPosY = dataList[i].fDistanceY;
          preTmp.fPosZ = dataList[i].fDistanceZ;
          if(i > 0){
              preTmp.fOffsetX = dataList[i].fDistanceX - dataList[i-1].fDistanceX;
              preTmp.fOffsetY = dataList[i].fDistanceY - dataList[i-1].fDistanceY;
              preTmp.fOffsetZ = dataList[i].fDistanceZ - dataList[i-1].fDistanceZ;
            }else{
              preTmp.fOffsetX = dataList[i].fDistanceX;
              preTmp.fOffsetY = dataList[i].fDistanceY;
              preTmp.fOffsetZ = dataList[i].fDistanceZ;
            }
          preTmp.fSpeedX = (preTmp.fOffsetX)*(1000/iTimeInterval);
          if(preTmp.fSpeedX<0){
              preTmp.fSpeedX = preTmp.fSpeedX*(-1);
            }
          preTmp.fSpeedY = (preTmp.fOffsetY)*(1000/iTimeInterval);
          if(preTmp.fSpeedY<0){
              preTmp.fSpeedY = preTmp.fSpeedY*(-1);
            }
          preTmp.fSpeedZ = (preTmp.fOffsetZ)*(1000/iTimeInterval);
          if(preTmp.fSpeedZ<0){
              preTmp.fSpeedZ = preTmp.fSpeedZ*(-1);
            }
          absList.append(preTmp);
        }
    }else{
      return false;
    }
  for(int i=0; i<absList.size(); i++){
      qDebug("AbsList[%d]: {%f, %f, %f, %f, %f, %f, %f, %f, %f}", i, absList[i].fPosX, absList[i].fOffsetX, absList[i].fSpeedX,absList[i].fPosY,  absList[i].fOffsetY, absList[i].fSpeedY,absList[i].fPosZ,  absList[i].fOffsetZ, absList[i].fSpeedZ);
    }
  return true;
}
