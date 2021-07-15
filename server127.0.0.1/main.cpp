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
  
  omp_set_nested(1);//打開openmp 巢狀執行緒 參考資料 https://www.phototalks.idv.tw/academic/?p=1997 
  // 註冊 Winsock DLL
  WSADATA wsadata;
  if(WSAStartup(0x101,(LPWSADATA)&wsadata) != 0) {
    printf("Winsock Error\n");
    exit(1);                                         
  }

  // 產生 server socket
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET(使用IPv4); SOCK_STREAM; 0(使用預設通訊協定，即TCP)
  if(server_sockfd == SOCKET_ERROR) {
    printf("Socket Error\n");
    exit(1);
  }
 
  // struct sockaddr_in {
  //     short int               sin_family; /* AF_INT(使用IPv4) */
  //     unsigned short int sin_port;    /* Port(埠號) */
  //     struct in_addr       sin_addr;   /* IP位址 */
  // };
  // sturct in_addr {
  //     unsigned long int s_addr;
  // };
  server_address.sin_family = AF_INET; // AF_INT(使用IPv4)
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 設定IP位址
  //server_address.sin_addr.s_addr = inet_addr("122.116.195.140"); // 設定IP位址
  server_address.sin_port = 9734; //設定埠號
  server_len = sizeof(server_address);
  
  if(bind(server_sockfd, (struct sockaddr *)&server_address, server_len) < 0) {
    printf("Bind Error\n");
    exit(1);
  }

  if(listen(server_sockfd, 5) < 0) {
    printf("Listen Error\n");
    exit(1);
  }
  printf("已建立伺服器 %s:%d\n","127.0.0.1",server_address.sin_port);
  //printf("已建立伺服器 %s:%d\n","122.116.195.140",server_address.sin_port);
  int timer_client_send_buffer = 0;
  
  bool client_UUID[100]; 		//設定一個0~100 的 ID數量 
  char **message_buffer;		//設定100組 存取使用者傳送資料的檔案 
  char *user_leave;
  SOCKET *client_sockfd_UUID;
  while(1){
  	//開始創建供給每個使用者存儲使用者位子的 buffer
	do{
  		client_sockfd_UUID = (SOCKET*)malloc(sizeof(SOCKET)*100);
  		if(client_sockfd_UUID == NULL)
  			printf("client_sockfd_UUID 記憶體創建失敗 正在重創\n");
  	}while(client_sockfd_UUID == NULL);
  	//開始創建供給每個使用者傳給伺服器的文字做存儲的buffer
  	do{
  		message_buffer = (char**)malloc(sizeof(char*)*100);
  		if(message_buffer == NULL)
  			printf("message_buffer 記憶體創建失敗 正在重創\n");
  	}while(message_buffer == NULL);
  	
  	for(int loopnum1 = 0;loopnum1 < 100;loopnum1++){
  		message_buffer[loopnum1] = (char*)malloc(sizeof(char)*1048576);
  		if(message_buffer[loopnum1] == NULL){
  			printf("message_buffer[%d] 記憶體創建失敗 正在重創\n",loopnum1);
  			loopnum1--;
  		}
  	}
  	//開始創建供給每個使用者離開伺服器的文字做存儲的buffer
  	do{
  		user_leave = (char*)malloc(sizeof(char)*1048576);
  		if(user_leave == NULL)
  			printf("user_leave 記憶體創建失敗 正在重創\n");
  	}while(user_leave == NULL);
  	user_leave[0] = '\0'; 
  	//初始化UUID 
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
  	
  	//回收記憶體區 
  	free(client_sockfd_UUID);
  	for(int loopnum1 = 0;loopnum1 < 100;loopnum1++){
  		free(message_buffer[loopnum1]);
  	}
  	free(message_buffer);
  	printf("程式結尾!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  	system("pause");
  } 
}

void wait_client(sockaddr_in server_address,SOCKET server_sockfd,int server_len,SOCKET *client_sockfd_UUID,bool *client_UUID,char **message_buffer,char *user_leave){
	int UUID;
	int client_len;
	struct sockaddr_in client_address;
	
	do{
	    //每 0.5 秒 尋找(等待)空閒的網路空間 
	    do{
	    	for(UUID = 0;client_UUID[UUID] && UUID<100;UUID++);
	    	Sleep(500);
	    }while(UUID == 100);
	    client_UUID[UUID] = true;			//將網路空間搶下來 
	    Sleep(10);
	    server_time_print(1);
	    printf("伺服器等待使用者連入中(ID:%d)...\n",UUID);
	    client_len = sizeof(client_address);
	
	    client_sockfd_UUID[UUID] = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
	    if(client_sockfd_UUID[UUID] == SOCKET_ERROR){
	    	server_time_print(1);
	    	printf("連線失敗正在重試...\n");
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
	//printf("使用者輸入了指令message = %s\n",message);
	char check_text[4][5] = {"name","abc","中文","end;"};
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
	sprintf(client_name,"使用者[ %d ]",UUID);
	server_time_print(1);
	printf("正在接收%s的網路連線\n",client_name,UUID);
	while(client_UUID[UUID] && client_sockfd_UUID[UUID] != SOCKET_ERROR){
		buffer = 0; 
		recv(client_sockfd_UUID[UUID], &buffer, 1, 0);
		ch = (char*)malloc(sizeof(char)*buffer);
		if(buffer <= 0){
			server_time_print(1);
			printf("%s回傳值異常,直接斷開連線!!\n",client_name);
			client_UUID[UUID] = false;
			break;
		}
		//printf("buffer = %d\n", buffer);
		recv(client_sockfd_UUID[UUID], ch, buffer, 0);
		//指令掰的檢測 
	    for(int i = 0;i<3;i++){
	    	if(bye_text[i] != ch[i]){
	    		break;
			}else if(i == 2 && ch[i+1] == '\0'){
				client_UUID[UUID] = false;
			}
		}
		if(!client_UUID[UUID])
			break;
			
		//message檢測
		int mode = -3;
		if(ch[0]=='/')
			mode = command_client(ch+1);
		char command_error[100];
		char command_error_len;
		//printf("mode = %d\n",mode);
		switch(mode){
			case	-2:
					sprintf(command_error,"查無指令");
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
					sprintf(message_buffer[UUID],"%s:已經更名為【%s】",client_name,ch);
					sprintf(client_name,"%s",ch);
				}else{
					sprintf(command_error,"輸入錯誤的指令 請輸入send /name <名字>");
					command_error_len = (int)strlen(command_error);
					command_error[command_error_len]='\0';
					send(client_sockfd_UUID[UUID], &command_error_len, 1, 0); 
		    		send(client_sockfd_UUID[UUID], command_error, command_error_len, 0);
				}
				break;
			case	1:
				sprintf(ch,"%s",ch+5);
				sprintf(command_error,"abc指令");
				command_error_len = (int)strlen(command_error);
				command_error[command_error_len]='\0';
				send(client_sockfd_UUID[UUID], &command_error_len, 1, 0); 
	    		send(client_sockfd_UUID[UUID], command_error, command_error_len, 0);
	    	case	2:
	    		sprintf(ch,"%s",ch+5);
				sprintf(command_error,"abc中文指令");
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
		
		//printf("已收到使用者訊息 訊息為 :%s\n",message_buffer[UUID]);
		free(ch);
	}
	sprintf(user_leave,"%s%s:已離開\n",user_leave,client_name);
	server_time_print(1);
	printf("已關閉 %s 的網路連線\n",client_name);
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
		    		send(client_sockfd_UUID[loopnum2], &user_leave_len, 1, 0); // Linux socket programming 為 write
		    		send(client_sockfd_UUID[loopnum2], user_leave, user_leave_len, 0); // Linux socket programming 為 write
		    	}
		    }
		    user_leave[0] = '\0';
		}
		
		if(message_buffer[index_UUID][0] != '\0'){
			message_len = (int)(strlen(message_buffer[index_UUID])+1);
		    message_buffer[index_UUID][message_len-1]='\0';
		    for(int loopnum2 = 0;loopnum2 < 100;loopnum2++){
		    	if(client_UUID[loopnum2]){
		    		send(client_sockfd_UUID[loopnum2], &message_len, 1, 0); // Linux socket programming 為 write
		    		send(client_sockfd_UUID[loopnum2], message_buffer[index_UUID], message_len, 0); // Linux socket programming 為 write
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
	/* time_t表示的時間（日曆時間）是從一個時間點 
    1970年1月1日0時0分0秒）到此時的秒數,是長整數型態 */
	time_t  t;
  
	//使用clock()函式來計時，回傳的資料型別為clock_ 
	clock_t  start_clock , end_clock;  
	double spend; 

	//取得程式從開始執行到此函數,經過的滴答數(ticks) 
	start_clock=clock();
                   
	//將目前的日期以MM/DD/YY方式，存入字串current_date內 
     _strdate(current_date);

     //將目前的時間以HH：MM：SS方式，存入字串current_time內 
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
			printf("聊天訊息");
			SetColor(7,0);
			printf("]:");
			break;
		case	1:
			printf("[%s][",current_time);
			SetColor(6,0);
			printf("系統訊息");
			SetColor(7,0);
			printf("]:");
			break;
		default:
			break;
	}
	
}
