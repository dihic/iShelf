#ifndef _COMM_STRUCTURES_H
#define _COMM_STRUCTURES_H

#include "TypeInfoBase.h"
#include <cstdint>
#include <string>

namespace IntelliShelf
{
	enum CodeType
	{
		HeartBeatCode 						= 1,          //class HeartBeat
		CommandBasketPlacement 		= 2,
    CommandBasketPlacementAck = 3,
    CommandInventoryRequest   = 4,
    CommandInventory					= 5,
    CommandTimeout						= 6,
		CommandIndicatorTest			= 7,
		CommandWhoAmI				 						= 0xff,		
	};
	
	DECLARE_CLASS(IndicatorTest)
	{
		public:
			IndicatorTest()
			{
				REGISTER_FIELD(test);
			}
			virtual ~IndicatorTest() {}
			int test;
	};

	DECLARE_CLASS(HeartBeat)
	{
		public:
			HeartBeat()
			{
				REGISTER_FIELD(times);
			}
			virtual ~HeartBeat() {}
			std::uint64_t times;
	};

	DECLARE_CLASS(Basket)
	{
		public:
			Basket()
			{
				REGISTER_FIELD(RFID);
			}
			virtual ~Basket() {}
			std::string RFID;
	};
	
	DECLARE_CLASS(BasketStatus)
	{
		public:
			BasketStatus()
			{
				REGISTER_FIELD(RFID);
				REGISTER_FIELD(status);
			}
			virtual ~BasketStatus() {}
			std::string RFID;
			int status;
	};

	DECLARE_CLASS(InventoryRequest)
	{
		public:
			InventoryRequest()
			{
				REGISTER_FIELD(list);
				REGISTER_FIELD(position);
			}
			virtual ~InventoryRequest() {}
			Array<std::string> list;
			int position;
	};

	DECLARE_CLASS(Inventory)
	{
		public:
			Inventory()
			{
				REGISTER_FIELD(list);
			}
			virtual ~Inventory() {}
			Array<BasketStatus> list;
	};
	
	class CommStructures
	{
		public:
			static void Register();
	};
}
#endif

