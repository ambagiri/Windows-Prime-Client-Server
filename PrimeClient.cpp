// 
//    This client sends 3 long ints sequentially to a localhost TCP Server listening on port 5150.  The server determines the primality of the
//    numbers and returns a true/false to the client.
//
// Command Line Options: None
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
 
#define DEFAULT_COUNT       3       
#define DEFAULT_PORT        5150
#define DEFAULT_BUFFER      100
 
char  szServer[128],              // Server to connect to
      szMessage[15],           // Number to send to sever
      szClient[12];
int   iPort     = DEFAULT_PORT;    // Port on server to connect to
DWORD dwCount   = DEFAULT_COUNT; // Number of times to send message
 
void write_text_to_log_file(const std::string &txt); 
 
// Function: main
// Description:
//    Main thread of execution. Initialize Winsock, parse the
//    command line arguments, create a socket, connect to the
//    server, and then send and receive data.
int main(int argc, char **argv)
{
    WSADATA       wsd;
    SOCKET        sClient;
    char          szBuffer[DEFAULT_BUFFER];
    int           ret, i;
    struct sockaddr_in server;
    struct hostent    *host = NULL;
 

    strncpy(szServer,"localhost",sizeof("localhost")); 
    // Seed for random number generation
    //
    srand(time(NULL));
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        printf("Failed to load Winsock library! Error %d\n", WSAGetLastError());
        return 1;
    }
    else
        printf("Winsock library loaded successfully!\n");
 
    // Create the socket, and attempt to connect to the server
    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sClient == INVALID_SOCKET)
    {
        printf("socket() failed with error code %d\n", WSAGetLastError());
        return 1;
    }
    else
        printf("socket() looks fine!\n");
 
    server.sin_family = AF_INET;
    server.sin_port = htons(iPort);
    server.sin_addr.s_addr = inet_addr(szServer);
 
    // If the supplied server address wasn't in the form
    // "aaa.bbb.ccc.ddd" it's a hostname, so try to resolve it
    if (server.sin_addr.s_addr == INADDR_NONE)
    {
        host = gethostbyname(szServer);
        if (host == NULL)
        {
            printf("Unable to resolve server %s\n", szServer);
            return 1;
        }
        else
            printf("The hostname resolved successfully!\n");
 
        CopyMemory(&server.sin_addr, host->h_addr_list[0], host->h_length);
    }
 
    if (connect(sClient, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("connect() failed with error code %d\n", WSAGetLastError());
        return 1;
    }
    else
        printf("connect() is fine!\n");
 
    // Send and receive data
    printf("Sending and receiving data if any...\n");
    for(i = 0; i < (int)dwCount; i++)
    {
	// Generate a random number and determine its primality
	//
	long int random = rand();
        // Convert to ascii base 10
	//
	ltoa(random, szMessage, 10);

        ret = send(sClient, szMessage, strlen(szMessage), 0);
        if (ret == 0)
            break;
        else if (ret == SOCKET_ERROR)
        {
            printf("send() failed with error code %d\n", WSAGetLastError());
            break;
        }
 
        printf("send() should be fine. Send %d bytes\n", ret);

        ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0);

        if (ret == 0)        // Graceful close
        {
            printf("It is a graceful close!\n");
            break;
        }
        else if (ret == SOCKET_ERROR)
        {
            printf("recv() failed with error code %d\n", WSAGetLastError());
            break;
        }
        szBuffer[ret] = '\0';
        printf("recv() is OK. Received %d bytes: %s\n", ret, szBuffer);
        std::string s1(szMessage);
	std::string s2;
            
        if (szBuffer[0] == '0')
        {		    
	    s2 = s1 + " is Not a Prime Number ";
        }		    
	else
        {		    
	    s2 = s1 + " is a Prime Number ";
	}		    
	ltoa(sClient, szClient, 10);
	std::string s3(szClient);
        std::string s4;
        s4 = s2 + s3 + " is the Client ID";	
        write_text_to_log_file(s4);
    }
 
    if(closesocket(sClient) == 0)
            printf("closesocket() is OK!\n");
    else
            printf("closesocket() failed with error code %d\n", WSAGetLastError());
 
    if (WSACleanup() == 0)
            printf("WSACleanup() is fine!\n");
    else
            printf("WSACleanup() failed with error code %d\n", WSAGetLastError());
 
    return 0;
}

void write_text_to_log_file(const std::string &text)
{
      std::ofstream log_file(
	       "log_file.txt", std::ios_base::out | std::ios_base::app );
      log_file  << text << std::endl;
}      
		     
		      



	

