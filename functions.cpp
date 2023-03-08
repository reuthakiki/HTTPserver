#include "HTTPserver.h"

//general functions - nonblocking server
bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			socketsCount++;
			return (true);
		}
	}
	return (false);
}
void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}
void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	//
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}
void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;

		if (sockets[index].len > 0)
		{
			if (strncmp(sockets[index].buffer, "TimeString", 10) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = SEND_TIME;
				memcpy(sockets[index].buffer, &sockets[index].buffer[10], sockets[index].len - 10);
				sockets[index].len -= 10;
				return;
			}
			else if (strncmp(sockets[index].buffer, "SecondsSince1970", 16) == 0)
			{
				sockets[index].send = SEND;
				sockets[index].sendSubType = SEND_SECONDS;
				memcpy(sockets[index].buffer, &sockets[index].buffer[16], sockets[index].len - 16);
				sockets[index].len -= 16;
				return;
			}
			else if (strncmp(sockets[index].buffer, "Exit", 4) == 0)
			{
				closesocket(msgSocket);
				removeSocket(index);
				return;
			}
		}
	}

}
void sendMessage(int index)
{
	int bytesSent = 0;
	char sendBuff[255];

	SOCKET msgSocket = sockets[index].id;
	if (sockets[index].sendSubType == SEND_TIME)
	{
		// Answer client's request by the current time string.

		// Get the current time.
		time_t timer;
		time(&timer);
		// Parse the current time to printable string.
		strcpy(sendBuff, ctime(&timer));
		sendBuff[strlen(sendBuff) - 1] = 0; //to remove the new-line from the created string
	}
	else if (sockets[index].sendSubType == SEND_SECONDS)
	{
		// Answer client's request by the current time in seconds.

		// Get the current time.
		time_t timer;
		time(&timer);
		// Convert the number to string.
		itoa((int)timer, sendBuff, 10);
	}

	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

	sockets[index].send = IDLE;
}
//

void findAndUpdateTheRequest(char* i_BuffRequest, int i_IndexOfSokcet)
{
	char startOfBuffReq[SIZEOFBUFF];
	strcpy(startOfBuffReq, i_BuffRequest);
	sockets[i_IndexOfSokcet].buffeReq = i_BuffRequest;
	strtok(i_BuffRequest, " ");//we skip on type request
	sockets[i_IndexOfSokcet].fileName = strtok(NULL, " ");//We will extract the file name
	sockets[i_IndexOfSokcet].fileName.insert(0, "C:\\temp");
	strcpy(i_BuffRequest, startOfBuffReq);

	if (strncmp(startOfBuffReq, "OPTIONS", 7) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = OPTIONS;
	}
	else if (strncmp(startOfBuffReq, "GET", 3) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = GET;
	}
	else if (strncmp(startOfBuffReq, "HEAD", 4) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = HEAD;
	}
	else if (strncmp(startOfBuffReq, "POST", 4) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = POST;
	}
	else if (strncmp(startOfBuffReq, "PUT", 3) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = PUT;
	}
	else if (strncmp(startOfBuffReq, "DELETE", 6) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = DDELETE;
	}
	else if (strncmp(startOfBuffReq, "TRACE", 5) == 0)
	{
		sockets[i_IndexOfSokcet].send = SEND;
		//sockets[i_IndexOfSokcet].carryRequest = TRACE;
	}
}
void PUT()
{
	string sendbuff, HttpHead, AllMessage, languageOpt;
	int lenSendBuff = 0;
	int socketNum;	
	HEAD(socketNum , &HttpHead, &lenSendBuff);

	if (sockets[socketNum].fileName.find("?lang") != EOF)
	{
		AddLangFile(socketNum);	//ask neta if its enough ??
	}

	else
	{
		sockets[socketNum].fileName.insert(sockets[socketNum].fileName.find("."), "-en"); //add english file 
	}

	AllMessage = OpenFile(sockets[socketNum].fileName, const_cast<char*>(sockets[socketNum].buffeReq.c_str()));
	AllMessage += HttpHead;
}

string OpenFile(string fileName, char* buffer)
{
	ofstream File;
	buffer = buffer + ((string)buffer).find("\n") + 3;	// why 3? didnt find anser
	File.open(fileName, ios::in);
	if (!File.is_open())
	{
		File.open(fileName, ios::trunc);
		string valid = checkValidFile(File);
		if (valid == STATUS_PRECONDITION) {
			return valid;
		}
		if (strlen(buffer) != 0)
		{
			return writeToFileAfterError(buffer, File);
		}
		else
		{
			checkLenBuffer(buffer, File);
		}
	}
	else
	{
		if (strlen(buffer) != 0)
		{
			writeToFile(buffer, File);
		}
		else
		{
			checkLenBuffer(buffer, File);
		}
	}

}

string checkValidFile(ofstream& fileName)
{
	if (!fileName.is_open())
	{
		return STATUS_PRECONDITION;
	}
	return "notFailed";
}

string writeToFileAfterError(char* buffer, ofstream& File)
{
	while ((*buffer) != 0)
	{
		File << (*buffer);
		buffer++;
	}
	if (File.fail())
		return STATUS_NOTIMPLEMENTED;
	File.close();
	return STATUS_CREATED;

}

string writeToFile(char* buffer, ofstream& File)
{
	while ((*buffer) != 0)
	{
		File << (*buffer);
		buffer++;
	}
	if (File.fail())
		return STATUS_NOTIMPLEMENTED;
	File.close();
	return STATUS_OK;

}

string checkLenBuffer(char* buffer, ofstream& File)
{
	if (File.fail())
		return STATUS_NOTIMPLEMENTED;
	File.close();
	return STATUS_NOCONTENT;
}
/// <summary>
/// /
/// </summary>
void Delete()
{
	string sendbuff, HttpHead //from outside
		, AllMessage;
	int lenSendBuff = 0;
	int socketNum;	//from outside

	HEAD(socketNum, &HttpHead, &lenSendBuff);
	//Send Delete Message
	if (sockets[socketNum].fileName.find("?lang") != EOF)
	{
		AddLangFile(socketNum);
	}

	if (remove(sockets[socketNum].fileName.c_str()) != 0)
	{
		cout << "The file is not deleted" << endl;
		AllMessage = STATUS_NOTFOUND;
	}
	else
	{
		AllMessage = STATUS_NOCONTENT;
	}
	AllMessage += HttpHead;
}

void AddLangFile(int socketNum)
{
	string langSocket;
	// search which page the client want open and change the page to the right language.
	langSocket = sockets[socketNum].fileName.substr(sockets[socketNum].fileName.find("=") + 1);
	sockets[socketNum].fileName.erase(sockets[socketNum].fileName.find("?"));

	AddLangHebrewFile(langSocket, sockets, socketNum);
	AddLangEnglishFile(langSocket, sockets, socketNum);
	AddLangFrenchFile(langSocket, sockets, socketNum);
}

void AddLangHebrewFile(string langToSocket, SocketState* sockets, int numberOfSocket)
{
	// get the page in hebrew.
	if (langToSocket.compare("he") == 0)
	{
		sockets[numberOfSocket].fileName.insert(sockets[numberOfSocket].fileName.find("."), "-he");
	}
}

void AddLangEnglishFile(string langToSocket, SocketState* sockets, int numberOfSocket)
{
	//get the page in english
	if (langToSocket.compare("en") == 0)
	{
	sockets[numberOfSocket].fileName.insert(sockets[numberOfSocket].fileName.find("."), "-en");
	}
}

void AddLangFrenchFile(string langToSocket, SocketState* sockets, int numberOfSocket)
{
	//get the page in french
	if (langToSocket.compare("fr") == 0)
	{
	sockets[numberOfSocket].fileName.insert(sockets[numberOfSocket].fileName.find("."), "-fr");
	}
}

void Trace()
{
	string sendbuff, HttpHead //from outside
		, AllMessage;
	int lenSendBuff = 0;
	int socketNum;	//from outside
	HEAD(socketNum, &HttpHead, &lenSendBuff);
	//Send Messge Type Trace
	sendbuff += sockets[socketNum].buffer;
	lenSendBuff = (sendbuff).size();
	AllMessage = STATUS_OK;
	AllMessage += HttpHead;
	AllMessage += sendbuff;
}

string OPTIONS(int socketNum, string* sendBuff, int* sendBuffLen)
{
	(*sendBuff) += "This web server supports these requests: OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE.\n";
	*sendBuffLen += (*sendBuff).length();
	return STATUS_OK;
}
string GET(int socketNum, string* sendBuff, int* sendBuffLen)
{
	char curr[255];// = new char[SIZEOFBUFF];
	string languageOpt;
	string fileName;
	string status;
	ifstream newFile;

	fileName = sockets[socketNum].fileName;
	if (fileName.find("?lang") != string::npos)
	{
		// found which page the client want open the page
		languageOpt = fileName.substr(fileName.find("=") + 1);
		fileName.erase(fileName.find("?"));
		if (strncmp(languageOpt.c_str(), "he", 2) == 0)
		{
			fileName.insert(fileName.find("."), "-he");
			status = STATUS_OK;
		}
		else if (strncmp(languageOpt.c_str(), "en", 2) == 0)
		{
			fileName.insert(fileName.find("."), "-en");
			status = STATUS_OK;
		}
		else if (strncmp(languageOpt.c_str(), "fr", 2) == 0)
		{
			fileName.insert(fileName.find("."), "-fr");
			status = STATUS_OK;
		}
	}
	else
	{
		fileName.insert(fileName.find("."), "-en");
	}
	newFile.open(fileName);
	if (!newFile.fail())
	{
		while (!newFile.eof())
		{
			//memset(curr, '\0', SIZEOFBUFF); //see if needed
			newFile.getline(curr, SIZEOFBUFF);
			(*sendBuff) += curr;
			(*sendBuffLen) += strlen(curr);
		}
		newFile.close();
		*sendBuff += "\r\n";
		*sendBuffLen += 2;
		status = STATUS_OK;
	}
	else
	{
		newFile.close();
		status = STATUS_NOTFOUND;
	}
	return status;
}

string POST(char* message)
{
	string status;
	cout << "POST request: " << endl;
	size_t result = ((string)message).find("\r\n");
	result += 3;//the location of the body in the request
	message += result;

	cout << message << endl;
	status = STATUS_OK;

	return status;
}

string HEAD(int socketNum, string* sendBuff, int* sendBuffLen)
{
	string status;
	char message[255];
	string time = ctime(&(sockets[socketNum].timeOfRequest));
	string sendBufflen = _itoa((*sendBuffLen), message, 10);
	(*sendBuff) += "Date: " + time + "Server: HTTP\r\n" + "Content-length: " + sendBufflen + "\r\nContent-type: text\\html\r\n\r\n";

	status = STATUS_NOTFOUND;
	return status;
}
