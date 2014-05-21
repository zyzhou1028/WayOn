
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
        //        qDebug("Line:%s!", strLine.toStdString().c_str());
        if(sscanf(strLine.toStdString().c_str(), "{%f,%f,%f}", &dataTmp.fDistanceX, &dataTmp.fDistanceY, &dataTmp.fDistanceZ) > 0){
            dataList.append(dataTmp);
            iSteps++;
        }
    }
    qDebug("Total steps:%d!", iSteps);
    //    for(int i=0; i<dataList.length();i++){
    //        qDebug("X:%f Y:%f Z:%f", dataList[i].fDistanceX, dataList[i].fDistanceY, dataList[i].fDistanceZ);
    //    }
    return iSteps;
}
