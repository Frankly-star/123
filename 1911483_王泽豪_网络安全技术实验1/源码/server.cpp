#include<iostream>
#include"socket.h"
#include"des.h"
#include<cstring>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define BUFFERSIZE 64
char strStdinBuffer[64];
char strSocketBuffer[64];
char strEncryBuffer[64];
char strDecryBuffer[64];
int  totalRecv(int s, void* buf, size_t len, int flags) {
	int nCurSize = 0;
	while (nCurSize < len) {
		int nRes = recv(s, (char*)buf + nCurSize, len - nCurSize, flags);
		if (nRes<0 || nRes + nCurSize>len) {
			return -1;
		}
		nCurSize += nRes;
	}
	return nCurSize;

}
void safeChat(int nSock, char* pRemoteName, char* pKey)
{
	CDesOperator cDes;
	if (strlen(pKey) != 8)
	{
		printf("Key length error");
		return;
	}
	pid_t nPid;
	nPid = fork();
	if (nPid != 0)
	{
		while (1)
		{
			bzero(&strSocketBuffer, BUFFERSIZE);
			int nLength = 0;
			nLength = totalRecv(nSock, strSocketBuffer, BUFFERSIZE, 0);
			if (nLength != BUFFERSIZE)
			{
				break;
			}
			else
			{
				int nLen = BUFFERSIZE;
				cDes.Decry(strSocketBuffer, BUFFERSIZE, strDecryBuffer, nLen, pKey, 8);
				strDecryBuffer[BUFFERSIZE - 1] = 0;
				if (strDecryBuffer[0] != 0 && strDecryBuffer[0] != '\n')
				{
					printf("Receive message form <%s>: %s\n", pRemoteName, strDecryBuffer);
					if (0 == memcmp("quit", strDecryBuffer, 4))
					{
						printf("Quit!\n");
						break;
					}
				}
			}
		}
	}
	else                                                                                                               //父线程发送消息
	{
		while (1)
		{
			bzero(&strStdinBuffer, BUFFERSIZE);
			while (strStdinBuffer[0] == 0)
			{
				if (fgets(strStdinBuffer, BUFFERSIZE, stdin) == NULL)                     
				{
					continue;
				}
			}
			int nLen = BUFFERSIZE;
			cDes.Encry(strStdinBuffer, BUFFERSIZE, strEncryBuffer, nLen, pKey, 8);
			if (send(nSock, strEncryBuffer, BUFFERSIZE, 0) != BUFFERSIZE)
			{
				perror("send");
			}
			else
			{
				if (0 == memcmp("quit", strStdinBuffer, 4))
				{
					printf("Quit!\n");
					break;
				}
			}
		}
	}
}

using namespace std;
int main()
{
	int server_sockfd;
	int client_sockfd;
	int len;

	struct sockaddr_in srv_addr;
	struct sockaddr_in clt_addr;
	socklen_t sin_size;
	char send_buf[BUFSIZ];
	char recv_buf[BUFSIZ];

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port = htons(8888);

	if ((server_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error";
		return 1;
	}

	if (bind(server_sockfd, (struct sockaddr*)&srv_addr, sizeof(struct sockaddr)) < 0)
	{
		cout << "bind error";
		return 1;
	}
	if (listen(server_sockfd, 5) < 0)
	{
		cout << "listen error";
		return 1;
	}
	cout << "Server is listening......" << endl;
	sin_size = sizeof(struct sockaddr_in);


	if ((client_sockfd = accept(server_sockfd, (struct sockaddr*)&(clt_addr), &sin_size)) < 0)
	{
		cout << "accept error";
		return 1;
	}
	cout << "accept client" << inet_ntoa(clt_addr.sin_addr) << " port(8888)" << endl;
	len = send(client_sockfd, "Welcome to server", 21, 0);

	while (1)
	{
		len = recv(client_sockfd, recv_buf, BUFSIZ, 0);
		if (len > 0)
		{
			recv_buf[len] = '\0';
			cout << "Received:" << recv_buf << endl;

		}
		cout << "Enter string to send: ";
		cin >> send_buf;

		if (!strcmp(send_buf, "quit"))
			break;

		len = send(client_sockfd, send_buf, strlen(send_buf), 0);

	}
	safeChat(client_sockfd, inet_ntoa(clt_addr.sin_addr), "wangzehao");
	close(server_sockfd);
	close(client_sockfd);


	return 0;
}

