#ifndef _SHELF_UNIT_H
#define _SHELF_UNIT_H

#include <string>
#include "Guid.h"
#include "CanDevice.h"

namespace IntelliShelf
{
#define SYNC_DATA				0x0100
#define SYNC_GOTCHA			0x0180
#define SYNC_LIVE				0x01ff
	
	struct IndicatorDirection
	{
		static const std::uint8_t Left = 0;
		static const std::uint8_t Right = 4;
		static const std::uint8_t Both = 8;
		static const std::uint8_t NA = 0xff;
	};

  struct IndicatorColor
	{
		static const std::uint8_t Red = 0;
		static const std::uint8_t Green = 1;
	};

	struct IndicatorState
	{
		static const std::uint8_t	Blink = 0x2;
		static const std::uint8_t LightOn = 0x8;
	};

	enum RfidType
	{
		RfidUnknown = 0,
		RfidIso15693 = 0x01,
		RfidIso14443A = 0x02,
		RfidIso14443B = 0x03,
	};

	enum RfidStatus
	{
		RfidNone = 0,
    RfidExpanded = 1,
    RfidAvailable = 2,
		RfidNA = 0xff
	};
	
	enum RfidAction
	{
		CardLeave,
		CardArrival,
	};
	
	struct DeviceAttribute
	{
		static const std::uint16_t RawData 				= 0x8006;   //R
		static const std::uint16_t Indicator 			= 0x8002;   //W
	};
	
	class ShelfUnit : public CanDevice
	{
		private:
			static void GenerateId(const uint8_t *id, size_t len, string &result);
			uint8_t lastCardType;
			volatile bool cardChanged;
			volatile std::uint8_t cardState;
			volatile std::uint8_t ledState;
			volatile bool processing;
			std::string cardId;
			std::string presId;
		public:
			static const std::uint8_t CardArrival = 0x80;
			static const std::uint8_t CardLeft    = 0x81;
		
			ShelfUnit(CANExtended::CanEx &ex, std::uint16_t id);
			virtual ~ShelfUnit() {}
			
			void WaitProcessing() const 
			{ 
				while (processing)
					__nop(); 
			}
			std::uint8_t GetIndicator() const { return ledState; }
			void SetIndicator(std::uint8_t data);
			void SetIndicator(std::uint8_t dir, std::uint8_t color, std::uint8_t status);
			void IndicatorOff();
			
			void RequestRawData()
			{
				ReadAttribute(DeviceAttribute::RawData);
			}
			
			bool CardChanged() const { return cardChanged; }
			bool IsEmpty() const { return cardState == CardLeft; }
			
			void UpdateCard();
			
			uint8_t GetCardState() const { return cardState; }
			const std::string &GetPresId() { return presId; }
			const std::string &GetCardId() { return cardId; }
			
			void RequestData()
			{
				canex.Sync(DeviceId, SYNC_DATA, CANExtended::Trigger);
			}
			
			virtual void ProcessRecievedEvent(boost::shared_ptr<CANExtended::OdEntry> entry);
	};
}

#endif

