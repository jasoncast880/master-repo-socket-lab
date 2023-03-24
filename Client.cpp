// libraries used
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync; 

// creating new class derived from Thread class
class ClientThread : public Thread
{

// private member variables used
private:

	bool &functioning; // refrence to boolean

	//conncected socket
	Socket& sock; // refrences a Socket object
	
	//variables to store sent and received data
	ByteArray info;
	
	std::string info_str;
	
// ClientThread constructor that has 2 parameters
public:

	ClientThread(Socket& sock, bool &functioning)
	: sock(sock), functioning(functioning)
	{}


	// destructor 
	~ClientThread()
	{}

	// virtual method that returns a long
	virtual long ThreadMain()
	{
	
		while(true) {
		
			try {
				std::cout << "Please input your data, enter DONE to exit: ";
				std::cout.flush();

				// getting data from user 
				std::getline(std::cin, info_str);
				
				// storing user data
				info = ByteArray(info_str); 

				// if the user enters done exit loop
				if(info_str == "done") {
					functioning = false;
					break;
				}


				// Write data to the server
				sock.Write(info);


				// Getting response from server
				int conn = sock.Read(info);

				
				if(conn == 0) {
					functioning = false;
					break;
				}


				// printing to console
				std::cout<<"Server Response: " << info.ToString() << std::endl;
				
				
			// catching exceptions
			} catch (std::string err)
            {
                std::cout<<err<<std::endl;
            }
		}

		return 0;
	}
};

int main(void)
{
	
	std::cout << "SE3313 Lab 3 Client" << std::endl; // printing message
	
	bool functioning = true; // condition for loop

	
	Socket sock("127.0.0.1", 3000); // creating socket
	
	ClientThread clientThread(sock, functioning); // creating thread and initalizing with socket
	
	sock.Open(); // opening connection
	
	
	// loop to wait 1 second while it is functioning
	while(functioning) {
	
		sleep(1);
		
	}

	sock.Close(); // closing connection
	
	return 0;
}

