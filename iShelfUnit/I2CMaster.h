#ifndef __I2C_MASTER
#define __I2C_MASTER

#include <stdint.h>
//#include "singleton.h"

using namespace std;

class I2CMaster
{
	public:
		static void Init();
		static void ReadData(uint8_t slaveAddr,uint8_t registerAddr,uint8_t readLength,uint8_t *data);
		static uint8_t ReadByte(uint8_t slaveAddr,uint8_t registerAddr);
		static void WriteByte(uint8_t slaveAddr,uint8_t registerAddr,uint8_t data);
		static void WriteCommand(uint8_t slaveAddr,uint8_t command);
		static void WriteData(uint8_t slaveAddr,uint8_t registerAddr,uint8_t writeLength,uint8_t* data);
	private:
		I2CMaster() {}; 
		~I2CMaster() {};
};

#endif
