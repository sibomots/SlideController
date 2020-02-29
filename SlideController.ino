//  SlideController for making gear parts
//  Copyright (C) 2020  Jeff Wandling
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published
//  by the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
// 
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// begin "do not edit" section
const int BIG_GEAR_TEETH_COUNT = 60;
const int SMALL_GEAR_TEETH_COUNT = 15;
// Calibration data  -- do not edit
const int SMALL_GEAR_ROTATION_REQUIRED = 1000;
const int BIG_GEAR_ROTATION_REQUIRED = 1000;
// end of section

// EDIT HERE:
const int NUMBER_OF_TEETH = BIG_GEAR_TEETH_COUNT;
const int GEAR_ROTATION_REQUIRED = BIG_GEAR_ROTATION_REQUIRED;

// STOP EDITING

//////////////////////////////////////////////////////////////
// DO NOT EDIT BELOW THIS LINE
//////////////////////////////////////////////////////////////

// These do not change during execution
// read only values (not variables).  
const int CW = 1;
const int CCW = 0;
const int N_ENABLE = 0;
const int N_DISABLE = 1;
const int TO_END = 1;
const int TO_PARK = 0;
const int slideEnablePin = 13;
const int slideDirectionPin = 12;
const int slidePulsePin = 11;
const int gearEnablePin = 8;
const int gearDirectionPin = 7;
const int gearPulsePin = 6;
const int contactEndSwitchPin = 10;
const int contactParkSwitchPin = 9;

// Define all of the nodes in the "state machine" graph.
//
// The 'state' machine is a graph of which nodes of functionality execute
// based on parameters determined during run-time.
typedef enum {
	PRE_INIT,
	INIT,
	MOVE_TO_END,
	CHECK_END_SWITCH,
	STOP_AT_END,
	MOVE_TO_PARK,
	CHECK_PARK_SWITCH,
	STOP_AT_PARK,
	CHECK_COUNTS,
	ROTATE_GEAR,
	CHECK_ROTATION,
	DONE_ROTATION,
	COMPLETED_GEAR,
	RESET,
} State;

// Globals Variables
// These change during execution
int numTeethCut = 0;
int gearRotationCount = 0;
State state = PRE_INIT;


// FUNCTIONS

// If all of the teeth needed are cut, return 1 (true)
// else return 0  (false)
bool doneWithGear()
{
	if (numTeethCut == NUMBER_OF_TEETH)
	{
		return true;
	}
	return false;
}

// If the End Switch has made contact, return 1 (true)
// else return 0 (false)
bool readEndSwitchContact()
{
	int pin = digitalRead(contactEndSwitchPin);
	return (pin != 0);
}

// If the Park Switch has made contact, return 1 (true)
// else return 0 (false)
bool readParkSwitchContact()
{
	int pin = digitalRead(contactParkSwitchPin);
	return (pin != 0);
}

// enable the slide motor.  The slide motor is
// ACTIVE LOW meaning that a LOW signal is going 
// to activate the motor.
void enableSlide()
{
	digitalWrite(slideEnablePin, N_ENABLE);
}

// disable the slide motor.  The slide motor is
// ACTIVE LOW meaning that a HIGH signal is going 
// to de-activate the motor.
void disableSlide()
{
	digitalWrite(slideEnablePin, N_DISABLE);
}

// enable the gear-rotation motor.  The motor is
// ACTIVE LOW meaning that a LOW signal is going 
// to activate the motor.
void enableGearRotation()
{
	// active low
	digitalWrite(gearEnablePin, N_ENABLE);
}

// disable the gear-rotation motor.  The motor is
// ACTIVE LOW meaning that a HIGH signal is going 
// to de-activate the motor.
void disableGearRotation()
{
	// active low
	digitalWrite(gearEnablePin, N_DISABLE);
}


void setSlideDirection(int dir)
{
	if (dir > 0) {
		digitalWrite(slideDirectionPin, CW);
	}
	else {
		digitalWrite(slideDirectionPin, CCW);
	}
}

void setGearDirection(int dir)
{
	if (dir > 0) {
		digitalWrite(gearDirectionPin, CW);
	}
	else {
		digitalWrite(gearDirectionPin, CCW);
	}
}

// TODO  calibrate the gear rotation pulse signal.
void pulseGearPin()
{
	static int value = 0;
	static long last = micros();
	long current_micro = micros();
	if ((current_micro - last) >= 10000) {
		digitalWrite(gearPulsePin, value);
		value = !value;
		last = micros();
	}
}

// TODO  calibrate the slide pulse signal.
void pulseSlidePin()
{
	static int value = 0;
	static long last = micros();

	long current_micro = micros();

	if ((current_micro - last) >= 10000) {
		digitalWrite(slidePulsePin, value);
		value = !value;
		last = micros();
	}
}

void setup() {

	pinMode(slideEnablePin, OUTPUT);
	pinMode(slideDirectionPin, OUTPUT);
	pinMode(slidePulsePin, OUTPUT);

	pinMode(gearEnablePin, OUTPUT);
	pinMode(gearDirectionPin, OUTPUT);
	pinMode(gearPulsePin, OUTPUT);

	disableSlide();
	disableGearRotation();
    state = PRE_INIT;
    numTeethCut = 0;
    gearRotationCount = 0;
}


void loop() {

	switch (state)
	{
	case PRE_INIT:
		// print instructions for setup
		disableGearRotation();
		disableSlide();
		state = INIT;
		break;
	case INIT:
		numTeethCut = 0;
		gearRotationCount = 0;

		// initialize counts for new gear
		// confirm readiness
		// reset all peripherals
		state = MOVE_TO_END;
		break;
	case MOVE_TO_END:
		// enable the slide to move towards the end
		// (we are cutting!)
		setSlideDirection(TO_END);
		enableSlide();
		state = CHECK_END_SWITCH;
		break;
	case CHECK_END_SWITCH:
        // check the state of the end switch
		// if the switch shows contact, we need to reverse
		// and go to MOVE_TO_PARK now
		if (readEndSwitchContact())
		{
			state = STOP_AT_END;
		}
		else {
			state = CHECK_END_SWITCH;
		}
		break;

	case STOP_AT_END:
		disableSlide();
		state = MOVE_TO_PARK;
		break;

	case MOVE_TO_PARK:
		setSlideDirection(TO_PARK);
		enableSlide();
		state = CHECK_PARK_SWITCH;
		break;


	case CHECK_PARK_SWITCH:
        // check the state of the park switch
		// if the switch shows contact, we need to reverse
		// and go to CHECK_COUNTS since we have made ONE pass over the gear.
		if (readParkSwitchContact())
		{
			state = STOP_AT_PARK;
		}
		else {
			state = CHECK_PARK_SWITCH;
		}
		break;

	case STOP_AT_PARK:
		disableSlide();
		state = CHECK_COUNTS;
		break;

	case CHECK_COUNTS:

		// do all calculations on the counts
		numTeethCut++;

		// check if we're done with this gear.
		if (doneWithGear())
		{
			state = COMPLETED_GEAR;
		}
		else {
			state = ROTATE_GEAR;
		}
		break;

	case ROTATE_GEAR:
		// we just do what is necesssary to rotate gear N deg.
		// set direction of rotationMotor
        // enable the rotationMotor
		gearRotationCount = 0;
		setGearDirection(CW);
		enableGearRotation();
		state = CHECK_ROTATION;
		break;

	case CHECK_ROTATION:
		gearRotationCount++;

		if (gearRotationCount > GEAR_ROTATION_REQUIRED)
		{
			disableGearRotation();
			state = DONE_ROTATION;
		}
		else
		{
			state = CHECK_ROTATION;
		}
		break;

	case DONE_ROTATION:
		// we've done rotating the gear.  
		// we've made one simple pass to cut a tooth.
		// start over
		state = MOVE_TO_END;
		break;

	case COMPLETED_GEAR:
		// ring the bell.
		// this gear is done.  All teeth are cut.
		state = RESET;
		break;
	case RESET:
		// it's time to reset the system for a new part
		state = PRE_INIT;
	}
}
