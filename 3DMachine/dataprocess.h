#ifndef DATAPROCESS_H
#define DATAPROCESS_H

#include <QList>
#include <QTextStream>

typedef struct _OperateData{
    int iDistanceX;
    int iDistanceY;
    int iDistanceZ;
}OperateData;

class dataprocess
{
public:
    dataprocess();

    int iTimeInterval;
    int iSteps;

    int parse(QTextStream& dataStream);
private:
    QList<OperateData> dataList;

};

#endif // DATAPROCESS_H
