// CLIENT TCP PROGRAM
// Revised and tidied up by
// J.W. Atwood
// 1999 June 30



char* getmessage(char *);



/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsock.h>
#include <stdio.h>
#include <iostream>

#include <windows.h>

#include <fstream>
#include <string>

using namespace std;

//user defined port number
#define REQUEST_PORT 0x5000;

int port=REQUEST_PORT;



//socket data types
SOCKET s;
SOCKADDR_IN sa;         // filled by bind
SOCKADDR_IN sa_in;      // fill with server info, IP, port



//buffer data types
char szbuffer[1024];

char *buffer;

int ibufferlen=0;

int ibytessent;
int ibytesrecv=0;



//host data types
HOSTENT *hp;
HOSTENT *rp;

char localhost[11],
     remotehost[11];


//other

HANDLE test;

DWORD dwtest;


//packet

struct Packet {
	char header[7];
	char sequence[1];
	char message[1016];
};


//reference for used structures

/*  * Host structure

    struct  hostent {
    char    FAR * h_name;             official name of host *
    char    FAR * FAR * h_aliases;    alias list *
    short   h_addrtype;               host address type *
    short   h_length;                 length of address *
    char    FAR * FAR * h_addr_list;  list of addresses *
#define h_addr  h_addr_list[0]            address, for backward compat *
};

 * Socket address structure

 struct sockaddr_in {
 short   sin_family;
 u_short sin_port;
 struct  in_addr sin_addr;
 char    sin_zero[8];
 }; */


int main(void){

	WSADATA wsadata;
	
	//Blank Packet
	Packet packet;
	memset(&packet,0,1024*sizeof(char));

	try {

		if (WSAStartup(0x0202,&wsadata)!=0){  
			cout<<"Error in starting WSAStartup()" << endl;
		} else {
			buffer="WSAStartup was successful\n";   
			WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 

			/* Display the wsadata structure */
			cout<< endl
				<< "wsadata.wVersion "       << wsadata.wVersion       << endl
				<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
				<< "wsadata.szDescription "  << wsadata.szDescription  << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
				<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
		}  


		//Display name of local host.

		gethostname(localhost,10);
		cout<<"Local host name is \"" << localhost << "\"" << endl;

		if((hp=gethostbyname(localhost)) == NULL) 
			throw "gethostbyname failed\n";

		//Ask for name of remote server

		cout << "please enter your remote server name :" << flush ;   
		cin >> remotehost ;
		cout << "Remote host name is: \"" << remotehost << "\"" << endl;

		if((rp=gethostbyname(remotehost)) == NULL)
			throw "remote gethostbyname failed\n";

		//Create the socket
		if((s = socket(AF_INET,SOCK_DGRAM,0))==INVALID_SOCKET) 
			throw "Socket failed\n";
		/* For UDP protocol replace SOCK_STREAM with SOCK_DGRAM */

		memset(&sa,0,sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY); //host to network
		//bind the port to the socket
		if (bind(s,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
				throw "can't bind the socket";


		//Specify server address for client to connect to server.
		memset(&sa_in,0,sizeof(sa_in));
		memcpy(&sa_in.sin_addr,rp->h_addr,rp->h_length);
		sa_in.sin_family = rp->h_addrtype;   
		sa_in.sin_port = htons(7000);

		//Display the host machine internet address

		cout << "Connecting to remote host:";
		cout << inet_ntoa(sa_in.sin_addr) << endl;

		//Connect Client to the server
		if (connect(s,(LPSOCKADDR)&sa_in,sizeof(sa_in)) == SOCKET_ERROR)
			throw "connect failed\n";

		ibytessent = sendto(s, (const char*)&packet, sizeof(packet), 0,(struct sockaddr*) &sa_in, sizeof(sa_in));
		//ibytessent = send(s, (const char*)&packet, sizeof(packet), 0);

		/* Have an open connection, so, server is 

		   - waiting for the client request message
		   - don't forget to append <carriage return> 
		   - <line feed> characters after the send buffer to indicate end-of file */

		//append client message to szbuffer + send.

		/*sprintf_s(packet.header,"COMMAND");
		sprintf_s(packet.message,"hello world!\r\n"); 

		ibytessent=0;    
		ibufferlen = 1024;
		
		if (ibytessent = send(s,szbuffer,1024,0);== SOCKET_ERROR)
			throw "Send failed\n";  
		else
			cout << "Message to server: " << szbuffer;

		//wait for reception of server response.
		ibytesrecv=0; 
		if((ibytesrecv = recv(s,szbuffer,1024,0)) == SOCKET_ERROR)
			throw "Receive failed\n";
		else
			cout << "hip hip hoorah!: Successful message replied from server: " << szbuffer;*/

		string choice;
		char filename[1500];
		bool error = true;
		//Get command & file name
		cout << "get or put file? ";
		cin >> choice;
		cout << "file name? ";
		cin >> filename;
		while(error) {
			if(choice.compare("get") == 0) {
				cout << "RETR: " << filename << "\n";
				ofstream file (filename, ios::out);
				file.close();
				file.open(filename, ios::out|ios::binary|ios::trunc);
				ofstream::pos_type size;
				sprintf_s(packet.header, "RETR");
				sprintf_s(packet.message, filename);
				memset(szbuffer,0,1024*sizeof(char));
				memcpy(szbuffer,packet.header,8*sizeof(char));
				memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
				//Send file name
				if (ibytessent = send(s,szbuffer,1024,0) == SOCKET_ERROR)
					throw "Send failed\n";
				memset(packet.message,0,1016*sizeof(char));
				memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
				//Get size
				if((ibytesrecv = recv(s,szbuffer,1024,0)) == SOCKET_ERROR)
					throw "Receive failed\n";
				memcpy(packet.message, szbuffer+8*sizeof(char), 1016*sizeof(char));
				cout << "Size: " << atoi(packet.message) << "bytes\n";
				for (size = atoi(packet.message); size > 1016*sizeof(char); size -= 1016) {
					cout << "Size Remaining: " << size << "bytes\n";
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					//Get packet
					if((ibytesrecv = recv(s,szbuffer,1024,0)) == SOCKET_ERROR)
						throw "Receive failed\n";
					memcpy(packet.message, szbuffer+8*sizeof(char), 1016*sizeof(char));
					file.write(packet.message, 1016*sizeof(char));
				}
				memset(packet.message,0,1016*sizeof(char));
				memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
				//Get last packet
				if((ibytesrecv = recv(s,szbuffer,1024,0)) == SOCKET_ERROR)
					throw "Receive failed\n";
				cout << "Last Packet Size: " << size << "bytes\n";
				memcpy(packet.message, szbuffer+8*sizeof(char), size*sizeof(char));
				file.write(packet.message, size*sizeof(char));
				memset(packet.message,0,1016*sizeof(char));
				memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
				file.close();
				cout << "Transfer Complete!\n";
				error = false;
			} else if(choice.compare("put") == 0) {
				cout << "STOR: " << filename << "\n";
				ifstream file (filename, ios::in|ios::binary|ios::ate);
				ifstream::pos_type size;
				if(file.is_open()) {
					sprintf_s(packet.header, "STOR");
					sprintf_s(packet.message, filename);
					memset(szbuffer,0,1024*sizeof(char));
					memcpy(szbuffer,packet.header,8*sizeof(char));
					memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
					//Send file name
					if (ibytessent = send(s,szbuffer,1024,0) == SOCKET_ERROR)
						throw "Send failed\n";
					size = file.tellg();
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					itoa((int)size,packet.message,10);
					memcpy(szbuffer,packet.header,8*sizeof(char));
					memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
					cout << atoi(packet.message) << "\n";
					//Send size
					if (ibytessent = send(s,szbuffer,1024,0) == SOCKET_ERROR)
						throw "Send failed\n";
					file.seekg(0, ios::beg);
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					while(file.read(packet.message,1016*sizeof(char))) {
						memcpy(szbuffer,packet.header,8*sizeof(char));
						memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
						//Send packet
						if (ibytessent = send(s,szbuffer,1024,0) == SOCKET_ERROR)
							throw "Send failed\n";
						memset(packet.message,0,1016*sizeof(char));
						memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
						size -= 1016;
					}
					memcpy(szbuffer,packet.header,8*sizeof(char));
					memcpy(szbuffer+8*sizeof(char),packet.message,size*sizeof(char));
					//Send last packet
					if((ibytessent = send(s,szbuffer,1024,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					file.close();
					error = false;
				}
				else {
					//No file found stuff
					cout << "File not found or incorrect.\n";
					cout << "\nnew file name? ";
					cin >> filename;
					error = true;
				}
			} else {
				cout << "Invalid command, only get or put accepted!\n";
				cout << "get or put file? ";
				cin >> choice;
				error = true;
			}
		}

	} // try loop

	//Display any needed error response.

	catch (char *str) { cerr<<str<<":"<<dec<<WSAGetLastError()<<endl;}

	//close the client socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();  
	system("pause");
	return 0;
}





