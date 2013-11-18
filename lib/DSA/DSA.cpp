/**
 * \file DSA.cpp
 * \brief Library for sending and receiving commands using the DSA protocol.
 *
 * The high-end Compact Disc modules made by daisy-laser http://www.daisy-laser.com
 * are controlled by the DSA protocol. A three wire interface using a data- (DSA),
 * strobe- (STB) and acknowledge (ACK) line.
 * More info on the DSA protocol on http://www.daisy-laser.com/technology/techdsa/techdsa.htm
 *
 * \author Created by Martin van den Berg, November, 2013
 *
 * \copyright Copyright (c) 2013, Martin van den Berg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * \li Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * \li Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * \li Neither the name of the {organization} nor the names of its contributors
 * may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Arduino.h"
#include "DSA.h"

/**
 * \fn DSA::DSA(int DSApin, int STBpin, int ACKpin)
 * \brief Constructor
 *
 * \param DSApin Pin to be used as "data" pin
 * \param STBpin Pin to be used as "strobe" pin
 * \param ACKpin Pin to be used as "acknowledge" pin
 */
DSA::DSA(int DSApin, int STBpin, int ACKpin)
{
	_DSApin = DSApin;
	_STBpin = STBpin;
	_ACKpin = ACKpin;
	
	resetPins();
}

/**
 * \fn bool DSA::transmitRequested()
 * \brief Checks if there is a transmit request from the other side.
 *
 * \return True on request to transmit, False on no request
 */
bool DSA::transmitRequested()
{
	return !digitalRead(_DSApin);
}

/**
 * \fn bool DSA::waitForMessage(byte *opcode, byte *parameter)
 * \deprecated Try to avoid using this function. It will block your
 * program for the timeout time when no message is received.
 * Better keep checking transmitRequested() in you main loop.
 * \brief Wait for a message to receive.
 *
 * When a message is expected this function will wait for the message to arrive.
 * A timeout is set to abort this function at timeout. 
 * \param *opcode Pointer to the opcode variable that will updated after a successful transfer
 * \param *parameter Pointer to the parameter variable that will updated after a successful transfer.
 * \return True on a successful transfer, False on a timeout or communication error
 */
bool DSA::waitForMessage(byte *opcode, byte *parameter)
{
	startTimeoutTimer(DSA_TIMEOUT);
	while(!transmitRequested()) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	return receiveMessage(opcode, parameter);
}

/**
 * \fn bool DSA::receiveMessage(byte *opcode, byte *parameter)
 * \brief Receive a message.
 *
 * \param *opcode Pointer to the opcode variable that will updated after a successful transfer
 * \param *parameter Pointer to the parameter variable that will updated after a successful transfer.
 * \return True on a successful transfer, False on a communication error
 */
bool DSA::receiveMessage(byte *opcode, byte *parameter)
{
	/* synchPhase */
	word command = 0;
	digitalWrite(_ACKpin, HIGH);
	resetPins();
	pinMode(_ACKpin, OUTPUT);
	startTimeoutTimer(DSA_TIMEOUT);
	digitalWrite(_ACKpin, LOW);

	while(!digitalRead(_DSApin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	digitalWrite(_ACKpin, HIGH);
	
	/* data transmission phase */
	startTimeoutTimer(DSA_TIMEOUT);
	unsigned int mask = 0x8000;
	int i;
	for (i = 0; i < 16; i++ ) {
		while(digitalRead(_STBpin)) {
			if(timeOut()) {
				resetPins();
				return false;
			}
		}

		if (digitalRead(_DSApin))
			command = command | mask;

		digitalWrite(_ACKpin, LOW);

		while(!digitalRead(_STBpin)) {
			if(timeOut()) {
				resetPins();
				return false;
			}
		}
		digitalWrite(_ACKpin, HIGH);
		mask = mask >> 1;
	}

	/* acknowledge phase */
	pinMode(_STBpin, OUTPUT);
	pinMode(_DSApin, OUTPUT);
	pinMode(_ACKpin, INPUT);

	startTimeoutTimer(DSA_TIMEOUT);

	while(digitalRead(_ACKpin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	
	if(i != 16)
		digitalWrite(_DSApin, LOW);

	digitalWrite(_STBpin, LOW);

	while(!digitalRead(_ACKpin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	digitalWrite(_DSApin, HIGH);
	digitalWrite(_STBpin, HIGH);
	
	*parameter	= lowByte(command);
	*opcode = highByte(command);

	resetPins();
	return true;
}

/**
 * \fn bool DSA::sendMessage(byte opcode)
 * \brief Send a message with empty parameter.
 *
 * A DSA message contains a opcode and a parameter send as one 16bit command.
 * When no parameter is defined a empty (0) parameter value is send.
 * \param opcode The DSA opcode to send.
 * \return True on a successful transfer, False on a communication error
 */
bool DSA::sendMessage(byte opcode)
{
	return sendMessage(opcode, 0);
}

/**
 * \fn bool DSA::sendMessage(byte opcode, byte param)
 * \brief Send a message.
 *
 * A DSA message contains a opcode and a parameter send as one 16bit command.
 * \param opcode The DSA opcode to send.
 * \param parameter The DSA parameter to send.
 * \return True on a successful transfer, False on a communication error
 */
bool DSA::sendMessage(byte opcode, byte parameter)
{
	/* synchPhase */
	int command = opcode;
	command = command << 8;
	command += parameter;

	digitalWrite(_DSApin, HIGH);
	resetPins();														
	pinMode(_DSApin, OUTPUT);

	startTimeoutTimer(DSA_TIMEOUT);
	digitalWrite(_DSApin, LOW);
	
	while(digitalRead(_ACKpin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	digitalWrite(_DSApin, HIGH);

	while(!digitalRead(_ACKpin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	/* data transfer phase */
	digitalWrite(_STBpin, HIGH);
	pinMode(_STBpin, OUTPUT);

	startTimeoutTimer(DSA_TIMEOUT);

	unsigned int mask = 0x8000;
	int i;
	for (i = 0; i < 16; i++ ) {
		if(!(command & mask))
			digitalWrite(_DSApin, LOW);
		digitalWrite(_STBpin, LOW);
		while(digitalRead(_ACKpin)) {
			if(timeOut()) {
				resetPins();
				return false;
			}
		}
		digitalWrite(_STBpin, HIGH);
		digitalWrite(_DSApin, HIGH);

		while(!digitalRead(_ACKpin)) {
			if(timeOut()) {
				resetPins();
				return false;
			}
		}
		mask = mask >> 1;
	}
	/* acknowledge phase */
	digitalWrite(_ACKpin, HIGH);
	resetPins();
	pinMode(_ACKpin, OUTPUT);

	startTimeoutTimer(DSA_TIMEOUT);

	digitalWrite(_ACKpin, LOW);
	while(digitalRead(_STBpin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	if(!digitalRead(_DSApin)) {
		resetPins();
		return false;
	}	
	digitalWrite(_ACKpin, HIGH);
	while(!digitalRead(_STBpin)) {
		if(timeOut()) {
			resetPins();
			return false;
		}
	}
	resetPins();
	return true;			
}

void DSA::startTimeoutTimer(int time) {
	_timeoutTime = time;
	_timeoutStartTime = millis();
}

bool DSA::timeOut() {
	if (millis() - _timeoutStartTime > _timeoutTime)
		return true;
	else
		return false;
}

void DSA::resetPins() {
	pinMode(_DSApin, INPUT);
	pinMode(_STBpin, INPUT);
	pinMode(_ACKpin, INPUT);
	// turn on pull-up resistors
	digitalWrite(_DSApin, HIGH);
	digitalWrite(_STBpin, HIGH);
	digitalWrite(_ACKpin, HIGH);
}
