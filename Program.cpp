#include "LiquidCrystal.h"
#include "Wire.h"
#include "WiFi.h"
#include "ThingSpeak.h"

#define CHANNEL_ID "************" //ThingSpeak Channel
#define CHANNEL_API_KEY "**************" //Key to Channel
#define WIFI_NETWORK "******" 
#define WIFI_PASSWORD "*******"
#define WIFI_TIMEOUT_MS 40000
#define YELLOW_BUTTON 22
#define RED_BUTTON 33
#define GREEN_BUTTON 21
#define BLUE_BUTTON 4
#define BUZZER 27
#define LED 26


int secondsBreak=15;
int minutesBreak=0;
int decsecBreak=0;

int minutesBreakToSave=0;
int secondsBreakToSave=15;

int isAlarmOn=0;

int decsec;    // Working time counter 
int second;
int minutes;
int hours;

int mode=2;               // Modes 
                          // 0 = working time
                          // 1 = break
                          // 2 = ready to work
int cursorHours;                          
int cursorMinute;                  // cursor positions
int cursorSeconds;
int buttonPressed(int button); // Function to handle button events
//void displayTime(int minuty, int sekundy, int dziesiate_sekundy); // Function to display time
void displayTime(int hours, int minutes, int seconds); //Function to display time
void displayBreakTime(int minutes, int seconds);

unsigned long actualTime  = 0; // actual working time
unsigned long previousTime = 0; // previous time
unsigned long deltaTime  = 0; // actual time - working time

int resultHours; //hours
int resultMinutes; // minutes
int resultSeconds; // seconds
int resultDecSeconds; // 0.1 * second

//Lcd display
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

//WiFi Client
WiFiClient client;

//Function to connect to wifi
bool connectToWifi();





void setup() 
{

  pinMode(YELLOW_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  Serial.begin(9600);
  lcd.begin(16,2);

  if(connectToWifi())
  delay(1000);
  ThingSpeak.begin(client);
  lcd.begin(16, 2);     
  lcd.clear();          
  lcd.setCursor(0,0);   
  lcd.print("The device" );
  lcd.setCursor(0,1);
  lcd.print("is ready!");
  delay(3000);
}
 
void loop()
{

  
  if (buttonPressed(GREEN_BUTTON)){
    int previousMode=mode;
    mode=0;
    if(previousMode!=mode){
  if(WiFi.status()!=WL_CONNECTED)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi");
  lcd.setCursor(0,1);
  lcd.print("failed");
  delay(1000);
}else{

  isAlarmOn=0;
  digitalWrite(BUZZER,LOW);
  digitalWrite(LED, LOW);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Start working!");
  delay(200);
  ThingSpeak.writeField(CHANNEL_ID, 2, 10, CHANNEL_API_KEY);
}}

  }
  if (buttonPressed(RED_BUTTON)) {
    int previousMode=mode;
    mode=1;
    if(previousMode!=mode){
    if(WiFi.status()!=WL_CONNECTED)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi");
        lcd.setCursor(0,1);
        lcd.print("failed");
        delay(1000);
      }else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Break Time!");
        ThingSpeak.writeField(CHANNEL_ID, 2, 0, CHANNEL_API_KEY);
        delay(200);
   }}


  }

  if(buttonPressed(BLUE_BUTTON))
  {
    mode=3;
  }

  
  actualTime = millis();
  deltaTime = actualTime - previousTime;

    if (deltaTime>=100UL) 
        {
         decsec ++;
         if (decsec>9){second++;decsec=0;}
         if (second>59){minutes++;second=0;}
         if(minutes>59){hours++; minutes=0;}

      

        if(mode==1)
        {
          
            
             decsecBreak--;
             if(decsecBreak<0)
             {
               secondsBreak--; decsecBreak=9;
             }
             if(secondsBreak<0)
             {
                secondsBreak=59; 
                minutesBreak--;
             }

             if(minutesBreak<0)
            {
             minutesBreak=0;
             secondsBreak=0;
             isAlarmOn=1;
            }
           
        }

         previousTime=actualTime;
                      
              if (mode==0) // Pomiar czasu
              {
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Working time");
                displayTime(resultHours, resultMinutes, resultSeconds);
                resultMinutes=minutes;
                resultSeconds=second;
                resultDecSeconds=decsec;
                resultHours=hours;
                secondsBreak=secondsBreakToSave;
                minutesBreak=minutesBreakToSave;
                if(isAlarmOn==1)
                {
                  isAlarmOn=0;
                }
              }
  
               if (mode==1) // Wynik
              {
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Break time");
                displayBreakTime(minutesBreak, secondsBreak);
                previousTime=millis();               
                minutes=resultMinutes;
                second=resultSeconds;
                decsec=resultDecSeconds;
                hours=resultHours;

                   if(isAlarmOn==1)
                     {
                       digitalWrite(BUZZER,HIGH);
                       digitalWrite(LED, HIGH);

                     }
              }
              
              if (mode==2) //Gotowy do pracy
              { 
                hours=0;
                minutes=second=decsec=0;
                minutesBreak=0;
                secondsBreak=15;
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Press green");
                lcd.setCursor(0,1);
                lcd.print("to start");
               
                
              }

              if(mode==3&&isAlarmOn==0)
              {

                previousTime=millis();               
                minutes=resultMinutes;
                second=resultSeconds;
                decsec=resultDecSeconds;
                hours=resultHours;

                lcd.clear();
                lcd.home();
                lcd.print("Set break time");
                lcd.setCursor(0,1);
                displayBreakTime(minutesBreak,secondsBreak);
                if(buttonPressed(YELLOW_BUTTON))
                {
                  secondsBreak++;
                  if(secondsBreak>59)
                  {minutesBreak++;; secondsBreak=0;}
                  if(minutesBreak>29)
                  {
                    minutesBreak=0; secondsBreak=0;
                  }
                }

                  if(buttonPressed(BLUE_BUTTON))
                {
                  secondsBreak--;
                  if(secondsBreak<0)
                  {minutesBreak--;; secondsBreak=59;}
                  if(minutesBreak<0)
                  {
                    minutesBreak=0; secondsBreak=0;
                  }
                }

                minutesBreakToSave=minutesBreak;
                secondsBreakToSave=secondsBreak;

              }
        }                
}

int buttonPressed(int button)
{
  if (digitalRead(button)==0)
      {
        delay(100);
        if(digitalRead(button)==0)return 1;
      }
  return 0;
}


void displayTime(int hours, int minutes, int seconds)
{
      if (hours>9) cursorHours=5; //Display hours
          else    
          {cursorHours=6;          
           lcd.setCursor(5,1);
           lcd.print(0);} 

      
      if (minutes>9) cursorMinute=8; //Display minutes  
            else    
          {cursorMinute=9;            
           lcd.setCursor(8,1);
           lcd.print(0);}
      
       if (seconds>9) cursorSeconds=11; //Display minutes  
            else    
          {cursorSeconds=12;            
           lcd.setCursor(11,1);
           lcd.print(0);}

      lcd.setCursor(0,1);               
      lcd.print("Time:");
             
      lcd.setCursor(cursorHours,1);   
      lcd.print(hours);

      lcd.setCursor(7,1);               
      lcd.print(":");
      
      lcd.setCursor(cursorMinute,1);  
      lcd.print(minutes);
      
      lcd.setCursor(10,1);               
      lcd.print(":");  
      
      lcd.setCursor(cursorSeconds,1);               
      lcd.print(seconds);
}


void displayBreakTime(int minutes, int seconds)
{
   if (minutes>9) cursorMinute=5; 
          else    
          {cursorMinute=6;         
           lcd.setCursor(5,1);
           lcd.print(0);} 

      
      if (seconds>9) cursorSeconds=8;   
          else    
          {cursorSeconds=9;            
           lcd.setCursor(8,1);
           lcd.print(0);}

      lcd.setCursor(0,1);             
      lcd.print("Time:");
             
      lcd.setCursor(cursorMinute,1);   
      lcd.print(minutes);

      lcd.setCursor(7,1);               
      lcd.print(":");
      
      lcd.setCursor(cursorSeconds,1);  
      lcd.print(seconds);
      
}


bool connectToWifi(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting");
  lcd.setCursor(0,1);
  lcd.print("to wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  unsigned long startAttempTime=millis();

while(WiFi.status()!=WL_CONNECTED&&millis()- startAttempTime<WIFI_TIMEOUT_MS)
{
  lcd.setCursor(7,1);
  delay(100);
  lcd.print(".");
  delay(100);
  lcd.setCursor(8,1);
  lcd.print(".");
  delay(100);
  lcd.setCursor(9,1);
  lcd.print(".");
  delay(100);
  lcd.setCursor(7,1);
  lcd.print("   ");
  delay(100);
}
if(WiFi.status()!=WL_CONNECTED)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connection");
  lcd.setCursor(0,1);
  lcd.print("failed");
  return false;
}
else if(WiFi.status()==WL_NO_SSID_AVAIL)
{
  lcd.clear();
  lcd.print("no ssdi ");
  return false;
}
else{
  lcd.clear();
  lcd.print("Connected ");
  return true;
}
}

