/*

  KeySwitches - an Arduino Library for creating a puzzle that requires 4 inputs to be activated within
                x mS of the first activated input.
				
  2016 Shawn Yates
  Want to automate this puzzle?  Visit our website to learn how:
  www.cluecontrol.com
  
  written and tested with Arduino 1.6.9
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

   -------------------
*/

/*
// pin assignments
 * Signal     Pin                
 *            Arduino Uno     
 * ------------------------
 * 0        Serial RX
 * 1        Serial TX
 * 2        Input 1
 * 3        Input 2
 * 4        SD crd SS if using Ethernet
 * 5        Input 3
 * 6        Input 4
 * 7        Input 5
 * 8        
 * 
 * 9        
 * 10         ChipSelect for Ethernet
 * 11         SPI MOSI for Ethernet
 * 12         SPI MISO for Ethernet
 * 13         SPI SCK  for Ethernet
 * 14  (A0)     
 * 15  (A1)     
 * 16  (A2)  
 
 
 The Ethernet Shield SPI bus (10,11,12,13) with 10 as select
 also uses 4 as select for the SD card on the ethernet shield
 high deselects for teh chip selects.  These will be reserved
 incase the unit is ever connected to ClueControl via the 
 ModBusTCP connection
 
 General operation:
 
 FirstSwitch will store the millis value present when the first switch is activated
 PuzzleSolved is a flag to indicate the new solved/unsolved state of the puzzle
 Voided is a flag to indicate the switches were not turned in time 
 

 Check each input
	If it has changed state to activated and voided is false,
		set its latch to true
		if FirstSwitch is 0, set it to current millis
		If all inputs are on and FirstSwitch is not 0, set PuzzleSolved
	
	If it has changed state to inactive
		Set its latch to false
		Make sure PuzzleSolved is false
		If all inputs are off, clear voided
		
In the main loop, if current millis - LastSwitch > limit 
		set LastSwitch = 0
		set Voided = true
		
 
 */

//Define the pin numbers

#define Input1      2
#define Input2      3
#define Input3      5
#define Input4      6
#define Input5      7

#define TimeLimit   1000 //  <-- Number of milliseconds players have to activate all inputs	
//	note that 1000 milliseconds = 1 second



// Global variables and objects
bool Input1Latch = false;
bool Input2Latch = false;
bool Input3Latch = false;
bool Input4Latch = false;
bool Input5Latch = false;

bool InputRead;
bool PuzzleSolved = false;
bool SolvedLatch = false;
bool Voided;

long FirstSwitch = 0;

void setup() {
	
	
	pinMode(Input1,INPUT_PULLUP);
	pinMode(Input2,INPUT_PULLUP);
	pinMode(Input3,INPUT_PULLUP);
	pinMode(Input4,INPUT_PULLUP);
	pinMode(Input5,INPUT_PULLUP);
	
	Serial.begin(9600);  // USB serial for debugging
	
}

void loop() {
	
	//check the state of each input one at a time.  
	
	CheckInput (Input1, Input1Latch);
	CheckInput (Input2, Input2Latch);
	CheckInput (Input3, Input3Latch);
	CheckInput (Input4, Input4Latch);
	CheckInput (Input5, Input5Latch);
	
	if (FirstSwitch > 0)
	{
		if (millis() - FirstSwitch > TimeLimit)
		{
			//if the time limit has passed, set the Voided flag
			FirstSwitch = 0;
			Voided = true;
		
			Serial.println(" Timer Expired, setting Voided to True ");
			
		}
	}
	
	//If the latch and the new value are not equal, take action
	if (SolvedLatch != PuzzleSolved){
		
		if (PuzzleSolved){
			//decide what to do if the puzzle is solved
			Serial.println("The puzzle has been solved!");
		} else {
			
			//decide what to do if the puzzle becomes unsolved
			Serial.println("The puzzle is no longer solved");
		}
		
		SolvedLatch = PuzzleSolved;
		
	}
	
}

void CheckInput (byte InputPin, bool& Latch){
	
	//notice that the Latch is passed by ref, that way we can update the value of the latch
	
	InputRead = !digitalRead(InputPin);     // read the pin, invert it because it has an internal pull-up
	if (InputRead != Latch)            // now see if the trigger value has changed.  if it has, take actions as needed
	{
		Serial.print("Input Pin# ");
		Serial.print(InputPin);
		Serial.print(" is now : ");
		Serial.println(InputRead);
		
		Latch = InputRead;                  //save the new value
		
		if (Latch)							//if the latch has just changed to TRUE, the input
		{	//was just activated.  
			
			if (Voided) {	return;}			//if voided is set, do not respond to newly activated			
			//switches
			
			if (FirstSwitch == 0)			//If firstswtich = 0, no other switch is on so save
			{	//the curent millis value to firstswitch 
				
				FirstSwitch = millis();
				
				Serial.print(" Setting FirstSwitch to ");
				Serial.println(FirstSwitch);
				
			} else							//if FirstSwitch had a value, we are still within the
			{	//time limit
				CheckSolved();					//if the switch was just activated, see if the puzzle is	
			}								//solved
			
		}									
		
		
		else								//this else handles if the key was just turned off
		{
			PuzzleSolved = false;			//make sure the solved flag is false
			CheckClear();					//see if all switches are turned off
		}
	}	
}

void CheckSolved(){
	//This routine is only called when a switch was just activated and there is
	//value in FirstSwitch. 
	
	//if all the latches are true, the puzzle is solved.		
	if (Input1Latch && Input2Latch && Input3Latch && Input4Latch && Input5Latch)
		
	{
		PuzzleSolved = true;
	}
}

void CheckClear(){
	//This routine is only called when a switch was just deactivated 
	
	//if all the latches are true, the puzzle is solved.		
	if (!Input1Latch && !Input2Latch && !Input3Latch && !Input4Latch && !Input5Latch)
		
	{
		Voided = false;
		Serial.println(" All inputs off, Clearing Voided to False ");
		
	}
}