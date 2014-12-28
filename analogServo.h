//================================================ file = pentiServo.h =========
//= Arduino API for pairing Analog input devices (Potentiometer's, 
//  variable resistors, photo resistors, etc...) to Standard Servos (non-continuous)
//==============================================================================
//=  Tested Successfully with: Flex Sensor's, Potentiometers
//=----------------------------------------------------------------------------=
//=  Example execution: See -->  Analog_to_Servo.ino                           =
//=----------------------------------------------------------------------------=
//=  Author: Dominic Cox                                                       =
//=----------------------------------------------------------------------------=
//=  History: Creation - 11-21-14 Dominic Cox                                  =
//==============================================================================



#include "C:\Program Files (x86)\Arduino\Arduino ERW 1.0.5\libraries\Servo\Servo.h"
#include "Arduino.h"
#include <String>
//Defined Servo's max degree
#ifndef SERVO_MAX
#define SERVO_MAX 180
#endif SERVO_MAX

//Defined servo's min
#ifndef SERVO_MIN
#define SERVO_MIN 0
#endif SERVO_MIN



class Analog_to_Servo
{
	public:
	
	int *analog_pins; //array that stores analog pins
	int *servo_pins;  //array that stores servo signal pins
	Servo *servos;    //array of Servo instances
	int *a_MINS;      //array of minimum read-in analog value
	int *a_MAXS;	  //array of maximum read-in analog value
	int *s_MINS;	  //array of minimum servo degree allowed
	int *s_MAXS;      //array of maximum servo degree allowed
	int *last_val;    //array of last written value
	int *change;      //array of needed degree change between writes for the servos
	
	int pairs;        //number of connected pairs 
	int current_first;// index of next available array location

	//Default constructor
	//num_pairs == The number of analog Servo pairs that will be linked
	Analog_to_Servo(int num_pairs)
	{
		pairs = num_pairs;
		current_first = 0;
		analog_pins = new int[pairs]();
		servo_pins = new int[pairs]();
		servos = new Servo[pairs]();
		a_MINS = new int[pairs]();
		a_MAXS = new int[pairs]();
		s_MINS = new int[pairs]();
		s_MAXS = new int[pairs]();
		last_val = new int[pairs]();
		change = new int[pairs]();

		//init(num_pairs);
	}
	
	//Default Destructor
	~Analog_to_Servo()
	{
		delete [] analog_pins;
		delete [] servo_pins;
		delete [] servos;
		delete [] a_MINS;
		delete [] a_MAXS;
		delete [] s_MINS;
		delete [] s_MAXS;
		delete [] last_val;
		delete [] change;
	}
	
	
	
	//Reinitializes Analog_Servo_instance with new pair size 
	void init(int num_pairs)
	{
		delete_arrays();
		pairs = num_pairs;
		analog_pins = new int[num_pairs]();
		servo_pins = new int[num_pairs]();
		servos = new Servo[num_pairs]();
		a_MINS = new int[num_pairs]();
		a_MAXS = new int[num_pairs]();
		s_MINS = new int[pairs]();
		s_MAXS = new int[pairs]();
		last_val = new int[pairs]();
		change = new int[pairs]();
	}

	
	//Required:
	//a_pin == pin to read an analog value from
	//serv_pin == pin servo is connected to that will be linked to a_pin
	//Optional:
	//min == minimum expected value from analog read
	//max == maximum expected value from analog read
	//s_min == minimum allowed servo degree
	//s_max == maximum allowed servo degree
	//change == needed change of degree between writes
	//
	//Links a_pin and serv_pin together so that the values read in from a_pin are mapped to the serv_pin 
	// -- min/max are expected read in analog value bounds
	// -- s_min/s_max are the bounds on the servo degree writes -- if not given defaults to defined servo min/max 
	// -- change is the needed degree change between sequential servo writes -- if not given defaults to 0
	// returns true if the pair of pins was successfully linked, else false
	bool link(int a_pin, int serv_pin, int min = 1027, int max = 0, int s_min = SERVO_MIN, int s_max = SERVO_MAX, int cge = 0)
	{
		if(current_first >= pairs)
		{
			return false;
		}
		else
		{
			analog_pins[current_first] = a_pin;
			servo_pins[current_first] = serv_pin;
			servos[current_first] = Servo();
			servos[current_first].attach(servo_pins[current_first]);
			a_MINS[current_first] = min;
			a_MAXS[current_first] = max;
			s_MINS[current_first] = s_min;
			s_MAXS[current_first] = s_max;
			last_val[current_first] = 0;
			change[current_first] = cge;
			
			current_first += 1;
			return true;
		}
	}
	

	//reads passed in analog pin, reads the value, and maps to the servo
	//returns int array with index 0 = analog value read and index 1 = servo degree written
	String read_and_write(int a_Pin)
	{
		bool found = false;
		int pin;
		
		for(unsigned int i = 0; i < pairs; ++i)
		{
			if (analog_pins[i] == a_Pin)
			{
				pin = i;
				found = true;
				break;
			}
		}
		if (found)
		{
			//read value from analog pin
			int curr_val = read(analog_pins[pin]);
			
			//check and update bound as necessary
			if (curr_val > a_MAXS[pin])
			{
				a_MAXS[pin] = curr_val;
			}
			else if (curr_val < a_MINS[pin])
			{
			
				a_MINS[pin] = curr_val;
			}
			
			//map the current read value to the set bounds
			int degree = map(curr_val, a_MINS[pin],a_MAXS[pin], s_MINS[pin], s_MAXS[pin]);
			
			if (degree >= (last_val[pin]+change[pin]) || degree <= (last_val[pin] - change[pin]))
			{
				//write to servo
				last_val[pin] = degree;
				write(pin, degree);	
			}
			
			String rep = "Analog value of pin "+ a_Pin;
			rep += " = " + curr_val;
			rep += "\nDegree written to Servo pin "+servo_pins[pin];
			rep += " = "+degree;
			return rep;
		}
		else
		{	
			String r = "Analog Pin not found";
			return r;
		}
	}
	
	
	//reads and writes all linked pairs of analog servos
	// d == delay in ms between reading/writing pairs == default is 0ms
	void read_and_write_all(int d = 0)
	{
		for(unsigned int i = 0; i < pairs; ++i)
		{
			int curr_val = read(analog_pins[i]);
			if (curr_val > a_MAXS[i])
			{
				a_MAXS[i] = curr_val;
			}
			else if (curr_val < a_MINS[i])
			{
			
				a_MINS[i] = curr_val;
			}
			
			int degree = map(curr_val, a_MINS[i], a_MAXS[i], s_MINS[i], s_MAXS[i]);
			
			if (degree >= (last_val[i]+change[i]) || degree <= (last_val[i] - change[i]))
			{
				//write to servo
				last_val[i] = degree;
				write(i, degree);	
			}
			
			delay(d);
			
		}
	
	}
	
	private:
	
	//helper function for refreshing internals
	void delete_arrays()
	{
		delete [] analog_pins;
		delete [] servo_pins;
		delete [] servos;
		delete [] a_MINS;
		delete [] a_MAXS;
		delete [] s_MINS;
		delete [] s_MAXS;
		delete [] last_val;
		delete [] change;
	}
	
		//helper function
	int read(int pen_pin)
	{
		return analogRead(pen_pin);
	
	}
	
	//helper function
	void write(int pin, int val)
	{
		servos[pin].write(val);
	}
	
};

