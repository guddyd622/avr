#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

#define VENT_level 750
#define LED_level 450

static int putchar0(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(putchar0, NULL, _FDEV_SETUP_WRITE);

unsigned char num[]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 
  0x6D, 0x7D, 0x27, 0x7F, 0x67};

unsigned char sel[]={0x01, 0x02, 0x04, 0x08};

volatile unsigned int ctrl_state = 0;//		Idle / auto / manual
volatile unsigned int MANUAL_ctrl = 0;//	off / F0 / 0F / VENT
volatile unsigned int CDS = 0;

ISR(INT4_vect)// SW1 ���۸�� ���� ����ġ 1: ù ����, �ڵ� �� ���� ��� ��ȯ.
{ 
	cli();
	if(ctrl_state == 0||ctrl_state == 2){
		ctrl_state = 1; 
		printf("AUTO MODE\n"); //"MANUAL MODE" �Ǵ� "AUTO MODE" �޽��� ����.
		EIMSK=0x10;
		}
	else if(ctrl_state == 1){
		ctrl_state = 2;
		printf("MANUAL MODE\n");
		EIMSK=0x30;
		}
	_delay_ms(100);
	sei();
}

ISR(INT5_vect) //���� ��忡�� LED(0~3/4~7) �� DC���͸� ���� �Ѱų� ��.
{
	cli();
	if(MANUAL_ctrl == 0){
		MANUAL_ctrl = 1;
		printf("MANUAL LED F0\n");//LED ���� ���� ��, ���� ���¸� �ø��� ������� ����
		}
	else if(MANUAL_ctrl == 1){
		MANUAL_ctrl = 2;
		printf("MANUAL LED 0F\n");
		}
	else if(MANUAL_ctrl == 2){
		MANUAL_ctrl = 3;
		printf("MANUAL VENT\n");
		}
	else if(MANUAL_ctrl == 3){
		MANUAL_ctrl = 0;
		printf("MANUAL OFF\n");
		}
	_delay_ms(100);
	sei();
}

int putchar0(char c, FILE *stream)
{
	if(c == '\n')
	putchar0('\r',stream);

	while(!(UCSR0A & (1<<UDRE0)));

	UDR0 = c;
	return 0;
}



unsigned short read_ADC()//ADC read
{
	unsigned char adc_low, adc_high;
	unsigned short value;
	ADCSRA |= (1<<ADSC);

	while((ADCSRA & (1<<ADIF)) == 0);

	adc_low = ADCL;
	adc_high = ADCH;
	value = (adc_high << 8) | adc_low;
	return value;
}

void display(int cnt) //FND ���÷��� �Լ�
{
int fnd[4];
	 fnd[0]=(cnt/1)%10;
	 fnd[1]=(cnt/10)%10;
	 fnd[2]=(cnt/100)%10;
	 fnd[3]=(cnt/1000)%10;

	 for(int i=0; i<4; i++)
	 {
	 PORTC=num[fnd[i]];
	 PORTG=sel[i];

	 if(i%2)
	 	_delay_ms(2);
	 else
	 	_delay_ms(3);
	 }
}

void LED_ctrl(int n)
{
	if(n<LED_level){
		if(PORTA == 0x00)
		printf("LIGHT ON\n");	//"LIGHT ON" �޽��� ����
		PORTA=0x0F;		//�̸��̸� : LED0~LED3 ����.
	}
	else if(n<VENT_level){
		if(PORTA == 0x0F)
		printf("LIGHT OFF\n");	//"LIGHT OFF" �޽��� ����.
		PORTA=0x00;		//�̻��̸�:LED0~LED3 ����.
	}
}

void VENT_ctrl(int n)
{
	if(n>VENT_level){
	PORTA=0xF0;		//�̻��̸�:LED4~LED7 ����.
	if(PORTB==0x00)
	printf("VENT ON\n");	//"VENT ON" �޽��� ����.
	PORTB=0x70;		//����: ��ǳ�� ON
	}
	else if(n>LED_level){
	PORTA=0x00;
	if(PORTB==0x70)		//����: ��ǳ�� ON
	printf("VENT OFF\n");	//"VENT OFF" �޽��� ����.
	PORTB=0x00;		//����: ��ǳ�� OFF
	}
}

void VENT_manual(int n)//���� �����Լ�
{
	if(n==0)
		PORTB=0x00;
	else if(n==1)
		PORTB=0x70;
}


void init()//����
{
	DDRA = 0xFF;
	DDRB = 0x70; // MOTOR_IN_1
	DDRC = 0xFF; // fnd
	DDRG = 0x0F; // fndSEL
	DDRE = 0xCF; // SW4, 5 input

	PORTC=num[0];//ini
	PORTG=sel[0];//ini

	EICRB=0x0A;//trig setting
	EIMSK=0x30;//int MSK 4, 5 enable
	SREG=0x80;//global int enable
	sei();

	UBRR0H = 0;
	UBRR0L = 8;//115200
	UCSR0B = 0x18;
	UCSR0C = 0x06;

	ADMUX = 0x00;//MUX-ADC0(CDS)
	ADCSRA = 0x87;//ADC ENA  Prescaler 128
}

int main()
{
	init();
	stdout = &mystdout;
	PORTG=0x0F;
	
	printf("IDLE\n");	//"IDLE" ���� �޽��� ����
	
		while(1)
		{
			CDS=read_ADC();	
 
			if(ctrl_state == 1){
				LED_ctrl(CDS);
				VENT_ctrl(CDS);
				display(CDS);
			}
			else if(ctrl_state == 2){
				display(CDS);
 
				switch(MANUAL_ctrl){
 
				case 0:
					PORTA=0x00;
					VENT_manual(0);
					break;
				case 1:
					PORTA=0xF0;
					VENT_manual(0);
					break;
				case 2:
					PORTA=0x0F;
					VENT_manual(0);
					break;
				case 3:
					PORTA=0x00;
					VENT_manual(1);
					break;
				}
			}
		}
}