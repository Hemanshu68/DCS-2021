
// basic i/p header
#include <winsock2.h>
#include <windows.h>              
#include <iostream>
#include<fstream>
//including the winsock header
#include<WS2tcpip.h>
#include<string>
#include <csignal>
#include"CRC.h"
#include <sstream>
#include<vector>
#include <bitset>

//pragma directs to the compiler instruction,it is prefere to use just after the declaration for clarification
#pragma comment (lib, "ws2_32.lib")
#define PORT 54000
using namespace std;




void signalHandler(int signum) {
	cout << "Interrupt signal (" << signum << ") received.\n";

	// cleanup and close up stuff here  
	// terminate program  

	exit(signum);
}

string BinaryStringToText(string binaryString) {
	string text = "";
	stringstream sstream(binaryString);
	while (sstream.good())
	{
		bitset<8> bits;
		sstream >> bits;
		text += char(bits.to_ulong());
	}
	return text.substr(0, text.length() - 2);
}



struct clientinfo {
	string ipAddress;
	string filename;
	int fileSizeRemaining;
};

vector<struct clientinfo> clientsInfo = {}; //ip address ,filename ,size remaining

bool checkPresent(string newIp) {
	for (auto x : clientsInfo) {
		if (x.ipAddress == newIp)
			return true;
	}
	return false;
}

//true for sucessfull write false for unsucessful
bool writeData(string buff, string clientip,int ReceicvedSize) {
	struct clientinfo s;
	
	int i = 0;
	for (i = clientsInfo.size()-1;i >= 0;i--) {
		if (clientsInfo[i].ipAddress == clientip)
		{
			s = clientsInfo[i];
			break;
		}
	}
	//get the client ip filename size in s;
	fstream fs;
	fs.open(s.filename.c_str(), fstream::app | fstream::out);

	if (!fs) {
		cout << "error reading the file " << s.filename << endl;
		return false;
	}
	
	fs << buff;
	fs.close();
	//reduce the remaining size of file
	s.fileSizeRemaining -= ReceicvedSize;
	if (!s.fileSizeRemaining) {
		//now remove the s from global one
		clientsInfo.erase(clientsInfo.begin()+i);
	}
	return true;

}

//tellp() for getting size


int main()
{

	
//initialise winsock

	//winsock main strucure
	WSADATA wsData;
	//creates the version which we will request
	WORD ver = MAKEWORD(2, 2);
	

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		cerr << "Can't Initalize Winsock! Quitting" << endl;
		return -1;
	}
//create socket

	SOCKET udpsock = socket(AF_INET, SOCK_DGRAM, 0);

	if (udpsock == INVALID_SOCKET) {
		cerr << "Can't create socket! Quitting" << endl;
		return -1;
	}


//bind  ip and port to socket
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	//due to big endian and little endian (htnod=host to network short)
	addr.sin_port = htons(PORT);
	//INADDR_ANY do not bind to a specific ip address it accepts everything
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	//binding the socket

	if (bind(udpsock, (sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	
	
	// register signal SIGINT and signal handler  
	signal(SIGINT, signalHandler);
	

	char buff[8*1024];
	cout << "SERVER STARTED!!" << endl;
	while (true) {
		sockaddr_in client; // Use to hold the client information (port / ip address)
		int ClientSize = sizeof(client); // The size of the client information
		ZeroMemory(&client, ClientSize); // Clear the client structure
		ZeroMemory(buff,8* 1024); // Clear the receive buffer

		
		// Wait for message
		int bytesIn = recvfrom(udpsock, buff,8* 1024, 0, (sockaddr*)&client, &ClientSize);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error receiving from client " << WSAGetLastError() << endl;
			continue;
		}
	
		buff[bytesIn - 1] = '\0';

			// Display message and client infon
	char clientIp[256]; // Create enough space to convert the address byte array
	ZeroMemory(clientIp, 256); // to string of characters
	if (crc8recv(string(buff))) {//if no error detected then we send this
			// Convert from byte array to chars
		inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
		cout << "> Data received from client:" << clientIp << endl;
		if (!checkPresent(clientIp)) {
			
			struct clientinfo b;
			b.ipAddress = clientIp;
			string x = BinaryStringToText(string(buff));
			int i = 0;
			for (i = 0;i < x.length();i++) {
				if (x[i] == ' ')
					break;
			}
			
			b.filename = x.substr(0, i);
			b.fileSizeRemaining = stoi(x.substr(i));
			clientsInfo.push_back(b);
			continue;
			//here we received filename and size so we do not send anythng to the client like retransmit or OK.
		}
		
		bool wr = writeData(BinaryStringToText(string(buff)), clientIp, bytesIn);
		string t;
		t = wr ? "yes" : "no";
		t = crc8send(t);
		//because 0 can be size of unit sent so send yes/no to client
		sendto(udpsock, t.c_str(), t.size() + 1, 0, (sockaddr*)&client, ClientSize);
	}
	else {
		//failed so ask it to send again
		string t = "no";
		sendto(udpsock, crc8send(t).c_str(), t.size() + 1, 0, (sockaddr*)&client, ClientSize);
	}

	}


	//cleanup winsock
	WSACleanup();

	return 0;
}




