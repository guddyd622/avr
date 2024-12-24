#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte degree[] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte PWMicon[] = {
  B00000,
  B01110,
  B01010,
  B01010,
  B01010,
  B11011,
  B00000,
  B00000
};

const int tmpPin1 = A0;  // NTC HOT
const int tmpPin2 = A1;  // NTC COOL
const int pwmRead = A2;  // PWM parameter VR
const int pwmPin1 = 5;   // FAN PWM Pin
const int pwmPin2 = 6;   // FET PWM Pin
const int seriesResistor = 10000;  // 10kohm

unsigned long pre_TMP_UART = 0;
unsigned long pre_DISPLAY = 0;
const long period_TMP_UART = 1000;  // UART period
const long period_DISPLAY = 100;   // display period

char buf_duty[20];  //


float sensingTMP(int sen)  // NTC analog -> °C  processing
{
  float voltage = sen * (5.0 / 1023.0);
  float NTC_res = (5.0 - voltage) * seriesResistor / voltage;
  float steinhart; // 서미스터 저항 값을 온도로 변환 (Steinhart-Hart 방정식 사용)
  steinhart = NTC_res / 10000.0;  // R/R0
  steinhart = log(steinhart);  // ln(R/R0)
  steinhart /= 3950.0;  // 1/B * ln(R/R0)
  steinhart += 1.0 / (25 + 273.15);  // + (1/T0)
  steinhart = 1.0 / steinhart;  // 역수
  steinhart -= 273.15;  // K -> °C
  return steinhart; // °C return
}


void FETctrl(int Parameter) // read VR, PWM OUTPUT
{
  int duty = Parameter * (255.0 / 1023.0);
  digitalWrite(pwmPin1, HIGH);
  analogWrite(pwmPin2, duty);
}


void printTMP(float HOT, float COOL)  // Serial OUT TMP
{
  Serial.println("Temperature: ");
  Serial.print(HOT);
  Serial.println(" °C (H)");
  Serial.print(COOL);
  Serial.println(" °C (C)");
}

void setup() {
  Serial.begin(9600);
  pinMode(pwmPin1, OUTPUT);
  pinMode(pwmPin2, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degree);
  lcd.createChar(1, PWMicon);
}


void loop() {
  unsigned long time = millis();

  int ADC1 = analogRead(tmpPin1); // read HOT
  int ADC2 = analogRead(tmpPin2); // read COOL
  int ADC3 = analogRead(pwmRead); // read PWM VR
  float duty = ADC3 * (100.0 / 1023.0);
  
  float TMP1 = sensingTMP(ADC1);
  float TMP2 = sensingTMP(ADC2);

  FETctrl(ADC3);
  
  if(time - pre_TMP_UART >= period_TMP_UART) // Serial UART Part  (period 1000ms)
  {
    pre_TMP_UART = time;
    printTMP(TMP1, TMP2);
    Serial.print(duty, 1);
    Serial.println("%");
  }

  if(time - pre_DISPLAY >= period_DISPLAY) // I2C 1602 Part  (period 100ms)
  {
    lcd.setCursor(0, 0);
    lcd.print("TMP(H/C) ");
    lcd.write(1); lcd.print(duty, 1); lcd.print("%  ");
    lcd.setCursor(0, 1);
    lcd.print(TMP1);
    lcd.setCursor(5, 1); lcd.write(0); lcd.print("C  ");
    lcd.print(TMP2);
    lcd.setCursor(14, 1); lcd.write(0); lcd.print("C");
  }
  
}
