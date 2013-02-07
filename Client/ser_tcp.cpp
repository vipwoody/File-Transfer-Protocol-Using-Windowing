//    SERVER TCP PROGRAM
// revised and tidied up by
// J.W. Atwood
// 1999 June 30
// There is still some leftover trash in this code.

/* send and receive codes between client and server */
/* This is your basic WINSOCK shell */
#pragma comment( linker, "/defaultlib:ws2_32.lib" )
#include <winsock2.h>
#include <ws2tcpip.h>

#include <winsock.h>
#include <iostream>
#include <windows.h>

#include <fstream>

using namespace std;

//port data types

#define REQUEST_PORT 0x7070

int port=REQUEST_PORT;

//socket data types
SOCKET s;

SOCKET s1;
SOCKADDR_IN sa;      // filled by bind
SOCKADDR_IN sa1;     // fill with server info, IP, port
union {struct sockaddr generic;
struct sockaddr_in ca_in;}ca;

int calen=sizeof(ca); 

//buffer data types

char szbuffer[1024];

char *buffer;
int ibufferlen;
int ibytesrecv;

int ibytessent;

//host data types

char localhost[11];

HOSTENT *hp;

//wait variables

int nsa1;

int r,infds=1, outfds=0;

struct timeval timeout;

const struct timeval *tp=&timeout;

fd_set readfds;

//others

HANDLE test;

DWORD dwtest;

//packet

struct Packet {
	char header[8];
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

	//Blank packet
	Packet packet;
	memset(&packet,0,1024*sizeof(char));

	try{        		 
		if (WSAStartup(0x0202,&wsadata)!=0){  
			cout<<"Error in starting WSAStartup()\n";
		}else{
			buffer="WSAStartup was suuccessful\n";   
			WriteFile(test,buffer,sizeof(buffer),&dwtest,NULL); 

			/* display the wsadata structure */
			cout<< endl
				<< "wsadata.wVersion "       << wsadata.wVersion       << endl
				<< "wsadata.wHighVersion "   << wsadata.wHighVersion   << endl
				<< "wsadata.szDescription "  << wsadata.szDescription  << endl
				<< "wsadata.szSystemStatus " << wsadata.szSystemStatus << endl
				<< "wsadata.iMaxSockets "    << wsadata.iMaxSockets    << endl
				<< "wsadata.iMaxUdpDg "      << wsadata.iMaxUdpDg      << endl;
		}  

		//Display info of local host

		gethostname(localhost,10);
		cout<<"hostname: "<<localhost<< endl;

		if((hp=gethostbyname(localhost)) == NULL) {
			cout << "gethostbyname() cannot get local host info?"
				<< WSAGetLastError() << endl; 
			exit(1);
		}

		//Create the server socket
		if((s = socket(AF_INET,SOCK_STREAM,0))==INVALID_SOCKET) 
			throw "can't initialize socket";
		// For UDP protocol replace SOCK_STREAM with SOCK_DGRAM 


		//Fill-in Server Port and Address info.
		sa.sin_family = AF_INET;
		sa.sin_port = htons(port);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);


		//Bind the server port

		if (bind(s,(LPSOCKADDR)&sa,sizeof(sa)) == SOCKET_ERROR)
			throw "can't bind the socket";
		cout << "Bind was successful" << endl;

		//Successfull bind, now listen for client requests.

		if(listen(s,10) == SOCKET_ERROR)
			throw "couldn't  set up listen on socket";
		else cout << "Listen was successful" << endl;

		FD_ZERO(&readfds);

		//wait loop

		while(1)

		{

			FD_SET(s,&readfds);  //always check the listener

			if(!(outfds=select(infds,&readfds,NULL,NULL,tp))) {}

			else if (outfds == SOCKET_ERROR) throw "failure in Select";

			else if (FD_ISSET(s,&readfds))  cout << "got a connection request" << endl; 

			//Found a connection request, try to accept. 

			if((s1=accept(s,&ca.generic,&calen))==INVALID_SOCKET)
				throw "Couldn't accept connection\n";

			//Connection request accepted.
			cout<<"accepted connection from "<<inet_ntoa(ca.ca_in.sin_addr)<<":"
				<<hex<<htons(ca.ca_in.sin_port)<<endl;

			/*//Fill in packet from accepted request.
			if((ibytesrecv = recv(s1,szbuffer,1024,0)) == SOCKET_ERROR)
				throw "Receive error in server program\n";*/

			//Get file name
			memset(szbuffer,0,1024*sizeof(char));
			if((ibytesrecv = recv(s1,szbuffer,1024,0)) == SOCKET_ERROR)
				throw "Receive error in server program\n";;
			memset(&packet,0,1024*sizeof(char));
			memcpy(packet.header,szbuffer,8*sizeof(char));
			memcpy(packet.message,szbuffer+8*sizeof(char),1016*sizeof(char));
			cout << packet.header << "\n";
			cout << packet.message << "\n";

			/*//Print reciept of successful message. 
			cout << "This is message from client: " << packet.message << endl;

			//Send to Client the received message (echo it back).
			ibufferlen = strlen(szbuffer);

			if((ibytessent = send(s1,szbuffer,ibufferlen,0))==SOCKET_ERROR)
				throw "error in send in server program\n";
			else cout << "Echo message:" << packet.message << endl; */

			string header(packet.header);
			char location[1500];

			if(header.compare("RETR") == 0) {
				sprintf_s(location, packet.message);
				cout << "RETR: " << location << "\n";
				ifstream file (location, ios::in|ios::binary|ios::ate);
				ifstream::pos_type size;
				if(file.is_open()) {
					size = file.tellg();
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					itoa((int)size,packet.message,10);
					memcpy(szbuffer,packet.header,8*sizeof(char));
					memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
					//Send size
					if((ibytessent = send(s1,szbuffer,1024,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					cout << "Size: " << atoi(packet.message) << "bytes\n";
					file.seekg(0, ios::beg);
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					while(file.read(packet.message,1016*sizeof(char))) {
						memcpy(szbuffer,packet.header,8*sizeof(char));
						memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
						//Send packet
						if((ibytessent = send(s1,szbuffer,1024,0))==SOCKET_ERROR)
							throw "error in send in server program\n";
						memset(packet.message,0,1016*sizeof(char));
						memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
						size -= 1016;
					}
					memcpy(szbuffer,packet.header,8*sizeof(char));
					memcpy(szbuffer+8*sizeof(char),packet.message,size*sizeof(char));
					//Send last packet                             
					if((ibytessent = send(s1,szbuffer,1024,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					file.close();
					cout << "Transfer Complete\n";
				}
				else {
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					//Send last packet
					memcpy(szbuffer,packet.header,8*sizeof(char));
					memcpy(szbuffer+8*sizeof(char),packet.message,1016*sizeof(char));
					if((ibytessent = send(s1,szbuffer,1024,0))==SOCKET_ERROR)
						throw "error in send in server program\n";
					cout << "No such file.\n";
					
				}
			} else if (header.compare("STOR") == 0) {
				sprintf_s(location, packet.message);
				ofstream file (location, ios::out);
				file.close();
				file.open(location, ios::out|ios::binary|ios::trunc);
				memset(packet.message,0,1016*sizeof(char));
				memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
				//Get file size
				if((ibytesrecv = recv(s1,szbuffer,1024,0)) == SOCKET_ERROR)
					throw "Receive error in server program\n";
				memcpy(packet.message, szbuffer+8*sizeof(char), 1016*sizeof(char));
				ofstream::pos_type size;
				cout << "Size: " << atoi(packet.message) << "bytes\n";;
				for (size = atoi(packet.message); size > 1016*sizeof(char); size -= 1016) {
					memset(packet.message,0,1016*sizeof(char));
					memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
					//Get packet
					if((ibytesrecv = recv(s1,szbuffer,1024,0)) == SOCKET_ERROR)
						throw "Receive error in server program\n";
					memcpy(packet.message, szbuffer+8*sizeof(char), 1016*sizeof(char));
					file.write(packet.message, 1016*sizeof(char));
				}
				memset(packet.message,0,1016*sizeof(char));
				memset(szbuffer+8*sizeof(char),0,1016*sizeof(char));
				//Get last packet
				if((ibytesrecv = recv(s1,szbuffer,1024,0)) == SOCKET_ERROR)
					throw "Receive error in server program\n";
				memcpy(packet.message, szbuffer+8*sizeof(char), size*sizeof(char));
				file.write(packet.message, size*sizeof(char));
				file.close();
				cout << "Transfer Complete\n";
			} else {

			}
		}//wait loop

	} //try loop

	//Display needed error message.

	catch(char* str) { cerr<<str<<WSAGetLastError()<<endl;}

	//close Client socket
	closesocket(s1);		

	//close server socket
	closesocket(s);

	/* When done uninstall winsock.dll (WSACleanup()) and exit */ 
	WSACleanup();
	system("pause");
	return 0;
}




