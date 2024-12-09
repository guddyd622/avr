#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#define state_lock 0																	//	mcu �ʱ���� lock
#define state_solve 1																	//	sw1�� �Էµ��� �� ���� solve(��й�ȣ �Է�â)
#define state_ans 2
																						//	��й�ȣ �Է� �� �������
unsigned char LOCK[4]={0x38, 0x3f, 0x39, 0x70};											//	LOCK FND
unsigned char digit[11]={0x40,0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67};		//	FND 0~9,�ʱ���� - ǥ��
unsigned char fnd_sel[4]={0x08,0x04,0x02,0x01};											//	FND select
unsigned char open[4]={0x3f,0x73,0x79,0x54};											//	OPEN FND
unsigned char fail[4]={0x71,0x77,0x06,0x38};											//	FAIL FND
volatile int password=1234;																//	�ʱ� ��й�ȣ
volatile int password_count[4];															//	��й�ȣ �Է¹޴� ��
volatile int state=state_lock;															//	�ʱ� mcu ����
volatile int count=0;																	//	�ʱ� count ����
volatile int selcount=0;																//	�ʱ� selcount(fnd �ڸ����� �Լ�)
volatile int reset=0;																	//	�ʱ�ȭ�鿡�� ��� count�� selcount �ʱ�ȭ
 
 
 
ISR(INT5_vect)																			//	sw2 �Է½� count�� �����ϸ�, 
{																						//	4�ڸ� �迭 password_count�� count���� �Է�
	cli();																				//	count�� 10�̵Ǹ� 0���� �ʱ�ȭ
	count++;																			//	���� reset�� ��Ű�� �ʱ����� reset�� 0���� �ʱ�ȭ
	password_count[selcount]=count;														//	chattering ���� delay
	if(count==10)
	{
	count=0;
	}
 	reset=0;
 	_delay_ms(100);
 	sei();
}
 
 
ISR(INT4_vect)																			//	sw1 �Է� �� MCU�� �ʱ� lock���¿��� solve���·�
{																						//	���� solve���¿��� sw1�� �ԷµǸ�
 	cli();																				//	selcount�� �ö󰡸� FND�� �ڸ��� ����
	if(state==state_lock)																//	sw1�� 5�� �ԷµǸ� MCU�� ans���·� ����
	{																					//	���� sw1�� 6���ԷµǸ� �ʱ���·� �ٽ� ����
	state = state_solve;																// 	chattering ���� delay
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
{															// reset�Լ�
			password_count[3]=0;							// ��� ��й�ȣ�� �ʱ���·� ��ȯ
			password_count[2]=0;
			password_count[1]=0; 
			password_count[0]=0;
			selcount=0;
			count=0;	
}
 
void init(){													// MCU �ʱ� �������� ����
	DDRC=0xff;													// DDRC,DDRG ( FND ) DDRA ( LED )
	DDRG=0x0f;													// DDRE ( SW 1,2 �Է����� ���� ) EIMSK ( interrupt 4,5 Ȱ��ȭ )
	DDRA=0xff;													// EICRB ( ���ͷ�Ʈ low edge �� �߻� )
	DDRE=0xcf;													// SREG ( ���� ���ͷ�Ʈ ��� ���� )
	EIMSK=0x30;													// sei ( ���ͷ�Ʈ ���� )
	EICRB=0x0A;
	SREG=0x08;
	sei();
	}
 
void setup()
{						
	reset=0;															// MCU�� lock ������ �� ���Ǵ� �Լ�
	reset1();													// ��� count�� selcount�� 0���� �ʱ�ȭ
	int i=0;													// LOCK FND �۵� ( FND�� LOCK �� ��� )
	for(i=0;i<4;i++){											// LED0 Ȱ��ȭ ( LOCK ���� �˸� )
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
 
 
 
 
void solve(){												// solve�Լ�
 															// MCU�� solve���·� ���� �� �۵�
	PORTA=0x00;
	int k=0;												// LED�� �������� FND�� �ڸ����� count���� �Է��Ͽ� password_count�� �迭���� ����
	int i=0;
	for(k=0;k<40;k++){
		if(k<20){														// ���� sw1�� ����� �� FND�� �ڸ����� ����Ǹ� password_count �迭�� ��ȯ�Ͽ� ���� ����
			for(i=0;i<4;i++)									// for���� �� �� ����Ǹ� 8ms�� �ð��� ��� �ǰ� reset���� 1�� ����
			{
				if(i==selcount)
				{
				PORTC=0x00;
				_delay_ms(2);		
				}
				else{										// for���� 125�� ����Ǹ� 1000ms �� ����ϸ� reset�� 125�� ��
				PORTC=digit[password_count[i]];				// for���� 625�� ����Ǹ� 5000ms �� ����Ͽ� MCU�� lock���·� ���ԵǸ� ��簪 reset
				PORTG=fnd_sel[i];
				_delay_ms(2);
					}
			}
				}
	else{
		for(i=0;i<4;i++){
				PORTC=digit[password_count[i]];				// for���� 625�� ����Ǹ� 5000ms �� ����Ͽ� MCU�� lock���·� ���ԵǸ� ��簪 reset
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
 
void ans()																				// SW1 5�� �Էµ� �� �Է¹��� ��й�ȣ��
{																						// ������ ��й�ȣ�� ��ġ���� �ƴ��� �Ǵ� ��
	int passwordinput=0;																// ��ġ �� ��� OPEN�� ���� LED2~LED4���� 1�ʴ����� �����ϸ� OPEN���� �˸�)
 																						// ��ġ���� ���� ��� FAIL�� ���� LED7�� ����
	passwordinput=(((password_count[0]-1)*1000)+((password_count[1]-1)*100)				// ���� selcount�� 5�� �� ��� ��, SW1�� 6�� ���� ��� MCU�� �ʱ���·� ����
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
