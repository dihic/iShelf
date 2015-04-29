#include "NetworkEngine.h"
#include "MemStream.h"
#include "Bson.h"
#include <ctime>
#include <cstring>

using namespace std;

namespace IntelliShelf
{	
	NetworkEngine::NetworkEngine(const std::uint8_t *endpoint, UnitManager &um)
		:tcp(endpoint), manager(um)
	{
		tcp.CommandArrivalEvent.bind(this, &NetworkEngine::TcpClientCommandArrival);
	}
	
	void NetworkEngine::SendHeartBeat()
	{
		static uint8_t HB[0x14]={0x14, 0x00, 0x00, 0x00, 0x12, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x00,
														 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		static uint64_t t = 0;
		++t;
		memcpy(HB+0x0b, &t, 8);
		tcp.SendData(HeartBeatCode, HB, 0x14);
	}
	
	void NetworkEngine::WhoAmI()
	{
		static const uint8_t ME[0x14]={0x14, 0x00, 0x00, 0x00, 0x12, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x00,
																	 0x01, 0x02, 0x01, 0x01, 			// Fixed 0x01, iShelf 0x02, PD 0x01, 0x01 Pharmacy Products
																	 0x00, 0x00, 0x00, 0x00, 0x00};
		tcp.SendData(HeartBeatCode, ME, 0x14);
	}
	
	void NetworkEngine::InventoryRfid()
	{
		static bool needReport = true;
		
		if (!tcp.IsConnected())
			needReport = true;
		
		uint8_t led;
		
		std::map<std::uint16_t, boost::shared_ptr<ShelfUnit> > unitList = manager.GetList();
		for (UnitManager::UnitIterator it = unitList.begin(); it!= unitList.end(); ++it)
		{
			switch (it->second->GetCardState())
			{
				case ShelfUnit::CardArrival:
					if (it->second->CardChanged())
					{
						if (manager.GetLEDState(it->second->GetCardId(), led))
							it->second->SetIndicator(led);
						else
						{
							if (!needReport)
								SendRequest(it->second);
							it->second->SetIndicator(IndicatorDirection::Both, IndicatorColor::Red, IndicatorState::LightOn);
						}
					}
					if (needReport)
						SendRequest(it->second);
					break;
				case ShelfUnit::CardLeft:
					if (it->second->CardChanged())
					{
#ifdef DEBUG_PRINT
						cout<<"Node "<<it->second->DeviceId<<" Card left"<<endl;
#endif
						it->second->IndicatorOff();
					}
					break;
				default:
					break;
			}
		}
		if (needReport && tcp.IsConnected())
			needReport = false;
	}
	
	void NetworkEngine::SendRequest(boost::shared_ptr<ShelfUnit> unit)
	{
		if (!tcp.IsConnected())
			return;
		if (unit.get() == NULL)
			return;
		size_t bufferSize = 0;
		boost::shared_ptr<Basket> basket(new Basket);
		basket->RFID = unit->GetCardId();
#ifdef DEBUG_PRINT
		cout<<"Node "<<unit->DeviceId<<" New card ID: "+basket->RFID<<endl;
#endif
		boost::shared_ptr<uint8_t[]> buffer = BSON::Bson::Serialize(basket, bufferSize);
		if (buffer.get()!=NULL && bufferSize>0)
			tcp.SendData(CommandBasketPlacement, buffer.get(), bufferSize);
	}
	
	void NetworkEngine::Process()
	{		
		TcpClient::TcpProcessor(&tcp);
	}
	
	void NetworkEngine::DeviceWriteResponse(CanDevice &device,std::uint16_t attr, bool result)
	{
//		boost::shared_ptr<CommandResult> response(new CommandResult);
//		response->NodeId = device.DeviceId;
//		response->Result = result;
//		
//		switch (attr)
//		{
//			case DeviceAttribute::Notice:
//				response->Command = RfidDataCode;
//				break;
//			default:
//				return;
//		}
//		
//		boost::shared_ptr<uint8_t[]> buffer;
//		size_t bufferSize = 0;
//		buffer = BSON::Bson::Serialize(response, bufferSize);
//		if (buffer.get()!=NULL && bufferSize>0)
//			tcp.SendData(CommandResponse, buffer.get(), bufferSize);
	}
	
	void NetworkEngine::DeviceReadResponse(CanDevice &device, std::uint16_t attr, const boost::shared_ptr<std::uint8_t[]> &data, std::size_t size)
	{
//		boost::shared_ptr<CommandResult> response;
//		boost::shared_ptr<uint8_t[]> buffer;
//		size_t bufferSize = 0;
//		
//		if (data.get()==NULL || size==0)
//		{
//			response.reset(new CommandResult);
//			response->NodeId = device.DeviceId;
//			response->Result = false;
//			switch (attr)
//			{
//				default:
//					return;
//			}
//			buffer = BSON::Bson::Serialize(response, bufferSize);
//			if (buffer.get()!=NULL && bufferSize>0)
//				tcp.SendData(CommandResponse, buffer.get(), bufferSize);
//			return;
//		}
		
//		switch (attr)
//		{
//			default:
//				break;
//		}
	}
	
	void NetworkEngine::TcpClientCommandArrival(boost::shared_ptr<std::uint8_t[]> payload, std::size_t size)
	{
		boost::shared_ptr<uint8_t[]> buffer;
		size_t bufferSize = 0;
		
		boost::shared_ptr<BasketStatus> 						status;
		boost::shared_ptr<InventoryRequest> 				request;
		boost::shared_ptr<Inventory>								ack;
		boost::shared_ptr<IndicatorTest>						test;
		
		CodeType code = (CodeType)payload[0];
		boost::shared_ptr<MemStream> stream(new MemStream(payload, size, 1));
		
		uint8_t side;
		int i;
		bool found;
		string cardIdString;
		
		std::map<std::uint16_t, boost::shared_ptr<ShelfUnit> > unitList;
		UnitManager::UnitIterator it;
		boost::shared_ptr<ShelfUnit> unit;
		
		//osMutexWait(mutex_id, osWaitForever);
		switch (code)
		{
			case CommandBasketPlacementAck:
				BSON::Bson::Deserialize(stream, status);
				if (status.get() == NULL)
					return;
				
				if (status->status == 0) //Corrected card
					manager.ClearLEDState(status->RFID);
				else
				{
					//Show warning or fail signal
					unit = manager.FindUnit(status->RFID);
					if (unit.get()!=NULL)
						//Red LED blinking on both sides
						unit->SetIndicator(IndicatorDirection::Both, IndicatorColor::Red,
															 IndicatorState::LightOn | IndicatorState::Blink);
				}
				break;
			case CommandInventoryRequest:
				BSON::Bson::Deserialize(stream, request);
				if (request.get() == NULL)
					break;
				side = request->position == 0
								? IndicatorDirection::Left
								: (request->position == 1
										? IndicatorDirection::Right
										: IndicatorDirection::NA);
			
				if (side != IndicatorDirection::NA)
					manager.RemoveSide(side);
				ack.reset(new Inventory);
				for (i = 0; i< request->list.Count(); ++i)
				{
					cardIdString = request->list[i];
					if (side == IndicatorDirection::NA) //If clear
						found = manager.RemoveLEDState(cardIdString, false);
					else
						//Green LED blinking on one side
						found = manager.SetLEDState(cardIdString, side, IndicatorColor::Green,
																					IndicatorState::LightOn | IndicatorState::Blink);
					status.reset(new BasketStatus);
					status->RFID = cardIdString;
					status->status = found ? 0 : 1;
					ack->list.Add(status);
				}
				buffer = BSON::Bson::Serialize(ack, bufferSize);
				if (buffer.get()!=NULL && bufferSize>0)
						tcp.SendData(CommandInventory, buffer.get(), bufferSize);
				break;
			case CommandTimeout:
				BSON::Bson::Deserialize(stream, request);
				if (request.get()==NULL)
					break;
				for (i = 0; i< request->list.Count(); ++i)
				{
					cardIdString = request->list[i];
					//Green LED on on same side
					manager.SetLEDState(cardIdString, IndicatorColor::Green, 
																				IndicatorState::LightOn);
					//Remove memory but keep LED shown no matter this basket on or off
					manager.RemoveLEDState(cardIdString, true);
				}
				break;
			case CommandIndicatorTest:
				BSON::Bson::Deserialize(stream, test);
				if (test.get()==NULL)
					break;
				unitList = manager.GetList();
				for(it = unitList.begin(); it!=unitList.end(); ++it)
				{
					if (test->test == 0)
						it->second->IndicatorOff();
					else if (test->test == 1)
						it->second->SetIndicator(IndicatorDirection::Both, IndicatorColor::Green, IndicatorState::LightOn);
					else if (test->test == 2)
						it->second->SetIndicator(IndicatorDirection::Both, IndicatorColor::Red, IndicatorState::LightOn);
				}
			default:
				break;
		}
	}
}

