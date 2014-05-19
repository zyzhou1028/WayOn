
#include<stdio.h>

#include "dataprocess.h"

dataprocess::dataprocess()
{
    iSteps = 0;
    iTimeInterval = 1;
}

int dataprocess::parse(QTextStream& dataStream)
{
    OperateData dataTmp;
    QString strLine;
    iSteps = 0;
    while((strLine = dataStream.readLine()).length() > 7){
        qDebug("Line:%s!", strLine.toStdString().c_str());
        if(sscanf(strLine.toStdString().c_str(), "{%d,%d,%d}", &dataTmp.iDistanceX, &dataTmp.iDistanceY, &dataTmp.iDistanceZ) > 0){
            dataList.append(dataTmp);
            iSteps++;
        }
    }
    qDebug("Total steps:%d!", iSteps);
    for(int i=0; i<dataList.length();i++){
        qDebug("X:%d Y:%d Z:%d", dataList[i].iDistanceX, dataList[i].iDistanceY, dataList[i].iDistanceZ);
    }
    return iSteps;
}
