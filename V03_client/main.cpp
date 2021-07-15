#include <omp.h>
#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <time.h>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
char buffer;
void getchar_message(bool *waiting_check1,char *write_message_buffer,bool *recv_write,bool *sendding_write);
void send_cmd(SOCKET sockfd,char *error_char,bool *recv_write,bool *sendding_write);
void recv_cmd(SOCKET sockfd,bool *waiting_recv,char *message);
void printf_out_recv(char *message, char **message_buffer,bool *recv_write,bool *sendding_write);
void printf_out_main(char *message, char **message_buffer, bool *printf_recv,bool *recv_write,bool *sendding_write);
void gotoxy(int x, int  y);
void getxy(int *x, int *y);
void CTime(char *current_date,char *current_time);

int main(int argc, char** argv) {
  	SOCKET sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	
	omp_set_nested(1);//打開openmp 巢狀執行緒 參考資料 https://www.phototalks.idv.tw/academic/?p=1997
	 
	WSADATA wsadata;
	if(WSAStartup(0x101,(LPWSADATA)&wsadata) != 0) 
	{
	  	printf("Winsock Error\n"); 
	  	exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = 9734;
	len = sizeof(address);
	int wait_num = 0;
	do{
		result = connect(sockfd, (struct sockaddr *)&address, len);
		printf("\r                                     \r");
		printf("127.0.0.1 連線失敗，嘗試重新連線中");
		for(int loopnum1 = 0;loopnum1 <= wait_num;loopnum1++)
			printf(".");
		wait_num++;
		if(wait_num == 3)
			wait_num = 0;
		Sleep(150);
	}while(result == -1);
	
	bool printf_recv = false;
	char **message_buffer;		//設定100組 存取使用者傳送資料的檔案 
	
	do{
  		message_buffer = (char**)malloc(sizeof(char*)*20);
  		if(message_buffer == NULL)
  			printf("message_buffer 記憶體創建失敗 正在重創\n");
  	}while(message_buffer == NULL);
  	
  	for(int loopnum1 = 0;loopnum1 < 20;loopnum1++){
  		message_buffer[loopnum1] = (char*)malloc(sizeof(char)*1024);
  		if(message_buffer[loopnum1] == NULL){
  			printf("message_buffer[%d] 記憶體創建失敗 正在重創\n",loopnum1);
  			loopnum1--;
  		}else
  			sprintf(message_buffer[loopnum1],"");
  	}
  	char *error_char;
  	
  	do{
  		error_char = (char*)malloc(sizeof(char)*1024);
  		if(error_char == NULL)
  			printf("error_char 記憶體創建失敗 正在重創\n");
  	}while(error_char == NULL);
  	
  	char *message;
  	
  	do{
  		message = (char*)malloc(sizeof(char)*1024);
  		if(message == NULL)
  			printf("error_char 記憶體創建失敗 正在重創\n");
  	}while(message == NULL);
  	bool recv_write = false,sendding_write = false;
	Sleep(500);
	printf("伺服器已連接~~\n");
	char ch[100];
	sprintf(ch,"加入聊天室");
	buffer = (char)(strlen(ch)+1);
	ch[buffer-1] = '\0';
	send(sockfd, &buffer, 1, 0);
	send(sockfd, ch, buffer, 0);
	gotoxy(0,20);
	printf("--------------------------------------------------------------");
	#pragma omp parallel sections 
	{
		#pragma omp section
		{
			send_cmd(sockfd,error_char,&recv_write,&sendding_write);
		}
		#pragma omp section
		{
			recv_cmd(sockfd,&printf_recv,message);	
		}
		#pragma omp section
		{
			printf_out_main(message,message_buffer,&printf_recv,&recv_write,&sendding_write);
		}
	}		
	closesocket(sockfd);
	
	for(int loopnum1 = 0;loopnum1 < 20;loopnum1++){
  		free(message_buffer[loopnum1]);
  	}
  	free(message_buffer);
  	
	exit(0);
}

void getchar_message(bool *waiting_check1,char *write_message_buffer,bool *recv_write,bool *sendding_write){
	char c; 
	bool waiting_check2 = true;
	gotoxy(0,21);
	while ((c=getchar())!='/n'){   /*每?getchar()依次?入一?字符*/
		if(c != '\n'){
			bool first = false;
			while(!*waiting_check1);
			while(*recv_write);
			*sendding_write = true;
			if(waiting_check2){
				first = true;
				waiting_check2 = false;
			}
			if(first)
				sprintf(write_message_buffer,"%c",c);
			else
				sprintf(write_message_buffer,"%s%c",write_message_buffer,c);
		}else{
			gotoxy(0,21);
			for(int loopnum1 = 0;loopnum1<100;loopnum1++)
				printf(" ");
			gotoxy(0,21);
			*waiting_check1 = false;
			waiting_check2 = true;
			*sendding_write = false;
		} 
	}
}

void send_cmd(SOCKET sockfd,char *error_char,bool *recv_write,bool *sendding_write){
	char char_message[100],*ch;
	char command_send[5]="send ";
	char command_bye[3]="bye";
	bool waiting_check1 = false;
	#pragma omp parallel sections 
	{
		#pragma omp section
		{
			getchar_message(&waiting_check1,char_message,recv_write,sendding_write);
		}
		#pragma omp section
		{
			while(1){	
				//fflush(stdin);
			 	//scanf("%[^\n]",char_message);
			 	while(waiting_check1)
			 		Sleep(5);
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
			 			Sleep(1000);
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
						sprintf(error_char,"輸入格式錯誤!請重新輸入\n");
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
			 	waiting_check1 = true;
		 	}
		}
	}	
}

void recv_cmd(SOCKET sockfd,bool *waiting_recv,char *message){
	while(1){
		char *ch;
		buffer = 0; 
		recv(sockfd, &buffer, 1, 0);
		if(buffer <= 0)
			exit(0);
		ch = (char*)malloc(sizeof(char)*buffer);
		recv(sockfd, ch, buffer, 0);
		sprintf(message,"%s\n", ch);
		free(ch);
		*waiting_recv = true;
		while(!*waiting_recv);
	}
}

void printf_out_recv(char *message, char **message_buffer,bool *recv_write,bool *sendding_write){
	for(int loopnum1 = 0;loopnum1 < 19;loopnum1++)
		sprintf(message_buffer[loopnum1],"%s",message_buffer[loopnum1+1]);
	char current_date[9],current_time[9];
	CTime(current_date,current_time);
	sprintf(message_buffer[19],"[%s] %s",current_time,message);
	while(*sendding_write)
		Sleep(1);
	*recv_write = true;
	int x,y;
	getxy(&x,&y);
	
	for(int loopnum1 = 0;loopnum1 < 20;loopnum1++){
		gotoxy(0,loopnum1);
		printf("%s",message_buffer[loopnum1]);
		printf("                                                  ");
		printf("\n");
	}
	gotoxy(0,20);
	printf("--------------------------------------------------------------");
	gotoxy(x,y);
	*recv_write = false;
}

void printf_out_main(char *message, char **message_buffer, bool *printf_recv,bool *recv_write,bool *sendding_write){
	while(1){
		if(*printf_recv){
			printf_out_recv(message,message_buffer,recv_write,sendding_write);
			*printf_recv = false;
		}
		if(!*printf_recv)
			Sleep(10);
	}
}
 
// =============================================
// 函式： gotoxy
// 說明： 移動游標至指定位置
// ============================================
void gotoxy(int x, int  y){
        COORD point;
        point.X = x, point.Y = y;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), point);
}

// =============================================
// 函式： getxy
// 說明： 取得目前游標位置
// ============================================
void getxy(int *x, int *y){
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        *x = csbi.dwCursorPosition.X;
        *y = csbi.dwCursorPosition.Y;
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
