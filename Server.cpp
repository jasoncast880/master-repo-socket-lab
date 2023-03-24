// importing necessary libraries
#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync; // using syncSocket namespace


/*
This is the SocketThread class. It extends the Thread class in the SyncSocket library. 
This class is used to handle each client socket
*/
class SocketThread : public Thread
{
private:
    
    Socket& sock; // refrencing client socket the thread will handle
    
    ByteArray info; // holds data from the client sokcet

    bool &terminate; // determines if thread should terminate
    
    
    // vector that will keep track of all threads
    std::vector<SocketThread*> &sockThrHolder; 
    
    
public:

    // Constructor for the SocketThread class, and initalize with input parameter
    SocketThread(Socket& sock, bool &terminate, std::vector<SocketThread*> &clientSockThr)
    : sock(sock), terminate(terminate), sockThrHolder(clientSockThr)
    {}
    
    
     // destructor
    ~SocketThread()
    {this->terminationEvent.Wait();} // wait for termination event signal


    // returns client socket the thread is handling 
    Socket& GetSocket()
    {
        return sock;
    }

    // declares virtual function and returns a long
    virtual long ThreadMain() {

	// This is the start of exception handling
        try
        {
            // loop that will run while terminate is true
            while(!terminate)
            {
                // reading from socket and storing in variable
                sock.Read(info);

                // Perform operations on the data
                std::string result = info.ToString(); // new string with data 
                
                // This turns everything to upper case
                std::for_each(result.begin(), result.end(), [](char & result){
                    result = ::tolower(result);
                    });
                
		
		// checking if "DONE" was entered
                if (result=="DONE") {
                
                    // removing object from vector
                    sockThrHolder.erase(std::remove(sockThrHolder.begin(), sockThrHolder.end(), this), sockThrHolder.end());
                    
                    terminate = true; 
                    
                    break;      
                }
                
                
                result.append("-received"); // appending to "res" string
                
                // Send it back
                sock.Write(ByteArray(result)); // writing to socket

            }
            
        // two try catch block
        }catch (std::string &s) {
        
            std::cout<<s<<std::endl;
        }
		
        catch (std::string err) // catching by value
        {
            std::cout<<err<<std::endl;
        }
        
        
        std::cout<<"Client Disconnected" <<std::endl;
        
        
        return 0;
    }
};

// delcare ServerThread class and inherit Thread class
class ServerThread : public Thread
{

// declaring private member variables
private:
    SocketServer& server; // refrence to SocketServer that thread will interact with
    
    bool terminate = false; // flag to indicate termination
    
    std::vector<SocketThread*> sockThrHolder; // holds pointers to SocketThread
    
    
/*
This is a public constructor that has a SocketServer object as a parameter
The constructor intializes the member variable with passed-in parameter
*/    
public:
    ServerThread(SocketServer& server)
    : server(server)
    {}

    // This is the destructor 
    ~ServerThread()
    {
        /*
        iterate over sockThrHolder and attemps to close each SocketThread
        */
        for(auto thread: sockThrHolder) {
            // handling exceptions thrown 
            try{
                Socket& toClose = thread->GetSocket();
                
                toClose.Close();
                
            } catch (...) {

            }
        } 
        std::vector<SocketThread*>().swap(sockThrHolder); // clearing vector
        
        std::cout<<"Closing the client from the server"<<std::endl;
        
        terminate = true;
        
    }


    // declare virtual function that returns long
    virtual long ThreadMain()
    {
    	// infinte loop
        while (1) { 
        
            // handling exception
            try {
            
            
                // Wait for a client to connect and create new socket object
                // pointer to new object is stored in newConnection
                Socket* newConnection = new Socket(server.Accept());


		// create new object to synchronize access to server
		// The semaphore initalized with count of 1
                ThreadSem serverBlock(1);

                // create refrence to new Socket object 
                Socket& socketReference = *newConnection;
                
                // create new SocketThread, stored in sockThrHolder
                sockThrHolder.push_back(new SocketThread(socketReference, terminate, std::ref(sockThrHolder)));
                
            // catching errors and printing to console 
            } catch (std::string error)
            {
                std::cout << "error: " << error << std::endl;
				
                return 1;
            }
			
			// catching terminalException and printing to console
			catch (TerminationException terminationException)
			{
				std::cout << "Server has been shut down!" << std::endl;
				
				return terminationException;
			}
        }
        return 0;
    }
};



int main(void)
{
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to terminate the server...";
    std::cout.flush();
	
    // This creates the server on port 3000
    SocketServer server(3000);    

    // creating thread for server options
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    
    cinWaiter.Wait(); // blocks the program until input is received
    
    std::cin.get(); // wait for "enter" and discard

    // Shut down and clean up the server
    server.Shutdown();

}
