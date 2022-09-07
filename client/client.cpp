#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <csignal>
#include<fstream>
#include"CRC8.h"
#include <sstream> 
#pragma comment(lib, "ws2_32.lib")

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



long long int getSize(string s)
{

	FILE* fp = fopen(s.c_str(), "r");
	if (fp == NULL) {
		cout << "file doesnt exist \n";
		return -1;
	}
	fseek(fp, 0L, SEEK_END);
	long int ans = ftell(fp);
	fclose(fp);
	return ans;
}

int main()
{
	string ipAddress = "125.99.146.239";			// IP Address of the server
	int port = 54000;						// Listening port # on the server

	// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return -1;
	}


	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(54000); // Little to big endian conversion
	inet_pton(AF_INET, "125.99.146.239", &server.sin_addr); // Convert from string to byte array

	// Socket creation, note that the socket type is datagram
	SOCKET udpSock = socket(AF_INET, SOCK_DGRAM, 0);

	// Write udpSock to that socket
	// register signal SIGINT and signal handler  
	signal(SIGINT, signalHandler);
	int serverSize = sizeof(server);
		string s;
		cout << "Enter the file name: ";
		cin >> s;
		
		fstream fs;
		fs.open(s);
		if (!fs) {
			cerr << "Error opening file " << s << endl;
			return -1;
		}
		//here s will be filename + size
		int size = getSize(s);
		s += " " + to_string(size);
		string t = crc8send(s);
		int sendOk = sendto(udpSock, t.c_str(), t.size() + 1, 0, (sockaddr*)&server,serverSize );
		if (sendOk == SOCKET_ERROR)
		{
			cout << "Error sending name " << WSAGetLastError() << endl;
			return -1;
		}
		cout << "---------------File transfer Starting!!" << endl;
		//now start sending the file
		while (size > 0) {
			
			char buff[1024];
			int xx = min(1000, size);
			fs.read(buff,xx);//store the position
			buff[xx] = '\0';
			string p = crc8send(string(buff));
			sendOk = sendto(udpSock, p.c_str(), p.length()+1 , 0, (sockaddr*)&server, serverSize);
			if (sendOk == SOCKET_ERROR)
			{
				cout << "Error sending File " << WSAGetLastError() << endl;
				return -1;
			}

			int bytesIn = recvfrom(udpSock, buff, 1024, 0, (sockaddr*)&server, &serverSize);
			if (bytesIn == SOCKET_ERROR)
			{
				cout << "Error receiving from client " << WSAGetLastError() << endl;
				continue;
			}
			string a=BinaryStringToText(string(buff));
			if (a == "yes") {
				//reducing the size
				size -= min(1024, size);
			}
			else if(a=="no") {
				fs.seekp(-min(size, 1024), std::ios::cur);
				//going back to the start of this loop
			}
			else {
				cerr << "Something went wrong aborting!" << endl;
				return -1;
			}
		}
		cout << "---------------FIle SEnt!!" << endl;
	// Close the socket
	closesocket(udpSock);

	WSACleanup();
	return 0;
}