#ifndef _UNIT_MANAGER_H
#define _UNIT_MANAGER_H

#include <map>
#include <cstdint>
#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>
#include "ShelfUnit.h"
#include "FastDelegate.h"

using namespace fastdelegate;

namespace IntelliShelf
{
	class CardListItem
  {
		public:
			CardListItem() {}
			~CardListItem() {}
      std::uint8_t Direction;
			std::uint8_t IndicatorData;
	};
	
	class UnitManager
	{
		private:
			std::map<std::uint16_t, boost::shared_ptr<ShelfUnit> > unitList;
			std::map<std::string, boost::shared_ptr<CardListItem> > cardList;
			
		public:
			typedef std::map<std::uint16_t, boost::shared_ptr<ShelfUnit> >::iterator UnitIterator;
			typedef std::map<std::string, boost::shared_ptr<CardListItem> >::iterator CardIterator;
//			typedef FastDelegate1<boost::shared_ptr<StorageUnit> > SendDataCB;
//			SendDataCB OnSendData;
			UnitManager() {}
			~UnitManager() {}
			void Add(std::uint16_t id, boost::shared_ptr<ShelfUnit> unit);
			std::map<std::uint16_t, boost::shared_ptr<ShelfUnit> > &GetList() { return unitList; }
			
			boost::shared_ptr<ShelfUnit> FindUnit(std::string cardId);
			boost::shared_ptr<ShelfUnit> FindUnit(std::uint16_t id);
			
			bool SetLEDState(std::string cardId, std::uint8_t dir, std::uint8_t color, std::uint8_t status);
			bool SetLEDState(std::string cardId, std::uint8_t color, std::uint8_t status);
			
			bool ClearLEDState(std::string cardId);
			bool RemoveLEDState(std::string cardId, bool keepShown);
			void RemoveAllLEDState();
			bool GetLEDState(std::string cardId, std::uint8_t &data);
			void RemoveSide(std::uint8_t dir);
	};
	
}

#endif
