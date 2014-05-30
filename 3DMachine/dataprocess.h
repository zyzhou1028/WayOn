#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <QList>
#include <QTextStream>

typedef struct _OperateData{
    float fDistanceX;
    float fDistanceY;
    float fDistanceZ;
}OperateData;

typedef struct _ShiftPre{
    float fSpeedX;
    short iDirX;
    bool bChangX;

    float fSpeedY;
    short iDirY;
    bool bChangY;

    float fSpeedZ;
    short iDirZ;
    bool bChangZ;
}ShiftPre;

typedef struct _AbsPre{
    float fSpeedX;
    float fPosX;
    float fOffsetX;

    float fSpeedY;
    float fPosY;
    float fOffsetY;

    float fSpeedZ;
    float fPosZ;
    float fOffsetZ;
}AbsPre;

class dataprocess
{
public:
    dataprocess();

    int iTimeInterval;
    int m_iSteps;

    int parse(QTextStream& dataStream);
    bool preProcessShift();
    bool preProcessAbs();

    QList<OperateData> dataList;
    QList<ShiftPre> shiftList;
    QList<AbsPre> absList;
private:


};

#endif // DATAPROCESS_H
