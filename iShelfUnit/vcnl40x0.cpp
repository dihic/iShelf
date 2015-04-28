#include "VCNL40x0.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::Init() 
{
	I2CMaster::Init(); // set I2C frequency to 1MHz
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetCommandRegister (unsigned char Command) 
{
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_COMMAND, Command);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadCommandRegister (unsigned char *Command) 
{
	*Command = I2CMaster::ReadByte(VCNL40x0_ADDRESS, REGISTER_COMMAND);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadID (unsigned char *ID) 
{
	*ID = I2CMaster::ReadByte(VCNL40x0_ADDRESS, REGISTER_ID);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetCurrent (unsigned char Current) 
{
	// VCNL40x0 IR LED Current register
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_PROX_CURRENT, Current);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadCurrent (unsigned char *Current) 
{
	// VCNL40x0 IR LED current register
	*Current = I2CMaster::ReadByte(VCNL40x0_ADDRESS, REGISTER_PROX_CURRENT);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetProximityRate (unsigned char ProximityRate) 
{
	// VCNL40x0 Proximity rate register
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_PROX_RATE, ProximityRate);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetAmbiConfiguration (unsigned char AmbiConfiguration) 
{
	// VCNL40x0 Ambient light configuration
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_AMBI_PARAMETER, AmbiConfiguration);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetInterruptControl (unsigned char InterruptControl) 
{
	// VCNL40x0 Interrupt Control register
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_CONTROL, InterruptControl);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadInterruptControl (unsigned char *InterruptControl) 
{
	// VCNL40x0 Interrupt Control register
	*InterruptControl = I2CMaster::ReadByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_CONTROL);
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetInterruptStatus (unsigned char InterruptStatus) {
	// VCNL40x0 Interrupt Status register
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_STATUS, InterruptStatus);
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetModulatorTimingAdjustment (unsigned char ModulatorTimingAdjustment) {
	// VCNL40x0 Modulator Timing Adjustment register
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_PROX_TIMING, ModulatorTimingAdjustment);
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadInterruptStatus (unsigned char *InterruptStatus) {
	// VCNL40x0 Interrupt Status register
	*InterruptStatus = I2CMaster::ReadByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_STATUS);
return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadProxiValue (unsigned int *ProxiValue) 
{
	uint8_t _receive[2];
	I2CMaster::ReadData(VCNL40x0_ADDRESS, REGISTER_PROX_VALUE, 2, _receive);
	*ProxiValue = ((unsigned int)_receive[0] << 8 | (unsigned char)_receive[1]);
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadAmbiValue (unsigned int *AmbiValue) 
{
	uint8_t _receive[2];
	I2CMaster::ReadData(VCNL40x0_ADDRESS, REGISTER_AMBI_VALUE, 2, _receive);
	*AmbiValue = ((unsigned int)_receive[0] << 8 | (unsigned char)_receive[1]);
	return VCNL40x0_ERROR_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetLowThreshold (unsigned int LowThreshold) 
{
	unsigned char LoByte=0, HiByte=0;
	LoByte = (unsigned char)(LowThreshold & 0x00ff);
	HiByte = (unsigned char)((LowThreshold & 0xff00)>>8);
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_LOW_THRES, HiByte);
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_LOW_THRES+1, LoByte);
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::SetHighThreshold (unsigned int HighThreshold) 
{
	unsigned char LoByte=0, HiByte=0;
	LoByte = (unsigned char)(HighThreshold & 0x00ff);
	HiByte = (unsigned char)((HighThreshold & 0xff00)>>8);	
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_HIGH_THRES, HiByte);
	I2CMaster::WriteByte(VCNL40x0_ADDRESS, REGISTER_INTERRUPT_HIGH_THRES+1, LoByte);
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadProxiOnDemand (unsigned int *ProxiValue) 
{
	unsigned char Command=0;
	// enable prox value on demand
	SetCommandRegister(COMMAND_PROX_ENABLE | COMMAND_PROX_ON_DEMAND);
	// wait on prox data ready bit
	do 
	{
		ReadCommandRegister (&Command); // read command register
	} while (!(Command & COMMAND_MASK_PROX_DATA_READY));
	ReadProxiValue (ProxiValue); // read prox value
	SetCommandRegister (COMMAND_ALL_DISABLE); // stop prox value on demand
	return VCNL40x0_ERROR_OK;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VCNL40x0Error_e VCNL40x0::ReadAmbiOnDemand (unsigned int *AmbiValue) 
{
	unsigned char Command=0;
	// enable ambi value on demand
	SetCommandRegister (COMMAND_PROX_ENABLE | COMMAND_AMBI_ON_DEMAND);
	// wait on ambi data ready bit
	do 
	{
		ReadCommandRegister (&Command); // read command register
	} while (!(Command & COMMAND_MASK_AMBI_DATA_READY));
	ReadAmbiValue (AmbiValue); // read ambi value
	SetCommandRegister (COMMAND_ALL_DISABLE); // stop ambi value on demand
	return VCNL40x0_ERROR_OK;
}

