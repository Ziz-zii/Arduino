/*передача данных сначало передаёт акДфту и читает майДату*///ПРИЁМНИК


#include <SPI.h>                    // Подключаем библиотеку для работы с шиной SPI.
#include <nRF24L01.h>               // Подключаем файл настроек из библиотеки RF24.
#include <RF24.h>                   // Подключаем библиотеку для работы с nRF24L01+.

#include <Wire.h>   //Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_SHT.h>    // Подключаем библиотеку для работы с датчиком температуры и влажности I2C-flash.
#include <iarduino_I2C_DSL.h>  //Подключаем библиотеку для датсяика освещённости.
#include <iarduino_Pressure_BMP.h>
#include <iarduino_I2C_Encoder.h>


/*-----------------------------------------------------------------------------------*/ 
//радио модуль
RF24 radio(9,53);
int sendData[4]; // массив для приёма данных
int acceptData[4]; //массив для отправки данных 

//Вентилятор----------------------------------------------
int ventVal = 0;
//Энкодер-------------------------------------------------
iarduino_I2C_Encoder enc;
int push = 0;
/*-----------------------------------------------------------------------------------*/
//Газ датчик---------------------------------------------------
int8_t gasPin = A5;// Определяем номер вывода, к которому подключен модуль
int8_t gasPwr = 22;// Определяем номер вывода, к которому подключено управление нагревателя модуля
/*-----------------------------------------------------------------------------------*/
// датчик Т и Вл-------------------
iarduino_I2C_SHT sht; 
/*-----------------------------------------------------------------------------------*/
//датчик света---------------------
iarduino_I2C_DSL dsl; 
/*-----------------------------------------------------------------------------------*/
//Барометр-------------------------
iarduino_Pressure_BMP bar;
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/


void setup(){     
  //радио передача//----------------------------------------------------------
 Serial.begin(9600);
  radio.begin           ();                                  // Инициируем работу модуля nRF24L01+.
  radio.setChannel      (27);                                // Указываем канал передачи данных (от 0 до 125), 27 - значит передача данных осуществляется на частоте 2,427 ГГц.
  radio.setDataRate     (RF24_1MBPS);                        // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек.
  radio.setPALevel      (RF24_PA_MAX);                       // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm).
  radio.enableAckPayload();                                  // Указываем что в пакетах подтверждения приёма есть блок с пользовательскими данными.
//  radio.enableDynamicPayloads();                             // Разрешить динамически изменяемый размер блока данных на всех трубах.
  radio.openReadingPipe (1, 0xAABBCCDD11LL);                    // Открываем трубу с адресом 0xAABBCCDD11 для передачи данных (передатчик может одновременно вещать только по одной трубе).
  radio.startListening  ();                                  // Включаем приемник, начинаем прослушивать открытые трубы.
  radio.writeAckPayload (1, &acceptData, sizeof(acceptData));      //отправляет актДату
    
//Датчики шины I2C-----------------------
delay(500);
while(!Serial){;}
enc.begin();  enc.setPosSettings(5, false);  enc.resPosition();
sht.begin(); 
dsl.begin();
bar.begin(); 

pinMode (12, OUTPUT);
pinMode (3, OUTPUT);





/*-----------------------------------------------------------------------------------*/

}     
/*-----------------------------------------------------------------------------------*/
void loop() {
  acceptData[0]=sht.getTem();
  acceptData[1]=sht.getHum();
  //acceptData[2]=analogRead(gasPin);
  //acceptData[3]=bar.pressure;
  
 
  if( radio.available() ){
    radio.read            (   &sendData,  sizeof(sendData)  ); // Читаем данные из буфера приёма в массив myData указывая сколько всего байт может поместиться в массив.
    radio.writeAckPayload (1, &acceptData, sizeof(acceptData) ); // Помещаем данные всего массива ackData в буфер FIFO для их отправки на следующее получение данных от передатчика на 1 трубе. 
  }
  
/*
  Serial.print( acceptData[0]);Serial.print('\t');
  Serial.print(acceptData[1]); Serial.print('\t');
  Serial.print(acceptData[2]); Serial.print('\t');
  Serial.println( acceptData[3]); */

Serial.println( sendData[0]);/*Serial.print('\t');
  Serial.print(sendData[1]); Serial.print('\t');
  Serial.print(sendData[2]); Serial.print('\t');
  Serial.println (sendData[3]);
*/

    
buttonCheck ();
 luxCheck() ;


}
/*-----------------------------------------------------------------------------------*/ 
 
/*-----------------------------------------------------------------------------------*/
int buttonCheck (){
    if( enc.getButton(KEY_PUSHED) ){              //   Если кнопка энкодера нажимается, то ...
       push = 1; 
    }   
    if( enc.getButton(KEY_TIME_PRESSED) > 1000 ){ //   Если время удержания кнопки возвращаемое функцией getButton больше 5000 миллисекунд, то...
       push = 2;}
 
    switch(push){
      case 0: modeA(); break;
      case 1: modeA(); break;
      case 2: modeB(); break;   
 }   
  }
int modeA (){
  int pose;
  int t = sht.getTem();
  int g = analogRead(gasPin);
  int sum = g+t;
  pose = map(sum, 0, 700, 0, 255); 
  analogWrite(12 ,255 - pose);
  if (pose > 200){
  analogWrite(8, 100);analogWrite(9, 0);
  }
  else{
  analogWrite(9, 100);analogWrite(8, 0);
  
 
}}
int modeB () {
  
  int pose = enc.getPosition();
  pose = map(pose, 0, 100, 0, 255); 
   analogWrite(12 ,255 - pose);

    if (pose > 200){
  analogWrite(8, 100);analogWrite(9, 0);
  }
  else{
  analogWrite(9, 100);analogWrite(8, 0);
  }
  }
/*----------------------------------------------------------------------------*/

int luxCheck( ){
  int lux =  dsl.getLux();
  lux = map(lux, 0, 3000, 0, 10);
  int slider = sendData[0];
  slider = map(slider, 0, 1023, 0, 100);
  //Serial.println (lux);
  if (slider >= 1){
    if (slider < 10){ digitalWrite(3, 0);}
      else{digitalWrite(3,slider);}
    }
    else{
      if (lux >7){ digitalWrite(3, 0);}
      else{digitalWrite(3, 8 - lux);}
  }
}












 
