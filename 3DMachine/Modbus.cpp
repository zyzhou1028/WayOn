#include <QDebug>
#include <stddef.h>
#include <memory.h>
#include "Modbus.h"

Modbus::Modbus(void)
{
    this->pDataPacket=new DataPacket;
    memset(pDataPacket, 0x0, sizeof(_DataPacket));

    this->pResponseData=new resDataPacket;
    memset(pResponseData, 0x0, sizeof(_resDataPacket));
}


Modbus::~Modbus(void)
{
    if(pDataPacket)
    {
        delete (pDataPacket);
        (pDataPacket)=NULL;
    }
    if(pResponseData)
    {
        delete (pResponseData);
        (pResponseData)=NULL;
    }
}

static unsigned char auchCRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static char auchCRCLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

unsigned short Modbus::getCRC ( unsigned char *szData, unsigned short iLen )
{
    unsigned char uchCRCHi = 0xFF ;
    unsigned char uchCRCLo = 0xFF ;
    unsigned uIndex ;
    while (iLen--)
    {
        uIndex = uchCRCLo ^ *szData++ ;
        uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
        uchCRCHi = auchCRCLo[uIndex] ;
    }
    return (uchCRCHi << 8 | uchCRCLo) ;
}

void Modbus::buildPacket(char* szDest, const char* szCMD, unsigned int iLen)
{
    memcpy(szDest, szCMD, iLen);
    unsigned short shCRC=getCRC((unsigned char*)szDest,iLen);
    szDest[iLen] = (char)shCRC;
    szDest[iLen + 1] = (char)(shCRC>>8);
    szDest[iLen + 2] = '\0';
    qDebug("CRC:%02x %02x : %02x %02x !", (char)shCRC, (char)(shCRC>>8), szDest[iLen], szDest[iLen + 1]);
}

void Modbus::setResponse(char szAddr, char szIndex, char szCount, char szByte1, char szByte2, char CRCLo, char CRChi)
{
    qDebug("Set szAddr:%02x!", szAddr);
    pResponseData->resaddress=szAddr;

    qDebug("Set szIndex:%02x!", szIndex);
    pResponseData->rescommandCode=szIndex;

    qDebug("Set szCount:%02x!", szCount);
    pResponseData->dataCount=szCount;

    qDebug("Set szByte1:%02x!", szByte1);
    pResponseData->dataByte1=szByte1;

    qDebug("Set szByte2:%02x!", szByte2);
    pResponseData->dataByte2=szByte2;

    qDebug("Set CRCLo:%02x!", CRCLo);
    pResponseData->resCRCLo=CRCLo;

    qDebug("Set CRChi:%02x!", CRChi);
    pResponseData->resCRCHi=CRChi;
}

bool Modbus::checkResponse()
{
    qDebug("CheckResponse Start!Len:%d!", m_strReadBuf.length());
    if(m_strReadBuf.length() <= 0){
        return false;
    }

    qDebug(" CRC check Data:");
    for(int i=0; i<m_strReadBuf.length();i++)
    {
        qDebug("%02x", (char)m_strReadBuf.c_str()[i]);
    }

    short resCRC;
    unsigned char szTmp[m_strReadBuf.length() - 1];
    memcpy((char *)szTmp, m_strReadBuf.c_str(), m_strReadBuf.length() -2);

    resCRC=getCRC(szTmp, m_strReadBuf.length() - 2);
    char resCRCLo=resCRC;
    char resCRCHi=resCRC>>8;
    qDebug("CRC Check:%02x->%02x, %02x->%02x!Len:%d!", resCRCLo, m_strReadBuf.c_str()[m_strReadBuf.length() - 2], resCRCHi, m_strReadBuf.c_str()[m_strReadBuf.length() - 1], m_strReadBuf.length() - 2);
    if (m_strReadBuf.c_str()[m_strReadBuf.length() - 2]==resCRCLo &&m_strReadBuf.c_str()[m_strReadBuf.length() - 1]==resCRCHi){
        return true;
    }
    return false;
}


float Modbus::getFloat(unsigned char *szModbusF) {
    float fDest;
    unsigned char *szTmp;
    szTmp=(unsigned char*)&fDest;
    *szTmp=szModbusF[1];
    szTmp++;
    *szTmp=szModbusF[0];
    szTmp++;
    *szTmp=szModbusF[3];
    szTmp++;
    *szTmp=szModbusF[2];
    //qDebug("Modbus float:%f!", fDest);
    return fDest;
}

short Modbus::getReg(unsigned char *szModbusSh) {
    short iDest;
    unsigned char *szTmp;
    szTmp=(unsigned char*)&iDest;
    *szTmp=szModbusSh[1];
    szTmp++;
    *szTmp=szModbusSh[0];

    qDebug("Modbus short:%d!", iDest);
    return iDest;
}

void Modbus::setFloat(unsigned char *szModbusF, float fSrc) {
    unsigned char *szTmp;
    szTmp=(unsigned char*)&fSrc;
    memcpy(szModbusF, szTmp+1, 1);
    memcpy(szModbusF+1, szTmp, 1);
    memcpy(szModbusF+2, szTmp+3, 1);
    memcpy(szModbusF+3, szTmp+2, 1);
    qDebug("Modbus float:%f!", fSrc);
    for(int i=0; i<4; i++){
        qDebug("%02x %02x", szModbusF[i], szTmp[i]);
    }
}

void Modbus::setReg(unsigned char *szModbusSh, short iValue) {
    unsigned char *szTmp;
    szTmp=(unsigned char*)&iValue;
    memcpy(szModbusSh, szTmp+1, 1);
    memcpy(szModbusSh+1, szTmp, 1);
    qDebug("Modbus Reg short:%d!", iValue);
    for(int i=0; i<2; i++){
        qDebug("%02x %02x", szModbusSh[i], szTmp[i]);
    }
}
