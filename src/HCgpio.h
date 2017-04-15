/* Housecat PLC interfacing application

    This file is part of Housecat.

    Housecat is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Housecat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Housecat.  If not, see <http://www.gnu.org/licenses/>.


*/



#ifndef HCGPIO_H
#define HCGPIO_H


#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>




//    To do:  MUX table implementation,  Linux Polling, ptread.
//




#define PATH  "/sys/class/gpio/"



using namespace std;
using std::string;

typedef int (*CallbackType)(int);

class gpioPins
{


public:
	enum EDGE { NONE,RISING, FALLING, BOTH };
	enum VALUE { LOW , HIGH };
	enum INSTR { STOP, START };
	enum MUX {SLEWCTRL, RXACTIVE , PUTYPESEL, PUDEN, MMODE };  // AM335x specific  //PUDEN: pull-up or pull-down resistor: enable=0 disable=1

public:

	gpioPins(int headerpin);
	gpioPins(int headerpin, string direction);  // eg "in" "out"

	virtual void 	exportPin	    (int headerpin);
	virtual int  	gpioWrite		(string path , string fileName, string value);
	virtual string  gpioRead		(string path , string fileName);

	//void 	setPinMux (gpioPins::MUX);

	virtual void 	setDirection   	(string direction );
	virtual void 	setValueOut    	(string valueOut);
	virtual int		setEdge			(gpioPins::EDGE );



	virtual string 	getDirection   (void);
	virtual int 	getPinValue    (void);
	virtual string 	getGpioNumber  (void);
	virtual gpioPins::EDGE  getEdge	(void);

	virtual int waitForEdge (void);
	virtual int waitForEdge(CallbackType callback);


	virtual int initialiseEdgeWait (void);
	virtual void destructEdgeWait (void);

	virtual ~gpioPins();


private:
	friend void* threadedPoll(void* value);





private:

	int pinNumber;
	string pin ;
	string pinDirection;
	string gpioPath;
	//string gpio = "/gpio";

	epoll_event epollevent;
	int epollfd, epollresult;

	//pthread

	pthread_t thread ;
	CallbackType callbackFunction;
	bool threadRunning = false;





};

void* threadedPoll(void* value);


#endif /* gpio_GPIO_H_ */


