/*
 * Arduino to Arduino using DSA
 * 
 * Example code to communicate between two Arduino board using the DSA protocol
 *
 * Copyright (c) 2013, Martin van den Berg, Bergrans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Bergrans nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL MARTIN VAN DEN BERG BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * -- HOWTO --
 * * Upload this sketch into two Arduino boards.
 * * Connect the tree DSA pins (a 47Ohm serial resistor is recommended to protect
 *   you Arduino) and GND between the two boards.
 * * Connect both Arduino boards to a computer.
 * * Open the "Serial Monitor" on both computers.
 * * Type some characters and hit Enter.
 * * If everything works fine the text should appear in the Serial Monitor of the other computer.
 *
 * Notice that characters are send in pairs (as DSA-opcode and DSA-parameter) so if an odd
 * number of characters is send the last character will not show up until new characters are send.
 */

#include <DSA.h>

byte opcode;		// Received opcode
byte parameter;	// Received parameter

DSA myDSA(2,3,4);	// Create a DSA controller and define the used pins

void setup() {
	Serial.begin(9600);
}

void loop(){
	if (Serial.available() >= 2) {
		opcode = Serial.read();
		parameter = Serial.read();
		if(!myDSA.sendMessage(opcode, parameter)) {
			Serial.println("send error");
		}
	}

	if(myDSA.transmitRequested()) {
		if(myDSA.receiveMessage(&opcode, &parameter)) {
			Serial.print(char(opcode));
			Serial.print(char(parameter));
		}
		else {
			Serial.println("receive error");
		}
	}
}
