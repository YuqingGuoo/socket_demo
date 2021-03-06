#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h> //usleep
#include <time.h>

#define checkError(ret) do{if(-1==ret){printf("[%d]err:%s\n", __LINE__, strerror(errno));exit(1);}}while(0)

unsigned char auSendData[13] = {0};
	
int main(int argc, char const *argv[])
{
	printf("this is tcp demo\n");

	int iSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	checkError(iSocketFd);

	int re = 1;
	checkError(setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(re)));

	struct sockaddr_in server_addr;  
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);        /*receive any address*/
    server_addr.sin_port = htons(7878);
    checkError(bind(iSocketFd, (struct sockaddr*)&server_addr, sizeof(server_addr)));

    checkError(listen(iSocketFd, 5));

    int iSockClient;
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);

reconnect:
	iSockClient = accept(iSocketFd, (struct sockaddr*)&client_addr, &client_len);
	checkError(iSockClient);	
	printf("client ipaddr:%s\n", inet_ntoa(client_addr.sin_addr));

	const char *aSend = "This is tcp server";
	unsigned char aRecv[2048] = {0};	
    while(1)
    {
    	printf("wait client data...\n");
    	int irecv = recv(iSockClient, aRecv, sizeof(aRecv), 0);
		if(-1 == irecv){
			printf("recv err:%s\n", strerror(errno));
			if(errno == EAGAIN){
				usleep(100);
				continue;
			} else {
				exit(1);
			}
		} else if (0 == irecv){
			printf("disconnect with client\n");
			close(iSockClient);
			goto reconnect;
		}
		printf("recv client ip:%s, data:", inet_ntoa(client_addr.sin_addr));
        int i = 0;
        for(i = 0; i < irecv; i++){
            printf("0x%02x,", aRecv[i]);
        }printf("\n");
		
		int iTime = time((time_t*)NULL);
		memcpy(auSendData, &iTime, sizeof(iTime));
		auSendData[4] = 0;
		auSendData[5] = 201;
		auSendData[6] = 6;
		int iDeviceID = 0x00000C060611;
		memcpy(&auSendData[7], &iDeviceID, 6);
		checkError(send(iSockClient, auSendData, sizeof(auSendData), 0));
		sleep(1);
    }

	return 0;
}
