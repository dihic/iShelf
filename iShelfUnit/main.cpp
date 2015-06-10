#include <LPC11xx.h>
#include <cstring>
#include <cmath>
#include "gpio.h"
#include "timer32.h"
#include "ssp.h"
#include "trf796x.h"
#include "iso15693.h"
#include "canonchip.h"
#include "VCNL40x0.h"
#include "delay.h"

using namespace std;

#define POS_LED  PORT2,7
#define R1_LED 	 PORT0,2
#define G1_LED 	 PORT0,3
#define R2_LED 	 PORT0,6
#define G2_LED 	 PORT0,7
#define VCNL_INT PORT2,8

#define OP_SET_INDICATOR 0x8002
#define SYNC_GOTCHA			0x0180
#define SYNC_RFID				0x0100
#define SYNC_LIVE				0x01ff

#define MEM_BUFSIZE 0x200

#define RFID_TIME_COUNT    	5
#define RFID_TIME_INTERVAL 	125
#define LED_INTERVAL 				500

__align(16) uint8_t MemBuffer[MEM_BUFSIZE];

uint8_t buf[300];
volatile uint8_t i_reg = 0x01;							// interrupt register
volatile uint8_t irq_flag = 0x00;
volatile uint8_t rx_error_flag = 0x00;
volatile uint8_t rxtx_state = 1;							// used for transmit recieve byte count
volatile uint8_t host_control_flag = 0;
volatile uint8_t stand_alone_flag = 1;
extern uint8_t UID[8];

static uint8_t UIDlast[8];

static uint8_t cardData[32] = {0x44, 0x49, 0x48 ,0x4D}; // "DIHM"

volatile bool syncTriggered = false;
volatile bool responseTriggered = false;
volatile CAN_ODENTRY syncEntry;

volatile bool Connected = true;
volatile bool Registered = false;		// Registered by host
volatile bool ForceSync = false;
volatile bool Gotcha = true;
volatile bool RfidTimeup = true;	// for power saving

uint8_t Command, InterruptStatus;
uint32_t ProxiValue, AmbiValue;

bool VcnlConnected = false;

volatile uint8_t LED_CONTROL = 0x00;

void Setup()
{
	memset((void *)(MemBuffer), 0, MEM_BUFSIZE);
	memset(UIDlast, 0, 8);
	
	syncEntry.entrytype_len = 9;
	syncEntry.index = SYNC_RFID;
	syncEntry.subindex = 0;
	syncEntry.val = MemBuffer;
	
	uint8_t id;
 	id = (LPC_GPIO[PORT1]->MASKED_ACCESS[0x007]);
	id |= (LPC_GPIO[PORT1]->MASKED_ACCESS[0x0F0])>>1;
	NodeId = 0x100 | id;
	
	GPIOSetDir(R1_LED, E_IO_OUTPUT);	
	GPIOSetDir(G1_LED, E_IO_OUTPUT);	
	GPIOSetDir(R2_LED, E_IO_OUTPUT);
	GPIOSetDir(G2_LED, E_IO_OUTPUT);
	
	GPIOSetValue(R1_LED, 0);
	GPIOSetValue(G1_LED, 0);
	GPIOSetValue(R2_LED, 0);
	GPIOSetValue(G2_LED, 0);
	
	GPIOSetDir(POS_LED, E_IO_OUTPUT);
	GPIOSetValue(POS_LED, 0);
	
	GPIOSetDir(VCNL_INT, E_IO_INPUT);
	GPIOSetInterrupt(VCNL_INT, 0, 0, 1);
	//GPIOIntEnable(VCNL_INT);
	
	VCNL40x0::Init();
	DELAY(10000); //wait for I2C
	VCNL40x0::ReadID (&id);
	if (id != VCNL40x0_PRODUCT)
		return;
	
	VcnlConnected = true;
	
	VCNL40x0::SetCurrent (10); // Set current to 100mA
	//uint8_t Current;
	//VCNL40x0::ReadCurrent (&Current); // Read back IR LED current
	//pc.printf("\n IR LED Current: %d", Current);
	// stop all activities (necessary for changing proximity rate, see datasheet)
	VCNL40x0::SetCommandRegister (COMMAND_ALL_DISABLE);
	// set proximity rate to 31/s
	VCNL40x0::SetProximityRate (PROX_MEASUREMENT_RATE_31);
	// enable prox and ambi in selftimed mode
	VCNL40x0::SetCommandRegister (COMMAND_PROX_ENABLE | COMMAND_AMBI_ENABLE | COMMAND_SELFTIMED_MODE_ENABLE);
	// set interrupt control for threshold
	VCNL40x0::SetInterruptControl (INTERRUPT_ALS_READY_ENABLE | INTERRUPT_PROX_READY_ENABLE);// | INTERRUPT_THRES_SEL_PROX | INTERRUPT_THRES_ENABLE | INTERRUPT_COUNT_EXCEED_1);
	// set ambient light measurement parameter
	VCNL40x0::SetAmbiConfiguration (AMBI_PARA_AVERAGE_16 | AMBI_PARA_AUTO_OFFSET_ENABLE | AMBI_PARA_MEAS_RATE_4 | AMBI_PARA_CONT_CONV_ENABLE);
	
	// measure average of prox value
//	uint32_t AverageProxiValue;
//	uint32_t SummeProxiValue = 0;
//	for (int i=0; i<30; i++) 
//	{
//		do { // wait on prox data ready bit
//			VCNL40x0::ReadCommandRegister (&Command); // read command register
//		} while (!(Command & COMMAND_MASK_PROX_DATA_READY)); // prox data ready ?
//		VCNL40x0::ReadProxiValue (&ProxiValue); // read prox value
//		SummeProxiValue += ProxiValue; // Summary of all measured prox values
//	}
//	AverageProxiValue = SummeProxiValue/30; // calculate average
//	VCNL40x0::SetHighThreshold (AverageProxiValue+100); // set upper threshold for interrupt
}

#define INDEX_DATA 0x100
#define INDEX_PRESID 9

void GetPresId()
{
	std::uint8_t len = 0;
	std::uint8_t *p = MemBuffer+INDEX_DATA;
	while (len<20 && p[len]!=0)
		++len;
	MemBuffer[INDEX_PRESID] = len;
	memcpy(MemBuffer+INDEX_PRESID+1, p, len);
	syncEntry.entrytype_len = 10+len;
}

bool UpdateRfid()
{	
	static uint8_t UIDlast[8];
	//static uint8_t found = 0;
	static uint32_t lostCount = 0;
	
	bool result = false;
	uint8_t suc = 0;
	uint8_t failCount;
	
	Trf796xTurnRfOn();
	DELAY(2000);
	if (Iso15693FindTag())
	{
		lostCount = 0;
		if (memcmp(UID, UIDlast, 8) != 0)
		{
			result = true;
			memcpy(UIDlast, UID, 8);
			memcpy(MemBuffer+1, UID, 8);

			failCount = 0;
			do
			{
				suc=Iso15693ReadSingleBlockWithAddress(0, UID, cardData+4);
				DELAY(1000);
			} while (suc && ++failCount<10);
			
			if (memcmp(cardData+4, cardData, 4)==0)
			{
				MemBuffer[0]= 0x02;
				failCount = 0;
				do
				{
					suc = Iso15693ReadMultipleBlockWithAddress(1, 27, UID, (uint8_t *)MemBuffer+INDEX_DATA);
					if (suc == 0)
						GetPresId();
					else
						DELAY(1000);
				} while (suc && ++failCount<10);
			}
			else
			{
				syncEntry.entrytype_len = 9;
				if (failCount>=10)
				{
					memset(MemBuffer, 0, 9);
					memset(UIDlast, 0, 8);
				}
				else
				{
					MemBuffer[0] = 1;
					memcpy(MemBuffer+1, UID, 8);
				}
			}
		}
	}
	else
	{
		if (++lostCount>RFID_TIME_COUNT)
		{
			lostCount = RFID_TIME_COUNT;
			if (MemBuffer[0] != 0)
			{
				result = true;
				syncEntry.entrytype_len = 9;
				memset(MemBuffer, 0, 9);
				memset(UIDlast, 0, 8);
			}
		}
	}
	Trf796xTurnRfOff();
	DELAY(2000);
	return result;
}

struct CanResponse
{
	uint16_t sourceId;
	CAN_ODENTRY response;
	uint8_t result;
};

volatile CanResponse res;

extern "C"
{
	void LedUpdate()
	{
		if (LED_CONTROL & 0x08)
		{
			if (LED_CONTROL & 0x01)	//Green1
			{
				GPIOSetValue(R1_LED, 0);
				if (LED_CONTROL & 0x02)
					GPIOSetValue(G1_LED, GPIOGetValue(G1_LED)==0);
				else
					GPIOSetValue(G1_LED, 1);
			}
			else	//Red1
			{
				GPIOSetValue(G1_LED, 0);
				if (LED_CONTROL & 0x02)
					GPIOSetValue(R1_LED, GPIOGetValue(R1_LED)==0);
				else
					GPIOSetValue(R1_LED, 1);
			}
		}
		else
		{
			GPIOSetValue(R1_LED, 0);
			GPIOSetValue(G1_LED, 0);
		}
		
		if (LED_CONTROL & 0x80)
		{
			if (LED_CONTROL & 0x10)	//Green2
			{
				GPIOSetValue(R2_LED, 0);
				if (LED_CONTROL & 0x20)
					GPIOSetValue(G2_LED, GPIOGetValue(G2_LED)==0);
				else
					GPIOSetValue(G2_LED, 1);
			}
			else	//Red1
			{
				GPIOSetValue(G2_LED, 0);
				if (LED_CONTROL & 0x20)
					GPIOSetValue(R2_LED, GPIOGetValue(R2_LED)==0);
				else
					GPIOSetValue(R2_LED, 1);
			}
		}
		else
		{
			GPIOSetValue(R2_LED, 0);
			GPIOSetValue(G2_LED, 0);
		}
	}
	
	void CanexReceived(uint16_t sourceId, CAN_ODENTRY *entry)
	{
		CAN_ODENTRY *response=const_cast<CAN_ODENTRY *>(&(res.response));

		res.sourceId = sourceId;
		res.result=0xff;
		
		response->val = const_cast<uint8_t *>(&(res.result));
		response->index = entry->index;
		response->subindex = entry->subindex;
		response->entrytype_len = 1;

		switch (entry->index)
		{
			case 0:	//system config
				break;
			case OP_SET_INDICATOR:
				if (entry->entrytype_len<1)
						break;
				LED_CONTROL = entry->val[0];
				LedUpdate();
				*(response->val) = 0;
				break;
			default:
				break;
		}
//		CANEXResponse(sourceId, response);
		responseTriggered = true;
	}

	void CanexSyncTrigger(uint16_t index, uint8_t mode)
	{	
		if (mode!=0)
			return;

		switch(index)
		{
			case SYNC_GOTCHA:
				Gotcha = true;
				break;
			case SYNC_RFID:
				ForceSync = true;
				break;
			case SYNC_LIVE:
				Connected=!Connected;
				if (Connected)
					Registered = true;
				break;
			default:
				break;
		}
	}
	
	volatile uint32_t hbCounter = 0;

	void TIMER32_0_IRQHandler()
	{
		
		static uint32_t counter2 = 0;
		static uint32_t counter3 = 0;
		
		if ( LPC_TMR32B0->IR & 0x01 )
		{
			LPC_TMR32B0->IR = 1;			/* clear interrupt flag */
			
			if (!RfidTimeup)
			{
				if (counter2++ >= RFID_TIME_INTERVAL)
				{
					RfidTimeup =true;
					counter2 = 0;
				}				
			}
			
			if (counter3++ >= LED_INTERVAL)
			{
				counter3=0;
				LedUpdate();
			}
		
				if (Connected)
				{
					if (Registered && !Gotcha)
					{
						if (hbCounter++>=HeartbeatInterval)
						{
							hbCounter = 0;
							syncTriggered = true;
						}
					}
				}
				else
				{
					if (hbCounter++>=HeartbeatInterval)
					{
						hbCounter = 0;
						CANEXHeartbeat(STATE_OPERATIONAL);
					}
				}
		}
	}
	
	void PIOINT2_IRQHandler(void)
	{
		//uint8_t status;
		if (GPIOIntStatus(VCNL_INT))
		{
			GPIOIntClear(VCNL_INT);
		}
	}
}

void UpdateIndicator()
{
	// read interrupt status register
			VCNL40x0::ReadInterruptStatus (&InterruptStatus);
	//		// check interrupt status for High Threshold
	//		if (InterruptStatus & INTERRUPT_MASK_STATUS_THRES_HI) 
	//		{
	//			GPIOSetValue(POS_LED, 1);
	//			//mled2 = 1; // LED on, Interrupt
	//			VCNL40x0::SetInterruptStatus (InterruptStatus); // clear Interrupt Status
	//			//mled2 = 0; // LED off, Interrupt
	//		}
	//		else
	//			GPIOSetValue(POS_LED, 0);
			// prox value ready for using
			if (InterruptStatus & INTERRUPT_STATUS_PROX_READY) 
			{
				VCNL40x0::ReadProxiValue (&ProxiValue); // read prox value
				VCNL40x0::SetInterruptStatus (INTERRUPT_STATUS_PROX_READY);
			}
			// ambi value ready for using
			if (InterruptStatus & INTERRUPT_STATUS_ALS_READY) 
			{
				VCNL40x0::ReadAmbiValue (&AmbiValue); // read ambi value
				VCNL40x0::SetInterruptStatus (INTERRUPT_STATUS_ALS_READY);
			}
			
			GPIOSetValue(POS_LED, AmbiValue<100 && ProxiValue>10000);
}

int main()
{
	SystemCoreClockUpdate();
	GPIOInit();
	Setup();
	
	//Test codes
//	LED_CONTROL = 0xB8;
//	LedUpdate();
	
	CANInit(500);
	CANEXReceiverEvent = CanexReceived;
	CANTEXTriggerSyncEvent = CanexSyncTrigger;
	
	Trf796xCommunicationSetup();
	Trf796xInitialSettings();
	
	init_timer32(0, TIME_INTERVAL(1000));		//	1000Hz
	DELAY(10);
	enable_timer32(0);
	
	DELAY(1000);
	
	while(1)
	{
		if (RfidTimeup)
		{
			RfidTimeup = false;
			bool updated = UpdateRfid();
			if (Registered && (updated || ForceSync))
			{
				ForceSync = false;
				Gotcha = false;
				hbCounter = HeartbeatInterval;	//Send at once
			}
		}
		
		if (VcnlConnected)
			UpdateIndicator();
		
		if (responseTriggered)
		{
			CANEXResponse(res.sourceId, const_cast<CAN_ODENTRY *>(&(res.response)));
			responseTriggered = false;
		}
		
		if (syncTriggered)
		{
			CANEXBroadcast(const_cast<CAN_ODENTRY *>(&syncEntry));
			syncTriggered = false;
		}
	}
}
