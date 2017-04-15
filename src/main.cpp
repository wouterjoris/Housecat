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


// Removed compiler options
//-mcpu=arm7tdmi
//-T ld/target.ld
//

// Compiler used: GNU Xtools compiler for ARMv7 32 bit. 32 bit linux is in phase out mode so this code should be migrate to 64 bit at a certain point

//#include "HCgpio.h"
//#include "HCsocket.h"
#include  "HCfifo.h"
#include  "HCgpio.h"


#include <unistd.h>

#include<stdio.h>
#include<string.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <syslog.h>
#include <array>



using namespace std;

//defining the possible modes.
//Setting a bit:       number |= 1 << x;
//learing a bit         number &= ~(1 << x);
//toggling a bit       number ^= 1 << x;
//checking a bit        bit = (number >> x) & 1;



#define RPI     (1<<1)
#define BEAGLE  (1<<2)
#define ODROID  (1<<3)
#define VERBOSE (1<<4)
#define LOG     (1<<5)
#define HELP    (1<<6) // 0b100000U
#define USER    (1<<7)

#define RPIOUT  26      //debug


array <int,26> RpiGpioOut = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27};
array <int,23> RpiGpioIn  = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};



void printHelp (void)
{
    cout <<  endl<<endl;
    cout << "HouseCat Deamon Helpfile" << endl;
    cout << "" << endl;
    cout << "A platform selection is obligated to run this application" << endl;
    cout << "Currently there are only a few platforms supported ( see section below )" << endl;
    cout << "If you don't specify a user, the program will run and settle as root" << endl;
    cout << "Example format: sudo HouseCat --user alarm -r" << endl<<endl;
    cout << "Example format: sudo HouseCat --help" << endl;
    cout << "" << endl<<endl;

    cout << "Bugs or comments can be addressed to: Wouter.Joris@gmail.com " << endl;

    cout << "The Housecat deamon takes following aguments:" << endl;
    cout << "Short style arguments:" << endl<<endl;
    cout << "-r             Raspberry pi selection." << endl;
    cout << "-b             Beaglebone selection." << endl;
    cout << "-o             Odroid  selection" << endl;
    cout << "-v             Verbose mode. " << endl;
    cout << "-l             Log occurences to file" << endl;
    cout << "-u             Start program as root but fallback as the specified user" <<endl;
    cout << "-h             Print this helpfile" << endl<<endl;
    cout << "Long style arguments:" << endl<<endl;
    cout << "--rpi          Raspberry pi selection. " << endl;
    cout << "--beaglebone   Beaglebone selection. " << endl;
    cout << "--odroid       Odroid selection" << endl;
    cout << "--verbose      Verbose mode." << endl;
    cout << "--log          Log occurences to file." <<endl;
    cout << "--user         Start program as root but fallback as the specified user" <<endl;
    cout << "--help         Print this helpfile." << endl<<endl;

}



int getArguments (int argc, char **argv, string &user)
{
    int c;
    int modes = 0;

        int option_index = 0;
        static struct option long_options[] = {
            {"rpi",     no_argument,       0,  'r' },
            {"beaglebone",  no_argument,   0,  'b' },
            {"odroid",  no_argument,       0,  'o' },
            {"verbose", no_argument,       0,  'v' },
            {"help", no_argument,          0,  'h' },
            {"logging", no_argument,       0,  'l' },
            {"user", required_argument,    0,  'u' },
            {0,         0,                 0,  0 }
        };

     while ((  c = getopt_long(argc, argv, "vlrbovu:h:", long_options, &option_index)) != -1 )
     {

           switch (c)
           {
            case 'h':
                printHelp();
                exit (0);
            case 'r':
                cout << "Raspberry pi selected" << endl;

                if (( modes & ODROID ) == ODROID || (modes & BEAGLE)== BEAGLE)
                {
                    cout<< "multiple platforms selected, please use only one platform" << endl;

                    //openlog ("housecat", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
                    //syslog (LOG_INFO, "Multiple platforms selected at startup, please use only one platform");
                    //closelog ();

                    exit(0);
                }
                else
                {
                     modes |= RPI;
                }

                break;
            case 'o':
                cout<< "Odroid selected" << endl;

                if (( modes & RPI ) == RPI || (modes & BEAGLE)== BEAGLE)
                {
                    cout<< "multiple platforms selected, please use only one platform" << endl;

                    openlog ("housecat", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
                    syslog (LOG_INFO, "Multiple platforms selected at startup, please use only one platform");
                    closelog ();

                    exit(0);
                }
                else
                {
                     modes |= ODROID;
                }

                break;
            case'v':
                cout << "Verbose selected" << endl;

                modes |= VERBOSE ;

                break;
            case 'b':
                cout << "Beaglebone selected" << endl;

                if (( modes & RPI ) == RPI || (modes & ODROID)== ODROID)
                {
                    cout<< "multiple platforms selected, please use only one platform" << endl;
                    exit(0);
                }
                else
                {
                     modes |=  BEAGLE;
                }

                cout << "modes = " << modes << endl;

                break;
            case 'l':
                cout<< "Logging selected" << endl;

                modes |= LOG;
                break;
            case 'u':
                {
                    user.clear();
                    user.assign(optarg,strlen(optarg));                 // put C type char array into the defined string
                    cout<< "user " << optarg << " selected" << endl;
                    modes |= USER;
                }
                break;
            case '?':
                printHelp();
                exit(0);
            default:
                abort();
           }
        }
       return modes;
}

gpioPins** setupHardware (int &modes)
{
    int mask = 1;
    int reinitMode = 0;
    gpioPins** gpio     ;    // first ininialisation of the array

    cout << "SetupHardware " << endl;

    while (modes)
    {
        switch (modes & mask)
        {

        case BEAGLE :
            cout << "beagle case: not yet implemented " <<endl;
            exit (0);

            break;

        case RPI :
            cout << "Pi case " << endl;

            gpio = new gpioPins*[RpiGpioOut.size()];                //redefine the array size according to the Rasp pi settings all as output

            for (unsigned int i = 0 ; i < RpiGpioOut.size(); i++ )  //auto create gpio objects into an array
            {
                gpio[i] = new gpioPins(RpiGpioOut[i]);
            }

            for (unsigned int i = 0 ; i < RpiGpioIn.size(); i++ )   //Change the previous defined outputs to inputs
            {
                gpio[(RpiGpioIn[i])]->setDirection("in");

            }
            reinitMode = RPI;                                            //set the modes selector back to RPI
            break;

        case ODROID:
            cout << "Odroid case: not yet implemented " <<endl;
            exit(0);
            break;

        }

    modes &= ~mask;
    mask <<= 1;
    }
    modes = reinitMode ;
    return gpio;
}




int main(int argc , char **argv)
{

    string selectedUser;
    string message;

    gpioPins** gpio;
    HCfifo *myfifo = new HCfifo;

    int counts = 0;

    int userModes = getArguments(argc,argv, selectedUser);          //Get the command line arguments
    gpio = setupHardware (userModes);                               //setup platform dependent GPIO's

    cout<< "user " << selectedUser << " selected" << endl;
    cout<< "pinvalue " << gpio[22]->getPinValue() << " " << endl;
   // cout<< "size of array "<< gpio.size() << endl;


    while (1)                                                           // keep our programm working endlessly
    {

        myfifo->ListenToFifo(message);                                  //check for any incoming messages trough the special fifo file
        myfifo->WriteToFifo(message);

        cout << "pin number: " << RpiGpioIn[20] << endl;
        cout << "usermodes: " << userModes << endl;


        switch (userModes)                                                  //gpio input polling
        {

        case BEAGLE:
            break;
        case ODROID:
            break;
        case RPI:
            cout << "pin number: " << RpiGpioIn[20] << endl;
            //for (unsigned int i = 0 ; i < RpiGpioIn.size(); i++ )   //Change the previous defined outputs to inputs
            //{
                //gpio[(RpiGpioIn[i])]->getPinValue();
                //cout<< "pinvalue pin :" << RpiGpioIn[i] << "  is  " << gpio[(RpiGpioIn[i])]->getPinValue() << endl;

            //}

            cout << "value pin " << gpio[(RpiGpioIn[2])]->getPinValue()<<  " pin check : " << RpiGpioIn[2] <<endl;
            //cout << "out / in? "  << gpio[(RpiGpioIn[20])]->getDirection() <<endl;
            if (gpio[(RpiGpioIn[2])]->getPinValue())
            {
                gpio[22]->setValueOut("1");                             //Debug: gpio 24 array number 22
            }
            else
            {
                gpio[22]->setValueOut("0");                             //Debug: gpio 24 array number 22
            }



            break;

        }

        cout << "message in main function is :  " << message << endl;

            counts++;
            cout << "count: " << counts << " seconds"   <<endl;
            if (counts >= 100)
            {
                counts = 0;
            }

    }

    delete myfifo;
    delete gpio;
    return 0;


}
