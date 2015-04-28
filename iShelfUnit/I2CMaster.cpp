#include "I2CMaster.h"
#include "I2C.h"
#include <cstring>

using namespace std;

extern volatile uint32_t I2CCount;
extern volatile uint8_t I2CMasterBuffer[I2C_BUFSIZE];
extern volatile uint8_t I2CSlaveBuffer[I2C_BUFSIZE];
extern volatile uint32_t I2CMasterState;
extern volatile uint32_t I2CReadLength, I2CWriteLength;

void I2CMaster::Init()
{
	I2CInit(I2CMASTER);
}

uint8_t I2CMaster::ReadByte(uint8_t slaveAddr,uint8_t registerAddr)
{
	I2CSlaveBuffer[0]=0;
	I2CWriteLength = 2;
  I2CReadLength = 1;
  I2CMasterBuffer[0] = slaveAddr;
  I2CMasterBuffer[1] = registerAddr;		/* address */
  I2CMasterBuffer[2] = slaveAddr | RD_BIT;
  I2CEngine();
	return I2CSlaveBuffer[0];
}
		
void I2CMaster::ReadData(uint8_t slaveAddr,uint8_t registerAddr,uint8_t readLength,uint8_t *data)
{
	I2CWriteLength = 2;
  I2CReadLength = readLength;
  I2CMasterBuffer[0] = slaveAddr;
  I2CMasterBuffer[1] = registerAddr;		/* address */
  I2CMasterBuffer[2] = slaveAddr | RD_BIT;
  I2CEngine();
	if (data != NULL)
		memcpy(data,(const void*)I2CSlaveBuffer,readLength);
}

void I2CMaster::WriteByte(uint8_t slaveAddr,uint8_t registerAddr,uint8_t data)
{
	I2CWriteLength = 3;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = slaveAddr;
  I2CMasterBuffer[1] = registerAddr;		/* address */
  I2CMasterBuffer[2] = data;		/* Data0 */
  I2CEngine();
}

void I2CMaster::WriteCommand(uint8_t slaveAddr,uint8_t command)
{
	I2CWriteLength = 2;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = slaveAddr;
  I2CMasterBuffer[1] = command;		/* address */
  I2CEngine();
}

void I2CMaster::WriteData(uint8_t slaveAddr,uint8_t registerAddr,uint8_t writeLength,uint8_t* data)
{
	if (data == NULL)
		return;
	I2CWriteLength = writeLength+2;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = slaveAddr;
  I2CMasterBuffer[1] = registerAddr;		/* address */
	memcpy((void*)(I2CMasterBuffer+2),data,writeLength);
  I2CEngine();
}
