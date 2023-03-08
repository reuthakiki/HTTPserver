#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <string>
#include <time.h>
#include  <fstream>
#define SIZEOFBUFF 1024

//status definition:
#define STATUS_OK "HTTP/1.1 200 OK\n"
#define STATUS_NOTFOUND "HTTP/1.1 404 NOT FOUND\n"
#define STATUS_NOCONTENT "HTTP/1.1 204 No Content\n"
#define STATUS_NOTIMPLEMENTED "HTTP/1.1 501 Not Implemented\n"
#define STATUS_CREATED "HTTP/1.1 201 Created\n"
#define STATUS_PRECONDITION "HTTP/1.1 412 Precondition failed\n"

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char* buffer;		// change from char* to string????
	int len;
	string fileName;
	time_t timeOfRequest;
};

const int TIME_PORT = 27015;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);
void findAndUpdateTheRequest(char* i_BuffRequest, int i_IndexOfSokcet);
void PUT();
string OpenFile(string fileName, char* buffer);
void Delete();
string SendDeleteMessge(int socketNum);
void AddLangFile(int socketNum);
void Trace();
string SendMessgeTypeTrace(int index, char* buffer, string* sendbuff, int* lenSendBuff);
string OPTIONS(int socketNum, string* sendBuff, int* sendBuffLen);
string GET(int socketNum, string* sendBuff, int* sendBuffLen);
string POST(char* message);
string HEAD(int socketNum, string* sendBuff, int* sendBuffLen);