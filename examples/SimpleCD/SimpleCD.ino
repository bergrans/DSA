/*
 * SimpleCD
 * 
 * Example code to control a CD-Pro2M/LF by sending serial commands
 *
 * A couple of DSA commands and return values are used to give you an
 * idea on how to control the CD-Pro unit. 
 * Find all valid DSA commands in the "DSA interface protocol & command set" 
 * http://www.daisy-laser.com/products/cd/modules/cdpro2/p10501_dsa.pdf
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
 *
 * -- HOWTO --
 * * Upload this sketch to your Arduino board
 * * Connect the CD-Pro2 unit DSA pins (1-Acknowledge, 2-Data, 3-Strobe, 5-GND)
 *   to the Arduino board (a 47Ohm serial resistor on the DSA lines is recommended).
 * * Open the Serial Monitor of the Arduino IDE.
 * * Type 't' and hit Enter.
 * * If everything works fine the CD-Pro will start spinning and returns some values.
 * * Type 'p' and hit Enter to start playing the first track.
 * 
 * Other valid commands are:
 * t = Read TOC
 * p = Play
 * h = Pause
 * s = Stop
 * > = next track
 * < = previous track
 */

#include <DSA.h>

byte opcode;		// Received opcode
byte parameter;	// Received parameter

// Pause state
bool paused;

// Returned values from CD-Pro2M/LF
int numberOfTracks;
int actualTitle;
int actualMinutes;
int actualSeconds;

// Create a DSA controller and define the used pins in order of "Data", "Strobe" and "Acknowledge" 
DSA myDSA(2,3,4);

void setup() {
	Serial.begin(9600);
	paused = false;
}

void loop(){
	if (Serial.available() > 0) {
		parameter = 0;
		bool validCommand = true;
		switch (Serial.read()) {
			case 'p':	//play
				opcode = 0x01;
				parameter = 1; // first track
				actualSeconds = 0;
				actualMinutes = 0;
				break;
			case 's':	//stop
				opcode = 0x02;
				break;
			case 't':	//toc read
				opcode = 0x03;
				break;
			case 'h':	//pause
				opcode = paused ? 0x05 : 0x04;
				break;
			case '>':	//next
				opcode = 0x01;
				parameter = actualTitle + 1;
				if (parameter > numberOfTracks)
					parameter = 1;
				break;
			case '<':	//previous
				opcode = 0x01;
				parameter = actualTitle - 1;
				if (parameter == 0)
					parameter = numberOfTracks;
				break;
			default: 
				validCommand = false;
				break;
		}
		if (validCommand) {
			Serial.println();
			Serial.print("Command ");
			Serial.print(char(opcode),HEX);
			Serial.print("h/");
			Serial.println(parameter);
			Serial.println();
						
			if(!myDSA.sendMessage(opcode, parameter)){
				Serial.println("! Send error");
			}
		}
	}

	if(myDSA.transmitRequested()) {
		if(myDSA.receiveMessage(&opcode, &parameter)) {
			switch (opcode) {
				case 0x01:	//Found
					switch (parameter) {
						case 0x41:	//Paused
						paused = true;
						break;
						case 0x42:	//Pause release
						paused = false;
						break;
					}
					break;
				case 0x02:	//Stopped
					paused = false;
					break;
				case 0x10:	//Actual title
					actualTitle = parameter;
					break;
				case 0x12:	//Actual minutes
					actualMinutes = parameter;
					break;
				case 0x13:	//Actual seconds
					actualSeconds = parameter;
					break;				
				case 0x21:	//Title maximum
					numberOfTracks = parameter;
					break;
				default: 
					break;
			}			
			Serial.print("CD-Pro  ");
			Serial.print(char(opcode),HEX);
			Serial.print("h/");
			Serial.println(parameter);
		}
		else {
			Serial.println("! Receive error");
		}
	}
}
