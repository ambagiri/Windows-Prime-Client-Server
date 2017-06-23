//
//   This WinSock Server accepts a number from multiple clients, determines the primality
//   of the number and returns a boolean to the client
//   
//
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
 
#define PORT 5150
#define DATA_BUFSIZE  100
#define MAX_CLIENTS 30
 
typedef struct _SOCKET_INFORMATION {
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   SOCKET Socket;
   OVERLAPPED Overlapped;
   DWORD BytesSEND;
   DWORD BytesRECV;
   BOOL IsPrime;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;
 
// Prototypes
BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Index);
BOOL DetPrime(long int);
 
// Global var
DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[MAX_CLIENTS];
 
int main(int argc, char **argv)
{
   SOCKET ListenSocket;
   SOCKET AcceptSocket;
   SOCKADDR_IN InternetAddr;
   WSADATA wsaData;
   INT Ret;
   FD_SET WriteSet;
   FD_SET ReadSet;
   DWORD i;
   INT Total;
   ULONG NonBlock;
   DWORD Flags;
   DWORD SendBytes;
   DWORD RecvBytes;
 
   if ((Ret = WSAStartup(0x0202,&wsaData)) != 0)
   {
      printf("WSAStartup() failed with error %d\n", Ret);
      WSACleanup();
      return 1;
   }
   else
      printf("WSAStartup() is fine!\n");
 
   // Prepare a socket to listen for connections
   if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
   {
      printf("WSASocket() failed with error %d\n", WSAGetLastError());
      return 1;
   }
   else
      printf("WSASocket() is OK!\n");
 
   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(PORT);
 
   if (bind(ListenSocket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return 1;
   }
   else
               printf("bind() is OK!\n");
 
   if (listen(ListenSocket, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return 1;
   }
   else
               printf("listen() is OK!\n");
 
   // Change the socket mode on the listening socket from blocking to
   // non-block so the application will not block waiting for requests
   NonBlock = 1;
   if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
   {
      printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
      return 1;
   }
   else
      printf("ioctlsocket() is OK!\n");
 
   while(TRUE)
   {
      // Prepare the Read and Write socket sets for network I/O notification
      FD_ZERO(&ReadSet);
      FD_ZERO(&WriteSet);
 
      // Always look for connection attempts
      FD_SET(ListenSocket, &ReadSet);
 
      // Set Read and Write notification for each socket 
      for (i = 0; i < TotalSockets; i++)
      {
          FD_SET(SocketArray[i]->Socket, &ReadSet);
      }	  
 
      if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
      {
         printf("select() returned with error %d\n", WSAGetLastError());
         return 1;
      }
      else
         printf("select() is OK!\n");
 
      // Check for arriving connections on the listening socket.
      if (FD_ISSET(ListenSocket, &ReadSet))
      {
         Total--;
         if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
         {
            // Set the accepted socket to non-blocking mode so the server will
            // not get caught in a blocked condition on WSASends
            NonBlock = 1;
            if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
            {
               printf("ioctlsocket(FIONBIO) failed with error %d\n", WSAGetLastError());
               return 1;
            }
            else
               printf("ioctlsocket(FIONBIO) is OK!\n");
 
            if (CreateSocketInformation(AcceptSocket) == FALSE)
            {
                 printf("CreateSocketInformation(AcceptSocket) failed!\n");
                 return 1;
            }
            else
                printf("CreateSocketInformation() is OK!\n");
 
         }
         else
         {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
               printf("accept() failed with error %d\n", WSAGetLastError());
               return 1;
            }
            else {} 
               printf("accept() is fine!\n");
         }
      }
 
      // Check each socket for Read and Write notification until the number
      // of sockets in Total is satisfied
      for (i = 0; Total > 0 && i < TotalSockets; i++)
      {
         LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
 
         // If the ReadSet is marked for this socket then this means data
         // is available to be read on the socket
         if (FD_ISSET(SocketInfo->Socket, &ReadSet))
         {
            Total--;
 
            SocketInfo->DataBuf.buf = SocketInfo->Buffer;
            SocketInfo->DataBuf.len = DATA_BUFSIZE;
 
            Flags = 0;
            if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
            {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  printf("WSARecv() failed with error %d\n", WSAGetLastError());
                  FreeSocketInformation(i);
               }
               else
                 printf("WSARecv() is OK!\n");
               continue;
            }
            else
            {
               SocketInfo->BytesRECV = RecvBytes;
 
               // If zero bytes are received, this indicates the peer closed the connection.
               if (RecvBytes == 0)
               {
                  FreeSocketInformation(i);
                  continue;
               }
	       else
	       {
                  long int LongNum = atol((const char *)SocketInfo->Buffer);
		  
                  // Print received number
		  printf("Received number: %ld\n", LongNum);
		  SocketInfo->IsPrime = DetPrime(LongNum);
                  SocketInfo->Buffer[0] = 0;  
               }                   
            }
         }
 

         if (SocketInfo->IsPrime)
         {		    
             SocketInfo->DataBuf.buf[0] = '1';
         }   
         else
         {		    
             SocketInfo->DataBuf.buf[0] = '0';
         }	       

 
         SocketInfo->DataBuf.len = 1;
         if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
         {
               if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  printf("WSASend() failed with error %d\n", WSAGetLastError());
                  FreeSocketInformation(i);
               }
               else
                  printf("WSASend() is OK!\n");
 
               continue;
         }
      }
   }
}
 
BOOL CreateSocketInformation(SOCKET s)
{
   LPSOCKET_INFORMATION SI;
 
   printf("Accepted socket number %d\n", s);
 
   if ((SI = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
   {
      printf("GlobalAlloc() failed with error %ld\n", GetLastError());
      return FALSE;
   }
   else
      //printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");
 
   // Prepare SocketInfo structure for use
   SI->Socket = s;
   SI->BytesSEND = 0;
   SI->BytesRECV = 0;
 
   SocketArray[TotalSockets] = SI;
   TotalSockets++;
   return(TRUE);
}
 
void FreeSocketInformation(DWORD Index)
{
   LPSOCKET_INFORMATION SI = SocketArray[Index];
   DWORD i;
 
   closesocket(SI->Socket);
   printf("Closing socket number %d\n", SI->Socket);
   GlobalFree(SI);
 
   // Squash the socket array
   for (i = Index; i < TotalSockets; i++)
   {
      SocketArray[i] = SocketArray[i + 1];
   }
 
   TotalSockets--;
}
 
BOOL DetPrime(long int LongNum)
{

// Use the modulo and square root operations to determine the primality of the Input
//
    bool isPrime = true;
    int ans;
    if (LongNum == 0 || LongNum == 1)
        ans = LongNum;

    // Do Binary Search for Floor(sqrt(LongNum))
    int start = 1, end = LongNum;
    while ( start <= end)
    {
        int mid = (start + end) / 2;

        // If LongNum is a perfect sqrt
	if (mid * mid == LongNum)
	   ans = mid;

	//Since we need floor, we update answer when mid*mid is
	//smaller than LongNum and approximate sqrt(LongNum)
	//
	if (mid*mid < LongNum)
        {
		start = mid + 1;
		ans = mid;
        }
	else // If mid*mid is greater than LongNum
		end = mid - 1;
     }

     // Ans has the floor(sqrt(LongNum))
     //
     for (int i = 2; i <= ans; ++i)
     {
         if(LongNum % i == 0)
         {
	     isPrime = false;
             break;
         }
     }
     return isPrime;
}     






	     
	     
	     
     















	    
	    











	

 

 
