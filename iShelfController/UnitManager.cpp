#include "UnitManager.h"

using namespace std;

namespace IntelliShelf
{
	void UnitManager::UpdateLatest(boost::shared_ptr<ShelfUnit> &unit)
	{
		string cardId = unit->GetCardId();
		for (UnitIterator it = unitList.begin(); it != unitList.end(); ++it)
		{
			if (it->second->IsEmpty() || it->second==unit)
				continue;
			if (it->second->IsLatest() && cardId == it->second->GetCardId())
			{
				it->second->latest = false;
				it->second->IndicatorOff();
			}
		}
	}
	
	boost::shared_ptr<ShelfUnit> UnitManager::FindUnit(const std::string &cardId)
	{
		boost::shared_ptr<ShelfUnit> unit;
		for (UnitIterator it = unitList.begin(); it != unitList.end(); ++it)
		{
			if (it->second->IsEmpty())
				continue;
			if (it->second->IsLatest() && cardId == it->second->GetCardId())
			{
				unit = it->second;
				break;
			}
		}
		return unit;
	}
	
	boost::shared_ptr<ShelfUnit> UnitManager::FindUnit(std::uint16_t id)
	{
		boost::shared_ptr<ShelfUnit> unit;
		UnitIterator it = unitList.find(id);
		if (it != unitList.end())
			unit = it->second;
		return unit;
	}
	
	void UnitManager::Add(std::uint16_t id, boost::shared_ptr<ShelfUnit> unit) 
	{
		unitList[id] = unit; 
	}
	
	bool UnitManager::SetLEDState(const std::string &cardId, std::uint8_t dir, std::uint8_t color, std::uint8_t status)
	{
		boost::shared_ptr<ShelfUnit> unit = FindUnit(cardId);
		if (unit.get() == NULL)
			return false;
		boost::shared_ptr<CardListItem> item;
		CardIterator it = cardList.find(cardId);
		if (it == cardList.end())
		{
			item.reset(new CardListItem);
			cardList[cardId] = item;
		}
		else
			item = it->second;
		unit->SetIndicator(dir, color, status);
		item->Direction = dir;
		item->IndicatorData = unit->GetIndicator();
		return true;
	}
	
	bool UnitManager::SetLEDState(const std::string &cardId, std::uint8_t color, std::uint8_t status)
	{
		boost::shared_ptr<ShelfUnit> unit = FindUnit(cardId);
		if (unit.get() == NULL)
			return false;
		CardIterator it = cardList.find(cardId);
		if (it == cardList.end())
			return false;
		boost::shared_ptr<CardListItem> item = it->second;
		unit->SetIndicator(item->Direction, color, status);
		item->IndicatorData = unit->GetIndicator();
		return true;
	}
	
	bool UnitManager::ClearLEDState(const std::string &cardId)
	{
		//Set clear state in cardList for this card ID
		boost::shared_ptr<CardListItem> item;
		CardIterator it = cardList.find(cardId);
		if (it == cardList.end())
		{
			item.reset(new CardListItem);
			cardList[cardId] = item;
		}
		else
			item = it->second;
		item->Direction = IndicatorDirection::NA;
		item->IndicatorData = 0;
		
		//Find unit and turn off indicator
		boost::shared_ptr<ShelfUnit> unit = FindUnit(cardId);
		if (unit.get() == NULL)
			return false;
		unit->IndicatorOff();
		return true;
	}
	
	bool UnitManager::RemoveLEDState(const std::string &cardId, bool keepShown)
	{
		cardList.erase(cardId);
		if (keepShown)
			return true;
		boost::shared_ptr<ShelfUnit> unit = FindUnit(cardId);
		if (unit.get() == NULL)
			return false;
		unit->IndicatorOff();
		return true;
	}
	
	bool UnitManager::GetLEDState(const std::string &cardId, std::uint8_t &data)
	{
		CardIterator it = cardList.find(cardId);
		if (it == cardList.end())
			return false;
		data = it->second->IndicatorData;
		return true;
	}
	
	void UnitManager::RemoveSide(std::uint8_t dir)
	{
		vector<string> list;
		for(CardIterator it=cardList.begin(); it!=cardList.end(); ++it)
		{
			if (it->second->Direction == dir)
				list.push_back(it->first);
		}
		for(vector<string>::iterator it=list.begin(); it!=list.end(); ++it)
			RemoveLEDState(*it, false);
	}
	
	inline void UnitManager::RemoveAllLEDState()
	{
		cardList.clear();
	}
}
