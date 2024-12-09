#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
 
unsigned char num[]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 
  0x6D, 0x7D, 0x27, 0x7F, 0x67};
 
unsigned char sel[]={0x01, 0x02, 0x04, 0x08};
 
unsigned char rdy[]={0x6E, 0x5E, 0x50};
 
unsigned char one_LED[]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
 
volatile unsigned int game_MODE = 0;
volatile unsigned int game_TIME = 1000;//원래 6000
volatile unsigned int score=0;
volatile unsigned int cnt1=0;
volatile unsigned int cnt2=0;
volatile unsigned int click=0;
 
 
 
ISR(INT4_vect)// SW1 동작모드 설정
{ 
	cli();
	game_MODE=1;//게임스타트
	_delay_ms(100);
	sei();
}
 
ISR(INT5_vect)
{
	cli();
	if(click == 0){
	click=1;	// 중복클릭 카운트
	score += 10;
	}
	_delay_ms(30);
	sei();
}
 
void RDY() // FND ready
{
		for(int i=0; i<3;i++)
		{
			PORTC=rdy[i];
			PORTG=sel[i];
			_delay_ms(2);
		}
}
 
void display(int n) //FND 디스플레이 함수
{
int fnd[4];
	 fnd[0]=(n/1)%10;
	 fnd[1]=(n/10)%10;
	 fnd[2]=(n/100)%10;
	 fnd[3]=(n/1000)%10;
 
	 for(int i=0; i<4; i++)
	 {
	 PORTG=sel[i];
 
	 if((fnd[1]<5) && i == 2)
	 PORTC=num[fnd[i]] | 0b10000000;
	 else
	 PORTC=num[fnd[i]];
 
	 if(i%2)
	 	_delay_ms(2);
	 else
	 	_delay_ms(3);
	 }
}
 
void game_TIME_reduce() //시간감소함수
{
	if(++cnt1 == 10)
	{
		cnt1=0;
		if(game_TIME!=0 && game_MODE == 1)
		game_TIME--;
	}
}
 
void rand_LED()	//랜덤LED함수
{
		if(++cnt2 == 1000)//OVF 1000번당 한번 작동 = 1초
		{
			cnt2=0;
 
			int i = rand()%8; //i set rand(0~7)
			PORTA=one_LED[i];
 
			if(click == 0 && score != 0 )
			score -= 10; // 점수 감소
 
			if(click == 0)
			PORTA=0xFF; //반응실패시 LED점등
 
			click = 0;
		}
 
}
 
 
ISR(TIMER0_OVF_vect) // OVERFLOW/////////
{
	TCNT0 = 0x06;
	game_TIME_reduce();
 
	if(game_MODE == 1)
	rand_LED();
 
	if(game_TIME == 0)//시간 0초시 게임 끝
	game_MODE = 2;
 
	if(click == 1)	//반응 성공시 LED 소등
	PORTA=0x00;
 
}
 
void init()
{
	DDRA=0xFF;
	DDRC = 0xFF; // fnd
	DDRG = 0x0F; // fndSEL
	DDRE = 0xCF; // SW4, 5 input
 
	PORTC=num[0];//ini
	PORTG=sel[0];//ini
 
	TCNT0=0x06;
	TCCR0=4;
	TIMSK=1;
 
	EICRB=0x0A;//trig setting
	EIMSK=0x30;//int MSK 4, 5 enable
	SREG=0x80;//global int enable
	sei();
 
	game_TIME++;
}
 
int main()
{
	init();
	while(1)
	{
		if(game_MODE==0)
		RDY();
 
		else if(game_MODE==1){	
			display(game_TIME);
		}
		else if(game_MODE==2){
			display(score);
		}
	}
}
