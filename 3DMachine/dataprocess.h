#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <QList>
#include <QTextStream>

typedef struct _OperateData{
    float fDistanceX;
    float fDistanceY;
    float fDistanceZ;
}OperateData;

class dataprocess
{
public:
    dataprocess();

    int iTimeInterval;
    int iSteps;

    int parse(QTextStream& dataStream);

    QList<OperateData> dataList;
private:


};

#endif // DATAPROCESS_H
