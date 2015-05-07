#include "ShelfUnit.h"
#include "System.h"
#include "stm32f4xx_hal_conf.h" 
#include <cstring>

using namespace std;

namespace IntelliShelf
{

	ShelfUnit::ShelfUnit(CANExtended::CanEx &ex, uint16_t id)
		:	CanDevice(ex, id), lastCardType(0), cardChanged(false), processing(false)
	{
		cardState = 0;
		cardId.clear();
		presId.clear();
	}

	void ShelfUnit::UpdateCard()
	{
		cardChanged = false;
		if (cardState == CardLeft)
		{
			cardId.clear();
			presId.clear();
		}
	}
	
	void ShelfUnit::GenerateId(const uint8_t *id, size_t len, string &result)
	{
		char temp;
		result.clear();
		for(int i=len-1;i>=0;--i)
		{
			temp = (id[i] & 0xf0) >>4;
			temp += (temp>9 ? 55 : 48);
			result+=temp;
			temp = id[i] & 0x0f;
			temp += (temp>9 ? 55 : 48);
			result+=temp;
		}
	}
	
	void ShelfUnit::SetIndicator(std::uint8_t data)
	{
		ledState = data;
		boost::shared_ptr<std::uint8_t[]> buf = boost::make_shared<std::uint8_t[]>(1);
		buf[0] = data;
		//Wait for last command completed
		while(IsBusy())
			__nop();
		WriteAttribute(DeviceAttribute::Indicator, buf, 1); 
	}

	
	void ShelfUnit::SetIndicator(std::uint8_t dir, std::uint8_t color, std::uint8_t status)
	{
		uint8_t led;
		if (dir == IndicatorDirection::Both)
			led = ((color | status) << 4) | ( color | status);
		else
			led = (color | status) << dir;
		SetIndicator(led);
	}
	
	void ShelfUnit::IndicatorOff()
	{
		boost::shared_ptr<std::uint8_t[]> buf = boost::make_shared<std::uint8_t[]>(1);
		buf[0] = ledState = 0;
		WriteAttribute(DeviceAttribute::Indicator, buf, 1); 
	}

	void ShelfUnit::ProcessRecievedEvent(boost::shared_ptr<CANExtended::OdEntry> entry)
	{
		CanDevice::ProcessRecievedEvent(entry);
		//Notice device that host has got the process data
		canex.Sync(DeviceId, SYNC_GOTCHA, CANExtended::Trigger);
		std::uint8_t *rawData = entry->GetVal().get();
		string id;
		if (rawData[0]!=0)
			GenerateId(rawData+1, 8, id);	//Generate temporary rfid id when got one
		if (lastCardType==rawData[0] && cardId==id) //identical
			return;
		processing = true;
		lastCardType = rawData[0];
		switch (rawData[0])
		{
			case 0:
				cardState = CardLeft;
				latest = false;
				break;
			case 1:
				cardState = CardArrival;
				latest = true;
				cardId = id;
				presId.clear();
				break;
			case 2:
				cardState = CardArrival;
				latest = true;
				cardId = id;
				presId.clear();
				presId.append(reinterpret_cast<char *>(rawData+10), rawData[9]);
				break;
			default:
				break;
		}
		cardChanged = true;
		processing = false;
	}
}

