#ifndef _CONFIG_COMM_H
#define _CONFIG_COMM_H

#include "ISerialComm.h"
#include "FastDelegate.h"
#include "fram.h"

#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>
#include <Driver_USART.h>

#define CONFIG_BUF_SIZE 0x400

using namespace fastdelegate;

class ConfigComm : public ISerialComm
{
	protected:
		static ConfigComm *comm;
		static void UARTCallback(uint32_t event);
		const ARM_DRIVER_USART &uart;
		static const std::uint8_t dataHeader[3];
	private:
		
		enum StateType
		{
			StateDelimiter1,
			StateDelimiter2,
			StateCommand,
			StateParameterLength,
			StateParameters,
		};
		
		uint8_t buffer[CONFIG_BUF_SIZE];
		uint32_t head;
		volatile uint32_t tail;
		uint8_t processRound;
		volatile uint8_t readRound;
		
		StateType dataState;
		uint8_t command;
		uint16_t parameterLen;
		uint16_t parameterIndex;
		boost::shared_ptr<uint8_t[]> parameters;
	public:
		typedef FastDelegate3<std::uint8_t, std::uint8_t *, std::size_t> CommandArrivalHandler;
		CommandArrivalHandler OnCommandArrivalEvent;
		ConfigComm(ARM_DRIVER_USART &u);
		virtual ~ConfigComm();
		virtual void Start();
		virtual void Stop();
		virtual void DataReceiver();
		virtual bool SendData(const std::uint8_t *data,std::size_t len);
		virtual bool SendData(std::uint8_t command, const std::uint8_t *data,std::size_t len);
};

#endif

