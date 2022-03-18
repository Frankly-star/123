#include<iostream>
#include<cstring>
#include"des.h"
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define BUFFERSIZE 64

using namespace std;
char strStdinBuffer[64];
char strSocketBuffer[64];
char strEncryBuffer[64];
char strDecryBuffer[64];
int totalRecv(int s, void* buf, int len, int flags) {
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
				if (fgets(strStdinBuffer, BUFFERSIZE, stdin) == NULL)                      //读取一行
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

int main()
{
	int client_sockfd;
	int len;

	struct sockaddr_in clt_addr;
	char send_buf[BUFSIZ];
	char recv_buf[BUFSIZ];
	memset(&clt_addr, 0, sizeof(clt_addr));
	clt_addr.sin_family = AF_INET;
	clt_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	clt_addr.sin_port = htons(8888);

	if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket error";
		return 1;

	}

	if (connect(client_sockfd, (struct sockaddr*)&clt_addr, sizeof(struct sockaddr)) < 0)
	{
		cout << "connect error";
		return 1;
	}

	cout << "Connect Success!" << endl;
	cout << "Begin to chat..." << endl;
	len = recv(client_sockfd, recv_buf, BUFSIZ, 0);
	recv_buf[len] = '\0';

	while (1)
	{
		cout << "Enter string to send :";
		cin >> send_buf;
		if (!strcmp(send_buf, "quit"))
			break;
		len = send(client_sockfd, send_buf, strlen(send_buf), 0);
		len = recv(client_sockfd, recv_buf, BUFSIZ, 0);
		if (len > 0)
		{
			recv_buf[len] = '\0';
			cout << "Receive message from <127.0.0.1>: " << recv_buf << endl;
		}
	}
	safeChat(client_sockfd, inet_ntoa(clt_addr.sin_addr), "wangzehao");
	close(client_sockfd);

	return 0;

}
