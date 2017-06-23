06/23/2017

The following files are for a WinSock Client/Server Application where the Client sends a long integer to a Server and the Server reurns a boolean that says whether or not the number is a Prime number.
Compiler used was TDM_GCC 5.1  C++ Version 14

Testing was done in 2 ways.
1.  Hard code a 10 digit integer into the client and send 3 of these to the Server.
Output is found in the log file log_file.txt.
2.  Create a batch file that launches 5 different clients each of which send 3 random number to the server.  Again output is found in log_file.txt.

Files:
PrimeServer.cpp-   Server Code
PrimeClient.cpp    Client Code
makewin.bat-       batch file to build the server
makewinclient.bat  batch file to build the client
run_multiple_clients.bat  batch file to run 5 simultaneous clients
log_file.txt       Log output from tests run
README.txt         This file
