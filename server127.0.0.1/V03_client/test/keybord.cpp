#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
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
int main(){
	int x,y,index = 0;
	bool num1 = false,num2 = true;
	#pragma omp parallel sections 	
	{
		#pragma omp section
		{
			while(1){
				if(num1){
					num2 = false;
				}
				if(!num1 && num2){
					getxy(&x,&y);
					gotoxy(0,0);
					printf("x = %d, y = %d                                                       ",x,y);
					gotoxy(x,y);
				}
				Sleep(10);
			}
		}
		#pragma omp section
		{
			char c; 
			while ((c=getchar())!='/n'){   /*每?getchar()依次?入一?字符*/
				if(c != '\n'){
					bool first = false;
					if(num2){
						first = true;	
					}
					num1 = true;
					while(num2);
					if(first)
						gotoxy(0,2);	
					printf("%c",c);    /*按照原??出*/
				}else{
					gotoxy(0,3);
					printf("輸出enter了index = %d",index);
					gotoxy(0,1);
					for(int loopnum1 = 0;loopnum1<100;loopnum1++){
						printf(" ");
					}
					num1 = false;
					num2 = true;
					index++;
					gotoxy(0,1);
				} 
			}
			getch();       /*等待按任一健*/ 
		}
	}	
	
}
