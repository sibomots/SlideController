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
const int SMALL_GEAR_ROTATION_REQUIRED = 4; // TODO EDIT TO calibration data.
const int BIG_GEAR_ROTATION_REQUIRED = 4; // TODO EDIT TO calibration data.
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
const int contactEndSwitchPin = 10;
const int contactParkSwitchPin = 9;
const int gearEnablePin = 8;
const int gearDirectionPin = 7;
const int gearPulsePin = 6;
const int nResetPin = 5;
const int nPausePin = 4;

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
	CHECK_RESET,
    RESET,
	FAKE,
} State;

// Globals Variable
// These change during execution
int numTeethCut = 0;
int gearRotationCount = 0;
State state = PRE_INIT;
char INSTRUCTIONS[1024];

// FUNCTIONS

void setInstructions();

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
    return (pin == 0);
}

// If the Park Switch has made contact, return 1 (true)
// else return 0 (false)
bool readParkSwitchContact()
{
    int pin = digitalRead(contactParkSwitchPin);
    return (pin == 0);
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

// given a motor direction value (CW or CCW), set
// the direction
void setSlideDirection(int dir)
{
    // only valid directions, please
    if ((dir == CCW || dir == CW)) 
    {
        digitalWrite(slideDirectionPin, dir);
    }
}

// given a motor direction value (CW or CCW), set
// the direction
void setGearDirection(int dir)
{
    // only valid directions, please
    if ((dir == CCW || dir == CW)) 
    {
        digitalWrite(gearDirectionPin, dir);
    }
}

// TODO  calibrate the gear rotation pulse signal.
void pulseGearPin()
{
    static int value = 0;
    static long last = micros();
    long current_micro = micros();
    if ((current_micro - last) >= 10000) 
    {
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
    if ((current_micro - last) >= 10000) 
    {
        digitalWrite(slidePulsePin, value);
        value = !value;
        last = micros();
    }
}

bool releaseResetButton()
{
    int pin = digitalRead(nResetPin);
	//Serial.print("releasePin value: ");
	//Serial.print(pin);
	//Serial.print("\n");
    return (pin == 0);
}


// arduino bsp will call setup once.
// we setup our peripherals and counters, and disable
// all periphral behavior here.
void setup() 
{
    pinMode(slideEnablePin, OUTPUT);
    pinMode(slideDirectionPin, OUTPUT);
    pinMode(slidePulsePin, OUTPUT);
    pinMode(gearEnablePin, OUTPUT);
    pinMode(gearDirectionPin, OUTPUT);
    pinMode(gearPulsePin, OUTPUT);
	pinMode(nResetPin, INPUT_PULLUP);
	pinMode(nPausePin, INPUT_PULLUP);

	pinMode(contactEndSwitchPin, INPUT_PULLUP);
	pinMode(contactParkSwitchPin, INPUT_PULLUP);

    disableSlide();
    disableGearRotation();
    state = PRE_INIT; // our state machine starts at this node
    numTeethCut = 0;
    gearRotationCount = 0;
	setInstructions();
	Serial.begin(115200);
}

bool inPause()
{
    // When the operator wants to make emergency Pause
    // when this button is released, the operation will resume
    // where it left from.
    int pin = digitalRead(nPausePin);
	//Serial.print(pin);
	//Serial.print("\n");
    return (pin == 0);
}
    

// The arduino bsp will call loop repeatedly.
// Our "loop" just implements a simple state-machine
// to direct exection at each loop iteration.
void loop() 
{
	static long debug_counter = 0;

    if ( inPause() )
    {
        //Serial.print("Check pause\n");
        // Patience.
        // 
        // Just wait here for a second. No reason to keep polling
        // so rapidly.  Whenever the Pause button is released
        // this will be skipped.
        delay(1000);
		return;
    }

    // Look for the reset switch
    // This is the control for the operator to allow the
    // system to proceed with a new run from the initial 
    // state.  Operator should not manipulate the Reset
    // switch during a run!!  TODO block out Reset during a run.

    // what node are we in?  run the behavior based on our state.

	//Serial.print("Debug counter: ");
	//Serial.print(debug_counter++);
	//Serial.print("\n");
    switch (state)
    {
    case PRE_INIT:
		// initially, disable the motors
        disableGearRotation();
        disableSlide();

        // print instructions for setup
		Serial.print(INSTRUCTIONS);

		// check for reset release
		state = CHECK_RESET;
		break;

	case CHECK_RESET:
		if (releaseResetButton())
		{
			Serial.print("Released RESET. Proceed!\n");
			state = INIT;
		}
		else
		{
			state = CHECK_RESET;
		}
		break;

    case INIT:
		Serial.print("INIT step\n");
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
		Serial.print("MOVE_TO_END initiated\n");
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
			Serial.print("End Switch contacted\n");
            state = STOP_AT_END;
        }
        else 
        {
            // come back to this state.
			//Serial.print("Check End switch again\n");
            state = CHECK_END_SWITCH;
        }
        break;

    case STOP_AT_END:
		Serial.print("Stopping at End. Disable motor!\n");
        disableSlide();
		delay(100);
        state = MOVE_TO_PARK;
        break;

    case MOVE_TO_PARK:
		Serial.print("Setting direction to Park\n");
        setSlideDirection(TO_PARK);
		delay(100);
		Serial.print("Slide Motor enabled\n");
        enableSlide();
		delay(100);
		Serial.print("Move to Park enabled..\n");
        state = CHECK_PARK_SWITCH;
        break;

    case CHECK_PARK_SWITCH:
        // check the state of the park switch
        // if the switch shows contact, we need to reverse
        // and go to CHECK_COUNTS since we have made ONE pass over the gear.
        if (readParkSwitchContact())
        {
			Serial.print("Park Switch contacted\n");
            state = STOP_AT_PARK;
        }
        else 
        {
            // come back to this state.
			//Serial.print("Check Park switch again\n");
            state = CHECK_PARK_SWITCH;
        }
        break;

    case STOP_AT_PARK:
		Serial.print("Stopping at Park. Disable motor!\n");
        disableSlide();
		delay(100);
        state = CHECK_COUNTS;
        break;

    case CHECK_COUNTS:
        // do all calculations on the counts
        numTeethCut++;

		Serial.print("Gear count: ");
		Serial.print(numTeethCut);
		Serial.print("\n");

        // check if we're done with this gear.
        if (doneWithGear())
        {
			Serial.print("Completed the gear!\n");
            state = COMPLETED_GEAR;
        }
        else 
        {
            state = ROTATE_GEAR;
        }
        break;

    case ROTATE_GEAR:
        // we just do what is necesssary to rotate gear N deg.
        // set direction of rotationMotor
        // enable the rotationMotor
        gearRotationCount = 0;
        setGearDirection(CW);
		delay(100);
        enableGearRotation();
		delay(100);
        state = CHECK_ROTATION;
        break;

    case CHECK_ROTATION:
        gearRotationCount++;
		Serial.print("Gear rotating index: ");
		Serial.print(gearRotationCount);
		Serial.print("\n");
        if (gearRotationCount > GEAR_ROTATION_REQUIRED)
        {
			Serial.print("Gear rotation finished.\n");
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
		Serial.print("Done rotation. New state, move to End\n");
        state = MOVE_TO_END;
        break;

    case COMPLETED_GEAR:
        // ring the bell, whatever...  
        // this gear is done.  All teeth are cut.
        state = RESET;
        break;
    case RESET:
        // it's time to reset the system for a new part

        // Note:
        // TODO enable a kill-switch disablement. 
        // We remain in RESET until the operator
        // has removed the signal to stay in RESET.
        // Goal: We do NOT enter PRE_INIT until the operator
        // has swapped out the parts of the mechanism
        // Until that's rectified, we are going to remain in RESET.
        // BUGBUG 
        // state = PRE_INIT;

		Serial.print("At RESET, Start over at PRE_INIT\n");
		state = PRE_INIT;
    }

	// delay(2000);
    // leave the loop iteration
}


void setInstructions()
{
	static char* instructions =
		"This is the instructions for the software\n"
		"Do step 1\n"
		"Do step 2\n"
		"Etc..\n";

	memset(INSTRUCTIONS, 0, 1024);
	memcpy(INSTRUCTIONS, instructions, strlen(instructions));
}
