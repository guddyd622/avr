#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#define state_lock 0																	//	mcu 초기상태 lock
#define state_solve 1																	//	sw1이 입력됐을 시 상태 solve(비밀번호 입력창)
#define state_ans 2
																						//	비밀번호 입력 후 결과상태
unsigned char LOCK[4]={0x38, 0x3f, 0x39, 0x70};											//	LOCK FND
unsigned char digit[11]={0x40,0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67};		//	FND 0~9,초기상태 - 표시
unsigned char fnd_sel[4]={0x08,0x04,0x02,0x01};											//	FND select
unsigned char open[4]={0x3f,0x73,0x79,0x54};											//	OPEN FND
unsigned char fail[4]={0x71,0x77,0x06,0x38};											//	FAIL FND
volatile int password=1234;																//	초기 비밀번호
volatile int password_count[4];															//	비밀번호 입력받는 값
volatile int state=state_lock;															//	초기 mcu 상태
volatile int count=0;																	//	초기 count 상태
volatile int selcount=0;																//	초기 selcount(fnd 자리선택 함수)
volatile int reset=0;																	//	초기화면에서 모든 count와 selcount 초기화
 
 
 
ISR(INT5_vect)																			//	sw2 입력시 count가 증가하며, 
{																						//	4자리 배열 password_count에 count값을 입력
	cli();																				//	count가 10이되면 0으로 초기화
	count++;																			//	또한 reset을 시키지 않기위해 reset값 0으로 초기화
	password_count[selcount]=count;														//	chattering 위한 delay
	if(count==10)
	{
	count=0;
	}
 	reset=0;
 	_delay_ms(100);
 	sei();
}
 
 
ISR(INT4_vect)																			//	sw1 입력 시 MCU의 초기 lock상태에서 solve상태로
{																						//	만약 solve상태에서 sw1이 입력되면
 	cli();																				//	selcount가 올라가며 FND의 자릿수 변경
	if(state==state_lock)																//	sw1이 5번 입력되면 MCU는 ans상태로 변경
	{																					//	만약 sw1이 6번입력되면 초기상태로 다시 돌입
	state = state_solve;																// 	chattering 위한 delay
	}
	else
	{
	selcount++;
	reset=0;
	count=0;
	}
	if(selcount==4)
			{
		state=state_ans;
			}
	if(selcount==5)
			{
		state=state_lock;
			}
	_delay_ms(100);
	sei();
 
}
 
void reset1()
{															// reset함수
			password_count[3]=0;							// 모든 비밀번호를 초기상태로 변환
			password_count[2]=0;
			password_count[1]=0; 
			password_count[0]=0;
			selcount=0;
			count=0;	
}
 
void init(){													// MCU 초기 레지스터 설정
	DDRC=0xff;													// DDRC,DDRG ( FND ) DDRA ( LED )
	DDRG=0x0f;													// DDRE ( SW 1,2 입력으로 설정 ) EIMSK ( interrupt 4,5 활성화 )
	DDRA=0xff;													// EICRB ( 인터럽트 low edge 시 발생 )
	DDRE=0xcf;													// SREG ( 전역 인터럽트 사용 선언 )
	EIMSK=0x30;													// sei ( 인터럽트 시작 )
	EICRB=0x0A;
	SREG=0x08;
	sei();
	}
 
void setup()
{						
	reset=0;															// MCU가 lock 상태일 때 사용되는 함수
	reset1();													// 모든 count와 selcount를 0으로 초기화
	int i=0;													// LOCK FND 작동 ( FND에 LOCK 를 띄움 )
	for(i=0;i<4;i++){											// LED0 활성화 ( LOCK 됨을 알림 )
		PORTC=LOCK[i];
		PORTG=fnd_sel[i];
		PORTA=0x01;
		_delay_ms(2);
					}
}
 
 
void ansLED()													// 
{																// 
		int i=0;												// 
		int k=0;												// 
		for(k=0;k<250;k++)										// 
		{
			if(k<125){
			for(i=0;i<4;i++){
			PORTC=open[i];
			PORTG=fnd_sel[i];
			_delay_ms(2);
			PORTA=0x02;	
							}
							}
			else if(k>125){
			for(i=0;i<4;i++){
			PORTC=open[i];
			PORTG=fnd_sel[i];
			_delay_ms(2);
			PORTA=0x06;		
							}
							}
 
		}
		while(selcount!=5)
		{
		for(i=0;i<4;i++){
			PORTC=open[i];
			PORTG=fnd_sel[i];
			_delay_ms(2);
			PORTA=0x0e;	}
		}
 
}
 
 
 
 
void solve(){												// solve함수
 															// MCU가 solve상태로 돌입 시 작동
	PORTA=0x00;
	int k=0;												// LED는 꺼버리고 FND의 자릿수에 count값을 입력하여 password_count의 배열값을 지정
	int i=0;
	for(k=0;k<40;k++){
		if(k<20){														// 만약 sw1이 실행될 시 FND의 자릿수가 변경되며 password_count 배열을 변환하여 값을 지정
			for(i=0;i<4;i++)									// for문이 한 번 실행되면 8ms의 시간이 경과 되고 reset값을 1씩 증가
			{
				if(i==selcount)
				{
				PORTC=0x00;
				_delay_ms(2);		
				}
				else{										// for문이 125번 실행되면 1000ms 가 경과하며 reset이 125가 됨
				PORTC=digit[password_count[i]];				// for문이 625번 실행되면 5000ms 가 경과하여 MCU를 lock상태로 돌입되며 모든값 reset
				PORTG=fnd_sel[i];
				_delay_ms(2);
					}
			}
				}
	else{
		for(i=0;i<4;i++){
				PORTC=digit[password_count[i]];				// for문이 625번 실행되면 5000ms 가 경과하여 MCU를 lock상태로 돌입되며 모든값 reset
				PORTG=fnd_sel[i];
				_delay_ms(2);
						}
		}
		reset++;
		if(reset==625)
		{
			state=state_lock;
			reset1();
		}
 
 	}
 
 
 
 
 
}
 
void ans()																				// SW1 5번 입력될 시 입력받은 비밀번호를
{																						// 설정한 비밀번호와 일치한지 아닌지 판단 후
	int passwordinput=0;																// 일치 할 경우 OPEN을 띄우고 LED2~LED4까지 1초단위로 증가하며 OPEN됨을 알림)
 																						// 일치하지 않을 경우 FAIL을 띄우고 LED7이 점멸
	passwordinput=(((password_count[0]-1)*1000)+((password_count[1]-1)*100)				// 만약 selcount가 5가 될 경우 즉, SW1이 6번 눌릴 경우 MCU를 초기상태로 돌입
	+((password_count[2]-1)*10)+password_count[3]-1);
							
	if(passwordinput==password)																														
	{
 			ansLED();
 
	}
	else
	{
 
	int i=0;
	int k=0;
	for(k=0;k<100;k++)
		if(k<50){
		for(i=0;i<4;i++)
			{
			PORTC=fail[i];
			PORTG=fnd_sel[i];
			_delay_ms(2);
			PORTA=0x00;
			}
			}
		else{
		for(i=0;i<4;i++)
		{
			PORTC=fail[i];
			PORTG=fnd_sel[i];
			_delay_ms(2);
			PORTA=0x80;
		}
 
			}
	}
 
 
} 
int main()
{
	init();
	setup();
 
	while(1)
	{
		if(state==state_lock)
		setup();
		else if(state==state_solve)
		solve();
		else if(state==state_ans)
		ans();
 
	}
 
 
}
