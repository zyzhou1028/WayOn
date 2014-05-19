#include <string.h>
#include <QString>

#pragma pack(push)
#pragma pack(1)
typedef	struct _DataPacket
{
    char address;
    char commandCode;
    char startRegisterHi;
    char startRegisterLo;
    char readRegisterHi;
    char readRegisterLo;
    char CRCLo;
    char CRCHi;
}DataPacket,* DataPacketPtr;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef	struct _resDataPacket
{
    char resaddress;
    char rescommandCode;
    char dataCount;
    char dataByte1;
    char dataByte2;
    char resCRCLo;
    char resCRCHi;
}resDataPacket,* resDataPacketPtr;
#pragma pack(pop)

class Modbus
{
public:
    Modbus(void);
    ~Modbus(void);
public:
    DataPacketPtr pDataPacket;
    resDataPacketPtr pResponseData;

    unsigned short iCoefficientX;
    unsigned short iCoefficientY;
    unsigned short iCoefficientZ;

    float iSpeedX;
    float iSpeedY;
    float iSpeedZ;

    float iPitchX;
    float iPitchY;
    float iPitchZ;

    unsigned short iSubdivX;
    unsigned short iSubdivY;
    unsigned short iSubdivZ;

    std::string m_strReadBuf;
    unsigned int m_iReadCounter;
public:
    unsigned short getCRC(unsigned char *szData, unsigned short iLen);
    void buildPacket(char* szDest, const char* szCMD, unsigned int iLen);
    void setResponse(char address,char commandCode,char dataCount,char databyte1,
                     char databyte2,char CRChi,char CRCLo);
    bool checkResponse();
    float getFloat(unsigned char *szModbusF);
    void setFloat(unsigned char *szModbusF, float fSrc) ;
    short getReg(unsigned char *szModbusSh);
    void setReg(unsigned char *szModbusSh, short iValue);
};


