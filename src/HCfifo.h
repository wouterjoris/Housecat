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


#ifndef HCFIFO_H
#define HCFIFO_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
//#include <pthread.h>
#include <pwd.h>        //UserUid
#include <sys/time.h>  // timeval
#include <sys/select.h>
//#include <poll.h>


#define BUFFIN 100      //Max incoming buffer length

using namespace std;
using std::string;

class HCfifo
{
    public:
        HCfifo();
        virtual ~HCfifo();

        int WriteToFifo(string &message);                            //Write to the special file
        //int ListenToFifo(string &message);
        //int CreateThread(HCfifo *arg);

        bool ExistanceFifoOut(void);                                //checks for excistence of special file Fifo out, true means file exists
        bool ExistanceFifoIn(void);                                 //checks for excistence of special file Fifo in, true means file exists
        void ListenToFifo(string &receivedMessage);    //call this function to check any new charachters are available and read the following stream
        //Pass a string where the stream will be written to.


    protected:

    private:

       //typedef string (*CallbackType) (string);

//       typedef struct  thread_data {
//                       string inPath;
//                       string returnedMessage;
//                       } thread_data;



        //static void *thread_ListenToFifo(void *message);
        struct timeval tv;

        string HFolder_user;
        string userName;

        int inFifo;
        int outFifo;

        string returnedMessage;

        string strInFile = "/HouseCatIn";
        string strOutFile  ="/HouseCatOut" ;

        string pipeInPath = "" ;
        string pipeOutPath = "" ;


        int CreateFifoFiles(void);
        int DestroyFifoFiles (void);



};
#endif // HCFIFO_H
