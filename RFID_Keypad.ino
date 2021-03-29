
/*
Eron Vieira 
*/

#include <EEPROM.h>
#include <Wire.h>   
#include <LiquidCrystal_I2C.h>  
#include <Password.h> 
#include <Keypad.h>   
#include <SPI.h>
#include <RFID.h>
#define SS_PIN 10
#define RST_PIN 9

RFID rfid(SS_PIN,RST_PIN);
int serNum[5];
int cards[][5] = {{84,113,62,254,229},{138,83,202,85,70}};
bool access = false;
LiquidCrystal_I2C lcd(0x27,16,2);
Password password = Password("000021");  
const byte ROWS = 4; // Four rows
const byte COLS = 4; //  columns

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = { 6,5,4,3 };
byte colPins[COLS] = { A3,A2,A1,A0, };
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

const int buzzer = 7; 
const int doorLock = 8; 

int alarm = 0;
uint8_t alarmStat = 0;
uint8_t maxError = 7;

unsigned long previousMillis = 0; 
const long interval = 1000;  
uint8_t pwMode=0;
uint8_t pwPos=0;

void setup(){
  
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    SPI.begin();
    rfid.init();
    keypad.addEventListener(keypadEvent);
    pinMode(doorLock, OUTPUT);
    digitalWrite(doorLock, LOW);
    lcd.setCursor (0,0);
    lcd.print(F("  RFID System   "));
    lcd.setCursor (0,1);
    lcd.print(F(" Control Access "));
    delay (1000);
    lcd.clear();
    lcd.print("Verificando....");   
    lcd.setCursor(4,1);
    for(int x = 0; x < 19; x++) {delay(30);lcd.setCursor(x,1);lcd.write(B11111111);}
    lcd.clear();
    tone (buzzer,900);    
    delay(200);  
    noTone (buzzer);
}

void loop(){
  keypad.getKey(); 
  if (alarm >= maxError){
  alarmStat = 1;    } 
  if (alarmStat == 0 && pwMode == 0){
  lcd.setCursor (0,0);
  lcd.print(F("  -Bem Vindo-   "));
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
  //previousMillis = currentMillis;
  lcd.setCursor (0,1);
  lcd.print(F(" Aproxime o Card  "));
  }
  if (currentMillis - previousMillis >= (4*interval)) {
      previousMillis = currentMillis;
  lcd.setCursor (0,1);
  lcd.print(F(" Digite a Senha   "));
  } 
   if(rfid.isCard()){    
        if(rfid.readCardSerial()){
            Serial.print(rfid.serNum[0]);
            Serial.print(" ");
            Serial.print(rfid.serNum[1]);
            Serial.print(" ");
            Serial.print(rfid.serNum[2]);
            Serial.print(" ");
            Serial.print(rfid.serNum[3]);
            Serial.print(" ");
            Serial.print(rfid.serNum[4]);
            Serial.println("");            
            for(int x = 0; x < sizeof(cards); x++){
              for(int i = 0; i < sizeof(rfid.serNum); i++ ){
                  if(rfid.serNum[i] != cards[x][i]) {
                      access = false;
                      break;
                  } else {
                      access = true;
                  }
              }
              if(access) break;
            }   
        }
        
       if(access){
          lcd.setCursor (0,0);
          delay(100);
          lcd.print(F(" Acesso Liberado "));
          lcd.setCursor (0,1);
          lcd.print(F("   Bem Vindo     "));          
          ACCEPT ();
           }       
      else {
           alarm = alarm+1;
           lcd.setCursor (0,0);
           lcd.print(F(" Nao Cadastrado!  "));
           lcd.setCursor (0,1);
           lcd.print("  Sem Aceso!!     ");
           RIJECT ();    
       }        
    }
 rfid.halt();
}
if (alarmStat == 1) {
  lcd.setCursor (0,0);
  lcd.print(F("Systema Blokeado"));
  lcd.setCursor (0,1);
  lcd.print(F(" Aguarde!!... "));
for(int i=20; i>0; i--){
tone(buzzer, 1200);
delay(100);
noTone(buzzer);
lcd.setCursor (13,1); lcd.print(i);
lcd.print(F("  "));delay (1000);}
noTone (buzzer);
alarmStat = 0;
alarm = 0;
  }
}
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
  case PRESSED:
  Serial.print("Tecla: ");
  Serial.println(eKey); 
  pwMode =1;
  pwPos=pwPos+1;
  if(pwPos==1){lcd.clear();}
  lcd.setCursor (0,0);
  lcd.print(F("Digite a Senha"));
  if(pwPos<7){lcd.setCursor (3+pwPos,1);

  lcd.print(F("*"));} 
  switch (eKey){
  case 'D': checkPassword(); break;
  case '*': password.reset(); break;    
    default: password.append(eKey);
     }
  }
}
void checkPassword(){
  if (password.evaluate()){
    Serial.println("Password OK");
    lcd.setCursor (0,1);
    lcd.print(F("   -Liberado-   "));
    ACCEPT ();
    password.reset();
    pwPos=0;
  }else{
    Serial.println("SENHA ERRADA!!");
    lcd.setCursor (0,0);
    lcd.print(F(" SENHA ERRADA!!  "));
    lcd.setCursor (0,1);
    lcd.print(F("Tente novamente "));
    alarm = alarm+1;
    RIJECT ();
    password.reset();
    pwPos=0;
  } 
}
void ACCEPT () {
           tone (buzzer,900);
           delay(100);
           tone (buzzer,1200);
           delay(100);
           tone (buzzer,1800);
           delay(200);
           noTone(buzzer);  
           delay(600);
           lcd.clear();
           lcd.setCursor (0,0);
           lcd.print(F("Liberando systema "));        
           lcd.setCursor(0,1);
           for(int x = 0; x < 16; x++) {delay(60);lcd.setCursor(x,1);lcd.write(B11111111);}            
           {
           delay (1000);
           }
           digitalWrite(doorLock, HIGH);
           delay(300);
           digitalWrite(doorLock, LOW);
           pwMode =0;
           lcd.clear();}
void RIJECT () {
           tone (buzzer,900);
           delay(200);
           noTone(buzzer);
           delay(200);
           tone (buzzer,900);
           delay(200);
           noTone (buzzer);  
           delay(500);
           pwMode =0;
           lcd.clear();  
}

