#include <iostream>
#include<winsock2.h>
#include<stdio.h>
#include<string.h>
#include<omp.h> 
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
char buffer,IP[16];
int send_cmd(SOCKET sockfd);
void recv_cmd(SOCKET sockfd);

int main(int argc, char** argv) 
{
	printf("請輸入要連線的IP(IPv4) : ");
	scanf("%[^\n]",&IP);
	
  	SOCKET sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	 
	WSADATA wsadata;
	if(WSAStartup(0x101,(LPWSADATA)&wsadata) != 0) 
	{
	  	printf("Winsock Error\n"); 
	  	exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(IP);//24號同學的固定IP : 122.116.195.140
	address.sin_port = 9734;
	len = sizeof(address);
	 
	result = connect(sockfd, (struct sockaddr *)&address, len);  
	if(result == -1) 
	{
		printf("Connect Error");
		exit(1);
	}
	Sleep(500);
	printf("伺服器已連接~~\n");
	char ch[100];
	sprintf(ch,"加入聊天室");
	buffer = (char)(strlen(ch)+1);
	ch[buffer-1] = '\0';
	send(sockfd, &buffer, 1, 0);
	send(sockfd, ch, buffer, 0);
	#pragma omp parallel sections 
	{
		#pragma omp section
		{
			send_cmd(sockfd);
		}
		#pragma omp section
		{
			recv_cmd(sockfd);	
		}			
	}		
	//closesocket(sockfd);
	exit(0);
}

int send_cmd(SOCKET sockfd)
{
	char char_message[100],*ch;
	char command_send[5]="send ";
	char command_bye[3]="bye";
	while(1){	
		fflush(stdin);
	 	scanf("%[^\n]",char_message);
	 	
	 	int j=0;
	 	while(j<3)
	 	{
	 		
	 		char byelength=4,byeword[]="bye";
	 		if(char_message[j]==command_bye[j])
	 			j++;
	 		else
	 			break;
	 		if(j==3)
	 		{
	 			send(sockfd,&byelength,1,0);
	 			send(sockfd,byeword,4,0);
	 			//Sleep(1000);
	 			closesocket(sockfd);
	 			exit(0);
	 		}
	 	}
	 	bool error = false;
	 	int i=0;
	 	while(i<5)
	 	{
			if(char_message[i]!=command_send[i])
			{
				char errorlength=5,errorword[]="error";
				printf("輸入格式錯誤!請重新輸入\n");
				error = true;
				break;
			}
			i++;
	 	}
	 	if(!error){
	 		ch = (char*)malloc(sizeof(char)*200);
			sprintf(ch,"%s",char_message+5);
			buffer = (char)(strlen(ch)+1);
			ch[buffer-1] = '\0';
			//printf("char = %s\n", ch);
			//printf("buffer = %d\n", buffer);
			send(sockfd, &buffer, 1, 0);
			send(sockfd, ch, buffer, 0);
			free(ch);
	 	}
 	}
	return 0;
}

void recv_cmd(SOCKET sockfd)
{
	while(1){
		char *ch;
		recv(sockfd, &buffer, 1, 0);
		//printf("buffer = %d\n", buffer);
		ch = (char*)malloc(sizeof(char)*buffer);
		recv(sockfd, ch, buffer, 0);
		//printf("char from server = %s\n", ch);
		printf("%s\n", ch);
		//system("pause");
		free(ch);
	}
}
