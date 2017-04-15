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




#include "HCfifo.h"

HCfifo::HCfifo()
{

    CreateFifoFiles();
}

HCfifo::~HCfifo()
{
     DestroyFifoFiles();
}


int HCfifo::CreateFifoFiles()

{

    passwd *pw = getpwuid(getuid());                //get User information and deduct home directory
    if (pw)
    {
       this->HFolder_user = pw->pw_dir;
       this->userName = pw->pw_name;
    }
    if ( access(this->HFolder_user.c_str(),F_OK ) < 0)      // test for the existence of the home directory
    {

        HFolder_user.append("/home/");
        HFolder_user.append(this->userName);

        if (mkdir(this->HFolder_user.c_str(), S_IWUSR | S_IRUSR | S_IXUSR ) < 0)         //create a users home folder with users permissions
        {
            printf("Error the creation of the user's home folder failed..");
        }

    }

    this->pipeInPath.append(this->HFolder_user);        // create the file/folder path
    this->pipeInPath.append(this->strInFile) ;

    this->pipeOutPath.append(this->HFolder_user);
    this->pipeOutPath.append(this->strOutFile) ;


    if( access( this->pipeInPath.c_str() , F_OK ) < 0 )         // Test for existence of special file
    {
        if ( (inFifo = mkfifo( this->pipeInPath.c_str(), S_IWUSR | S_IRUSR | S_IXUSR)) < 0 )  // write user, read for user, execute for user permissions
        {

      printf ("Error: The creation of the file: %s failed\n", this->pipeInPath.c_str() );

        //return -1 ;
        }
    }
    if( access( this->pipeOutPath.c_str() , F_OK ) < 0 )
    {

        if ( (outFifo = mkfifo(this->pipeOutPath.c_str() , S_IWUSR | S_IRUSR | S_IXUSR)) < 0 )
        {

            printf ("Error: The creation of the file %sfailed\n", this->pipeOutPath.c_str() );
            //return -1;
        }

    }

    return 0;
}

bool HCfifo::ExistanceFifoIn(void)                             // return value true is file exists
{
    if( access( this->pipeInPath.c_str() , F_OK ) < 0 )         // Test for existance of special file < 0 == non existent
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool HCfifo::ExistanceFifoOut(void)                            // return value true is file exists
{
    if( access( this->pipeOutPath.c_str() , F_OK ) < 0 )         // Test for existance of special file < 0 == non existent
    {
        return false;
    }
    else
    {
        return true;
    }
}


int HCfifo::DestroyFifoFiles (void)
{
    close (inFifo);
    close (outFifo ) ;


    if( remove(this->pipeInPath.c_str()) != 0 )
        printf ( "Error deleting  special file: %s \n", this->pipeInPath.c_str());
    else
        printf ( "File %s successfully deleted\n" , this->pipeInPath.c_str() );


    if( remove(pipeOutPath.c_str() ) != 0 )
        printf ( "Error deleting  special file: %s \n" , this->pipeOutPath.c_str() );
    else
        printf ( "File: %s successfully deleted\n" , this->pipeOutPath.c_str());


    return 0;
}

//
//int HCfifo::CreateThread(HCfifo *arg)
//{
//    pthread_t thread;
//
//    //string _path = pipeOutPath.c_str();
//
//    //thread_data tdata;
//    //tdata.inPath.copy(pipeOutPath,50);
//    //tdata.returnedMessage.copy(pipeOutPath.c_str(),50);
//
//    //.inPath = pipeOutPath.c_str();
//
//
//    cout << "arg path is :  " << arg->pipeOutPath << endl;
//
//    if (pthread_create ( &thread, NULL, &HCfifo::thread_ListenToFifo,(void*) &arg ) < 0)    //  &_path
//    {
//        cout << "Failed to create the pipe input thread\n" << endl ;
//        return -1;
//    }
//
//
//    //cout << "tdata returned message is :  " << tdata.returnedMessage << endl;
//    //message = tdata.returnedMessage;
//
////    void* result;
////    pthread_join(thread, (void**) &result);
////    message = *static_cast<std::string*>(result);
////
////    cout << "thread message message 2 is :  " << message << endl;
//    return 0;
//
//
//}


int HCfifo::WriteToFifo(string &message)
{
    fd_set wrset;

    FILE * pFile;
    char buffer [message.length()];
    strcpy(buffer, message.c_str());
    pFile = fopen (this->pipeOutPath.c_str(),  "w+" );              //open special file O_RDWR as the next best thing normally O_NONBLOCK is needed to get passed a bug of the select fuction

    if (pFile == NULL)
    {
        perror ("Error opening file");
    }

    FD_ZERO(&wrset);                                                //clear the set

    struct timeval tv;

    tv.tv_sec= 0 ;
    tv.tv_usec= 50 ;                                                 //5ms waiting delay to check incoming messages


        FD_SET((fileno (pFile)), &wrset );                           // The function fileno converts a standard I/O file pointer into its corresponding descriptor, FD set adds the fd to the set.

        select(fileno (pFile)+1, NULL, &wrset, NULL, &tv);           //fileno (pFile)+1


        if (strlen(buffer) > 0 )
        {

            write(fileno(pFile), buffer , strlen(buffer));

            if (FD_ISSET(fileno(pFile),  &wrset))
            {
                cout << "sent" << endl;
            }

            else
            {
                cout << "timed out" << endl;
            }
       }

}

//int HCfifo::ListenToFifo(string &message)
//{
//    ifstream ifs;
//
//	ifs.open(this->pipeOutPath.c_str());
//
//	if (!ifs.is_open())
//	{
//		perror ("unable opening file to read");
//    }
//
//	getline (ifs , message);
//	return 0 ;
//}

//void *HCfifo::thread_ListenToFifo(void *arg)
//{
//    //std::string &_path = *static_cast<std::string*>(arg);
//
//    cout << "here we are.. " << endl;
//
//    thread_data *tdata=(thread_data *)arg;
//
//
//
//    string str;
//    ifstream ifs;
//
//    //cout<< "path is: " << _path << endl;
//    cout<< "path is:"   << tdata->inPath << endl;
//
//	ifs.open(tdata->inPath);                                    // opening special fifo file
//	if (!ifs.is_open())                                 // check if special file is open
//	{
//		perror ("Pthread: unable opening file to read");
//
//	}
//
//	//getline (ifs , str );                                // Get the received message
//	getline (ifs, tdata->returnedMessage);
//
//	//cout << "thread message is :  " << str << endl;
//
//	cout << "thread message is :  " << tdata->returnedMessage << endl;
//
//    string * result = new string (str);
//	//return (void*) (result);
//
//    //return (void*) tdata;
//
//
//}

//void *HCfifo::thread_ListenToFifo(void *arg)
//{
//    //std::string &_path = *static_cast<std::string*>(arg);
//    HCfifo *This = reinterpret_cast <HCfifo*>(arg);
//
//    cout << "here we are.. " << endl;
//
//    string &_path = new string (This->pipeOutPath);
//
//
//
//    string str;
//    ifstream ifs;
//
//    //cout<< "path is: " << _path << endl;
//    cout<< "path is:"   << _path << endl;
//
//	ifs.open(_path);                                    // opening special fifo file
//	if (!ifs.is_open())                                 // check if special file is open
//	{
//		perror ("Pthread: unable opening file to read");
//
//	}
//
//	//getline (ifs , str );                                // Get the received message
//	getline (ifs, This->returnedMessage);
//
//	//cout << "thread message is :  " << str << endl;
//
//	cout << "thread message is :  " << This->returnedMessage << endl;
//
//    string * result = new string (str);
//	//return (void*) (result);
//
//    //return (void*) tdata;
//
//
//}

void HCfifo::ListenToFifo( string &receivedMessage )
{


    fd_set rset;
    receivedMessage.clear();                                       //clear previous received message

    FILE * pFile;
    char buffer [BUFFIN];
    pFile = fopen (this->pipeInPath.c_str(),  "r+" );              //open special file O_RDWR as the next best thing normally O_NONBLOCK is needed to get passed a bug of the select fuction

    if (pFile == NULL)
    {
        perror ("Error opening file");
    }

    FD_ZERO(&rset);                                                //clear the set

    struct timeval tv;

    tv.tv_sec= 1 ;
    tv.tv_usec= 0 ;                                                 //5ms waiting delay to check incoming messages

    {
        FD_SET((fileno (pFile)), &rset );                           // The function fileno converts a standard I/O file pointer into its corresponding descriptor, FD set adds the fd to the set.


        select(fileno (pFile)+1, &rset, NULL, NULL, &tv);           //fileno (pFile)+1

        if (FD_ISSET(fileno(pFile), &rset))                         //Message received
        {

           while (!feof (pFile))                                    //keep on reading until "end of file"
           {
              if (fgets(buffer, BUFFIN , pFile) != NULL)            //get charachters and put them in the buffer
              {
                fclose (pFile);
                receivedMessage.assign(buffer,strlen(buffer));      //Put buffer into string of this function
                return;
              }
              else
              {
                break;
              }
           }
        }
    }
}


