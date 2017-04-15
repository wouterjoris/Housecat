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



#include "HCgpio.h"


gpioPins::gpioPins (int headerpin)
{
	this->exportPin(headerpin);
}

gpioPins::gpioPins(int headerpin, string direction)
{
	this->exportPin(headerpin);
	this->setDirection (direction);
}

void gpioPins::exportPin(int headerpin)
{
	this->pinNumber = headerpin;
		this->pin = to_string (headerpin);
		if ( gpioWrite ( PATH , "export", pin))
		{
			cout << "error exporting gpio pin: " << pin << endl;

		}
		this->gpioPath = gpioPath.append(PATH);
		this->gpioPath = gpioPath.append("gpio");
		this->gpioPath = gpioPath.append (pin);
		this->gpioPath = gpioPath.append("/");
}


int gpioPins::gpioWrite (string path, string fileName, string value)
{
	ofstream ofs;
	ofs.open ( (path + fileName).c_str() );

	if (!ofs.is_open())
	{
		perror ("Unable to access output stream ");
		return -1;
	}

	ofs << value ;
	ofs.close();
	return 0;

}

string gpioPins::gpioRead (string path, string fileName )
{
	ifstream ifs;

	ifs.open((path + fileName).c_str());

	if (!ifs.is_open())
	{
		perror ("unable opening file to read");
		//return -1;
	}

	string value ;
	getline (ifs , value);
	return value;
}



void gpioPins::setDirection(string direction)
{
	gpioWrite ( this->gpioPath , "direction" , direction );
	this->pinDirection = direction ;
}

void gpioPins::setValueOut (string valueOut)
{
	this->setDirection("out");
	gpioWrite (this->gpioPath, "value" , valueOut );
}

string gpioPins::getDirection()
{
	return gpioRead ( this->gpioPath, "direction"  );
}

int gpioPins::getPinValue ()
{
	string value;
	value = gpioRead (this->gpioPath, "value");
	return std::stoi (value);
}

string gpioPins::getGpioNumber()
{
	return pin ;
}

int gpioPins::setEdge(gpioPins::EDGE value)
{
	switch ( value )
	{
		case NONE:
		{
			return  gpioWrite (this->gpioPath, "edge", "none") ;
			break;
		}
		case RISING:
		{
			return  gpioWrite (this->gpioPath, "edge", "rising");
			break;
		}
		case FALLING :
		{
			return  gpioWrite (this->gpioPath, "edge" , "falling");
			break;
		}
		case BOTH :
		{
			return  gpioWrite (this->gpioPath, "edge" , "both")  ;
			break;
		}
		return -1;
	}
	//return 0;
}

gpioPins::EDGE gpioPins::getEdge()
{
	std::string edge =  ( gpioRead (this->gpioPath, "edge"));

	if ( edge == "none")
	{
		return NONE;
	}
	else if (edge== "rising")
	{
		return RISING;
	}
	else if (edge == "falling")
	{
		return FALLING;
	}
	else if (edge == "both")
	{
		return BOTH;
	}

}

int gpioPins::initialiseEdgeWait ()
{
	this->setDirection("in");
		// make file descriptor fd, epolldestriptor epollfd, edgde value i;
		int fd;
		//epoll_event epollevent;
		this->epollfd = epoll_create (1);

		if (this->epollfd == -1)
		{
			perror ("GPIO:unable to create epoll event");
			cout << "GPIO:epoll creation failed " << endl;
			return -1;
		}

		// first get file descriptor of file to be read.
		if (( fd = open((this->gpioPath + "value").c_str() , O_RDONLY | O_NONBLOCK )) == -1 )
		{
			perror("GPIO: unable to open epoll file");
			cout << "GPIO:unable to create epoll file" << endl;

		}
		// see man epoll_ctl  read event | edge triggered | urgent data
		epollevent.events = EPOLLIN | EPOLLET | EPOLLPRI ;     ///epollevent.events = EPOLLIN |  EPOLLET | EPOLLPRI ;
		epollevent.data.fd = fd;

		if (epoll_ctl ( this->epollfd, EPOLL_CTL_ADD, fd , &epollevent)==-1)
		{
			perror ("GPIO: error registering epoll event");
			return -1;

		}


}
void  gpioPins::destructEdgeWait ()
{
	close (this->epollfd);
}


int gpioPins::waitForEdge ()
{

	this->epollresult=0;
	while (this->epollresult == 0 )
	 {
		this->epollresult = epoll_wait (epollfd, &epollevent, 1 , -1);
	 }
	if (epollresult == -1)
	{
		perror ("GPIO: epoll waiting error");
		return -1;
	}

	return this->epollresult ;

}

void* threadedPoll(void *value)
{
	gpioPins *gpiopins = static_cast<gpioPins*>(value);
	while (gpiopins->threadRunning == true)
	{
		gpiopins->callbackFunction  (gpiopins->waitForEdge());
		usleep(1000);
	}
}

int gpioPins::waitForEdge ( CallbackType callback)
{
	this->threadRunning = true;
	this->callbackFunction = callback;
	if (pthread_create(&this->thread, NULL, &threadedPoll, static_cast <void*>(this)))
	{
		perror ("failed to create pthread");
		return -1;
		this->threadRunning = false;
	}
	return 0;


}



gpioPins::~gpioPins()
{
	gpioWrite (PATH , "unexport" , pin );
}


