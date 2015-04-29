#include "CommStructures.h"

namespace IntelliShelf
{
	CLAIM_CLASS(HeartBeat);
	CLAIM_CLASS(Basket);
	CLAIM_CLASS(BasketStatus);
	CLAIM_CLASS(InventoryRequest);
	CLAIM_CLASS(Inventory);
	CLAIM_CLASS(IndicatorTest);

	void CommStructures::Register()
	{
		REGISTER_CLASS(HeartBeat);
		REGISTER_CLASS(Basket);
		REGISTER_CLASS(BasketStatus);
		REGISTER_CLASS(InventoryRequest);
		REGISTER_CLASS(Inventory);
		REGISTER_CLASS(IndicatorTest);
	}
}
	
