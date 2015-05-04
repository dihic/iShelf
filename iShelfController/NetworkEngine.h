#ifndef _NETWORK_ENGINE_H
#define _NETWORK_ENGINE_H

#include "TcpClient.h"
#include "CommStructures.h"
#include "UnitManager.h"
#include <boost/type_traits.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>

namespace IntelliShelf
{
	class NetworkEngine
	{
		private:
			TcpClient tcp;
			UnitManager &manager;
			void TcpClientCommandArrival(boost::shared_ptr<std::uint8_t[]> payload, std::size_t size);
			void WhoAmI();
		public:

			NetworkEngine(const std::uint8_t *endpoint, UnitManager &um);
			~NetworkEngine() {}
			void SendHeartBeat();
			void SendRequest(boost::shared_ptr<ShelfUnit> unit, const std::string &cardId);
			void InventoryRfid();
			void Process();
			void Connection();
			void ChangeServiceEndpoint(const std::uint8_t *endpoint)
			{
				tcp.ChangeServiceEndpoint(endpoint);
			}
			
			void DeviceReadResponse(CanDevice &device,std::uint16_t attr, const boost::shared_ptr<std::uint8_t[]> &data, std::size_t size);
			void DeviceWriteResponse(CanDevice &device,std::uint16_t attr, bool result);
	};
}

#endif
