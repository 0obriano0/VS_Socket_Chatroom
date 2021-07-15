#include <omp.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#define client_num 3

void wait_client(sockaddr_in server_address,SOCKET server_sockfd,int server_len,SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,char *user_leave);
void server_recv(SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,int UUID,char *user_leave);
void server_send(SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,char *user_leave);
void server_time_print(int mode);

int main(int argc, char** argv) {
  SOCKET server_sockfd;
  int server_len, client_len;
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;
  
  omp_set_nested(1);//���}openmp �_������� �ѦҸ�� https://www.phototalks.idv.tw/academic/?p=1997 
  // ���U Winsock DLL
  WSADATA wsadata;
  if(WSAStartup(0x101,(LPWSADATA)&wsadata) != 0) {
    printf("Winsock Error\n");
    exit(1);                                         
  }

  // ���� server socket
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET(�ϥ�IPv4); SOCK_STREAM; 0(�ϥιw�]�q�T��w�A�YTCP)
  if(server_sockfd == SOCKET_ERROR) {
    printf("Socket Error\n");
    exit(1);
  }
 
  // struct sockaddr_in {
  //     short int               sin_family; /* AF_INT(�ϥ�IPv4) */
  //     unsigned short int sin_port;    /* Port(��) */
  //     struct in_addr       sin_addr;   /* IP��} */
  // };
  // sturct in_addr {
  //     unsigned long int s_addr;
  // };
  server_address.sin_family = AF_INET; // AF_INT(�ϥ�IPv4)
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // �]�wIP��}
  //server_address.sin_addr.s_addr = inet_addr("122.116.195.140"); // �]�wIP��}
  server_address.sin_port = 9734; //�]�w��
  server_len = sizeof(server_address);
  
  if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0) {
    printf("Bind Error\n");
    exit(1);
  }

  if(listen(server_sockfd, 5) < 0) {
    printf("Listen Error\n");
    exit(1);
  }
  printf("�w�إߦ��A�� %s:%d\n","127.0.0.1",server_address.sin_port);
  //printf("�w�إߦ��A�� %s:%d\n","122.116.195.140",server_address.sin_port);
  int timer_client_send_buffer = 0;
  
  bool client_UUID[100]; 		//�]�w�@��0~100 �� ID�ƶq 
  char **message_buffer;		//�]�w100�� �s���ϥΪ̶ǰe��ƪ��ɮ� 
  char *user_leave;
  SOCKET *client_sockfd_UUID;
  while(1){
  	//�}�l�Ыبѵ��C�ӨϥΪ̦s�x�ϥΪ̦�l�� buffer
	do{
  		client_sockfd_UUID = (SOCKET*)malloc(sizeof(SOCKET)*100);
  		if(client_sockfd_UUID == NULL)
  			printf("client_sockfd_UUID �O����Ыإ��� ���b����\n");
  	}while(client_sockfd_UUID == NULL);
  	//�}�l�Ыبѵ��C�ӨϥΪ̶ǵ����A������r���s�x��buffer
  	do{
  		message_buffer = (char**)malloc(sizeof(char*)*100);
  		if(message_buffer == NULL)
  			printf("message_buffer �O����Ыإ��� ���b����\n");
  	}while(message_buffer == NULL);
  	
  	for(int loopnum1 = 0;loopnum1 < 100;loopnum1++){
  		message_buffer[loopnum1] = (char*)malloc(sizeof(char)*1048576);
  		if(message_buffer[loopnum1] == NULL){
  			printf("message_buffer[%d] �O����Ыإ��� ���b����\n",loopnum1);
  			loopnum1--;
  		}
  	}
  	//�}�l�Ыبѵ��C�ӨϥΪ����}���A������r���s�x��buffer
  	do{
  		user_leave = (char*)malloc(sizeof(char)*1048576);
  		if(user_leave == NULL)
  			printf("user_leave �O����Ыإ��� ���b����\n");
  	}while(user_leave == NULL);
  	user_leave[0] = '\0'; 
  	//��l��UUID 
  	for(int loopnum1 = 0;loopnum1 < 100; loopnum1++)
  		client_UUID[loopnum1] = false;
  	#pragma omp parallel
  	{  	
	  	#pragma omp sections
	  	{
	  		#pragma omp section
	  		{
	  			wait_client(server_address,server_sockfd,server_len,client_sockfd_UUID,client_UUID,message_buffer,user_leave);
	  		}
	  		#pragma omp section
	  		{
	  			server_send(client_sockfd_UUID,client_UUID,message_buffer,user_leave);
	  		}	
	  	}
  	}
  	
  	//�^���O����� 
  	free(client_sockfd_UUID);
  	for(int loopnum1 = 0;loopnum1 < 100;loopnum1++){
  		free(message_buffer[loopnum1]);
  	}
  	free(message_buffer);
  	printf("�{������!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  	system("pause");
  } 
}

void wait_client(sockaddr_in server_address,SOCKET server_sockfd,int server_len,SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,char *user_leave){
	int UUID;
	int client_len;
	struct sockaddr_in client_address;
	
	do{
	    //�C 0.5 �� �M��(����)�Ŷ��������Ŷ� 
	    do{
	    	for(UUID = 0;client_UUID[UUID] && UUID<100;UUID++);
	    	Sleep(500);
	    }while(UUID == 100);
	    client_UUID[UUID] = true;			//�N�����Ŷ��m�U�� 
	    Sleep(10);
	    server_time_print(1);
	    printf("���A�����ݨϥΪ̳s�J��(ID:%d)...\n",UUID);
	    client_len = sizeof(client_address);
	
	    client_sockfd_UUID[UUID] = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
	    if(client_sockfd_UUID[UUID] == SOCKET_ERROR){
	    	server_time_print(1);
	    	printf("�s�u���ѥ��b����...\n");
	    	client_UUID[UUID] = false;
	    }
    }while(client_sockfd_UUID[UUID] == SOCKET_ERROR);
	#pragma omp parallel
	{
		#pragma omp sections
	  	{
	  		#pragma omp section
	  		{
	  			wait_client(server_address,server_sockfd,server_len,client_sockfd_UUID,client_UUID,message_buffer,user_leave);
	  		}
	  		#pragma omp section
	  		{
				server_recv(client_sockfd_UUID,client_UUID,message_buffer,UUID,user_leave);
			}
		}
	}
}

int command_client(char *message){
	//printf("�ϥΪ̿�J�F���Omessage = %s\n",message);
	char check_text[4][5] = {"name","abc","����","end;"};
	int check_text_index = -1;
	for(int loopnum1 = 0;check_text_index == -1;loopnum1++){
		char end_text[] = "end;";
		for(int loopnum2 = 0;loopnum2 <= strlen(end_text)+1;loopnum2++){
			if(check_text[loopnum1][loopnum2] == end_text[loopnum2]){
				if(end_text[loopnum2] == '\0')
					check_text_index = loopnum1;
			}else
				break;
		} 
	}
	//printf("check_text_index = %d\n",check_text_index);
	//printf("strlen(check_text[0]) = %d\n",strlen(check_text[0]));
	if(message[0] == '/')
		return -1;
	int mode = -2;
	for(int loopnum1 = 0;loopnum1 < check_text_index && mode == -2;loopnum1++){
		for(int loopnum2 = 0;loopnum2 < strlen(check_text[loopnum1]);loopnum2++){
	    	if(check_text[loopnum1][loopnum2] != message[loopnum2]){
	    		break;
			}else if(loopnum2 == strlen(check_text[loopnum1])-1){
				mode = loopnum1;
				break;
			}
		}
	}
	return mode;
}

void server_recv(SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,int UUID,char *user_leave){
	char *ch;
	char bye_text[] = "bye";
	char buffer;
	char client_name[20];
	sprintf(client_name,"�ϥΪ�[ %d ]",UUID);
	server_time_print(1);
	printf("���b����%s�������s�u\n",client_name,UUID);
	while(client_UUID[UUID] && client_sockfd_UUID[UUID] != SOCKET_ERROR){
		buffer = 0; 
		recv(client_sockfd_UUID[UUID], &buffer, 1, 0);
		ch = (char*)malloc(sizeof(char)*buffer);
		if(buffer <= 0){
			server_time_print(1);
			printf("%s�^�ǭȲ��`,�����_�}�s�u!!\n",client_name);
			client_UUID[UUID] = false;
			break;
		}
		//printf("buffer = %d\n", buffer);
		recv(client_sockfd_UUID[UUID], ch, buffer, 0);
		//���O�T���˴� 
	    for(int i = 0;i<3;i++){
	    	if(bye_text[i] != ch[i]){
	    		break;
			}else if(i == 2 && ch[i+1] == '\0'){
				client_UUID[UUID] = false;
			}
		}
		if(!client_UUID[UUID])
			break;
			
		//message�˴�
		int mode = -3;
		if(ch[0]=='/')
			mode = command_client(ch+1);
		char command_error[100];
		char command_error_len;
		//printf("mode = %d\n",mode);
		switch(mode){
			case	-2:
					sprintf(command_error,"�d�L���O");
					command_error_len = (int)strlen(command_error);
					command_error[command_error_len]='\0';
					send(client_sockfd_UUID[UUID], &command_error_len, 1, 0); 
		    		send(client_sockfd_UUID[UUID], command_error, command_error_len, 0);
				break;
			case	0:
				sprintf(ch,"%s",ch+5);
				if(ch[0] == ' ' && ch[1] != '\0'){
					sprintf(ch,"%s",ch+1);
					while(message_buffer[UUID][0]!='\0');
					sprintf(message_buffer[UUID],"%s:�w�g��W���i%s�j",client_name,ch);
					sprintf(client_name,"%s",ch);
				}else{
					sprintf(command_error,"��J���~�����O �п�Jsend /name <�W�r>");
					command_error_len = (int)strlen(command_error);
					command_error[command_error_len]='\0';
					send(client_sockfd_UUID[UUID], &command_error_len, 1, 0); 
		    		send(client_sockfd_UUID[UUID], command_error, command_error_len, 0);
				}
				break;
			case	1:
				sprintf(ch,"%s",ch+5);
				sprintf(command_error,"abc���O");
				command_error_len = (int)strlen(command_error);
				command_error[command_error_len]='\0';
				send(client_sockfd_UUID[UUID], &command_error_len, 1, 0); 
	    		send(client_sockfd_UUID[UUID], command_error, command_error_len, 0);
	    	case	2:
	    		sprintf(ch,"%s",ch+5);
				sprintf(command_error,"abc������O");
				command_error_len = (int)strlen(command_error);
				command_error[command_error_len]='\0';
				send(client_sockfd_UUID[UUID], &command_error_len, 1, 0); 
	    		send(client_sockfd_UUID[UUID], command_error, command_error_len, 0);
			case	-1:
				sprintf(ch,"%s",ch+1);
			case	-3:
			default:
				if(message_buffer[UUID][0] != '\0')
					sprintf(message_buffer[UUID],"%s\n             %s",message_buffer[UUID],ch);
				else
					sprintf(message_buffer[UUID],"%s:%s",client_name,ch);
				break;
		}
		//printf("ch = %s\n", ch);
		
		//printf("�w����ϥΪ̰T�� �T���� :%s\n",message_buffer[UUID]);
		free(ch);
	}
	sprintf(user_leave,"%s%s:�w���}\n",user_leave,client_name);
	server_time_print(1);
	printf("�w���� %s �������s�u\n",client_name);
	closesocket(client_sockfd_UUID[UUID]);
}

void server_send(SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,char *user_leave){
	int index_UUID = 0;
	char message_len,user_leave_len;
	while(1){
		if(user_leave[0] != '\0'){
			Sleep(20);
			user_leave_len = (int)(strlen(user_leave));
		    user_leave[user_leave_len-1]='\0';
			for(int loopnum2 = 0;loopnum2 < 100;loopnum2++){
		    	if(client_UUID[loopnum2]){
		    		send(client_sockfd_UUID[loopnum2], &user_leave_len, 1, 0); // Linux socket programming �� write
		    		send(client_sockfd_UUID[loopnum2], user_leave, user_leave_len, 0); // Linux socket programming �� write
		    	}
		    }
		    user_leave[0] = '\0';
		}
		
		if(message_buffer[index_UUID][0] != '\0'){
			message_len = (int)(strlen(message_buffer[index_UUID])+1);
		    message_buffer[index_UUID][message_len-1]='\0';
		    for(int loopnum2 = 0;loopnum2 < 100;loopnum2++){
		    	if(client_UUID[loopnum2]){
		    		send(client_sockfd_UUID[loopnum2], &message_len, 1, 0); // Linux socket programming �� write
		    		send(client_sockfd_UUID[loopnum2], message_buffer[index_UUID], message_len, 0); // Linux socket programming �� write
		    	}
		    }
		    Sleep(5);
		    server_time_print(0);
		    printf("%s\n",message_buffer[index_UUID]);
		    message_buffer[index_UUID][0] = '\0';
		}
	    index_UUID++;
	    if(index_UUID >= 100)
	    	index_UUID = 0;
	    Sleep(10);
	}
}

void CTime(char *current_date,char *current_time){ 
	/* time_t��ܪ��ɶ��]���ɶ��^�O�q�@�Ӯɶ��I 
    1970�~1��1��0��0��0��^�즹�ɪ����,�O����ƫ��A */
	time_t  t;
  
	//�ϥ�clock()�禡�ӭp�ɡA�^�Ǫ���ƫ��O��clock_ 
	clock_t  start_clock , end_clock;  
	double spend; 

	//���o�{���q�}�l����즹���,�g�L���w����(ticks) 
	start_clock=clock();
                   
	//�N�ثe������HMM/DD/YY�覡�A�s�J�r��current_date�� 
     _strdate(current_date);

     //�N�ثe���ɶ��HHH�GMM�GSS�覡�A�s�J�r��current_time�� 
     _strtime(current_time); 
}
void SetColor(unsigned short ForeColor,unsigned short BackGroundColor){
	HANDLE hCon=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon,(ForeColor%16)|(BackGroundColor%16*16));
}
void server_time_print(int mode){
	char current_date[9],current_time[9];
	CTime(current_date,current_time);
	switch(mode){
		case	0:
			printf("[%s][",current_time);
			SetColor(2,0);
			printf("��ѰT��");
			SetColor(7,0);
			printf("]:");
			break;
		case	1:
			printf("[%s][",current_time);
			SetColor(6,0);
			printf("�t�ΰT��");
			SetColor(7,0);
			printf("]:");
			break;
		default:
			break;
	}
	
}
