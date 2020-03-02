#include <Wire.h>
#include <Password.h>
#include <Keypad.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

//////////SENSORs & LEDs /////////
int pirPin1 = 2;   // Bedroom Pir 
int pirPin2 = 34;   // Garage Pir
int proxPin1 = 8;  // Front Proximity
int proxPin2 = 9;  // Back Proximity
int greenLED = 30; 
int redLED = 32;
int LED_Relay = 14;  //LED Strip Relay 

///////////Password//////////
Password password = Password("2015");

///////Servo Motor Intialisation/////// 
Servo camservo;        // Servo Object Creation
int pos = 100;         // Intial Servo Position

///////Other Variables///////
int HomeZone =0;      
int pass_pos = 11;  // Password Input Position
int buzzerPin = 22; 
boolean systemActive = false;
boolean systemStatus = false;
int calibrationTime = 30; //the time we give the sensor to calibrate (10-60 secs according to the datasheet)
boolean motionGarage = false;
boolean motionBack = false;
boolean motionFront = false;
boolean motionBed = false;
//////////Keypad Creation/////////
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'}, 
  {'7', '8', '9'}, 
  {'*', '0', '#'} 
};
byte rowPins[ROWS] = {53, 51, 49, 47};  //Rows Pinouts
byte colPins[COLS] = {45, 43, 41};   //Columns Pinouts
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

////// Assignign arduino pins to LCD display module///////
LiquidCrystal_I2C lcd(0x27,16,2);


void setup() {
  
  Serial.begin(9600); // Setup Baidu Rate
 //Setup Sensors and Relay 
  pinMode(pirPin1, INPUT);  //Bedroom 
  pinMode(pirPin2, INPUT);  //Garage
  pinMode(proxPin1, INPUT); //Front door
  pinMode(proxPin2, INPUT); //Back door
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED_Relay, OUTPUT); //LED Strip 
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  digitalWrite(pirPin1, LOW);
  digitalWrite(pirPin2,LOW);
  lcd.init();
  lcd.init();
  lcd.backlight();
     calibrationScreen();
    // Default Display Screen
  camservo.attach(13); // Attach Servo to Pin 9
  keypad.addEventListener(keypadEvent);
  camservo.write(pos);
  
}    

void loop() {
  
  // Get Pin//
  keypad.getKey();
  //Check For Motion//
  if (systemActive == true){
  
   if (digitalRead(proxPin1) == LOW){
   delay(200);
   if(digitalRead(proxPin1) == LOW && !motionFront)
   {HomeZone = 0;
    motionFront = true; 
      motionBed= false;
     motionGarage = false; 
     motionBack = false;
    MotionDetected();}
   }
   if (digitalRead(proxPin2) == LOW){
   delay(200);
   if(digitalRead(proxPin2) == LOW && !motionBack )
   { HomeZone = 2; 
      motionBed= false;
     motionGarage = false; 
     motionFront = false;
     motionBack = true;
    MotionDetected();}
   }
   if (digitalRead(pirPin1) == HIGH){ 
     delay(100);
     if (digitalRead(pirPin1) == HIGH && !motionGarage)
   {HomeZone = 1;
     motionBed= false;
     motionGarage = true; 
     motionFront = false;
     motionBack = false;
     MotionDetected();}}
   if (digitalRead(pirPin2)== HIGH){ 
     delay(100);
     if (digitalRead(pirPin2) == HIGH && !motionBed)
     { HomeZone = 3;
     motionBed= true;
     motionGarage = false; 
     motionFront = false;
     motionBack = false;
     MotionDetected();}
     }
   
   }
   
  }
  
/////////////////////////  Functions  /////////////////////////////////
////Motion Detected////
void MotionDetected(){
  int expected_pos; // Excpexcted Servo Position
  int rotateSteps; // Steps Of Rotation Till Specified Angle
  digitalWrite(LED_Relay, HIGH);
  digitalWrite(buzzerPin,HIGH);
   password.reset();
  systemStatus = true;
  // LCD EDIT
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("BREACH..!!");
  lcd.setCursor(0,1);
  if (HomeZone == 0)
  { 
    lcd.print("Frontdoor Open");
    expected_pos = 120;
    delay(500);
   }
   if(HomeZone == 1){
    expected_pos = 125;
    lcd.print("Motion @ Bedroom");
    delay(500);
  }
  else if(HomeZone == 2){
    expected_pos = 150;
    lcd.print("Backdoor Open");
    delay(500);
  }
   else if(HomeZone == 3){
   expected_pos = 65;
   lcd.print("Motion @ Garage");
   delay(500);
 }
   
  if (expected_pos > pos)
  {rotateSteps = 1;} 
  else {rotateSteps = -1;}
   
   for (pos = pos; pos != expected_pos; pos += rotateSteps) {
    camservo.write(pos);                  // tell servo to go to position in variable 'pos' 
    delay(5);                            // waits 5ms for the servo to reach the position 
   }
   
}


//KEYPAD EVENTS //
void keypadEvent(KeypadEvent eKey) {
  switch (keypad.getState()) {
    case PRESSED:
      if (pass_pos - 11 >= 5) {
        return ;
      }
      lcd.setCursor((pass_pos++), 0);
      switch (eKey) {
        case '#':                 //# is to validate password
          pass_pos  = 11;
          checkPassword();
          break;
        case '*':                 //* is to reset password attempt
          password.reset();
          pass_pos = 11;
          break;
        default:
         password.append(eKey);
         lcd.print("*");
      }
  }
}
void checkPassword() {
    if (password.evaluate()) {
    if (systemActive == false && systemStatus == false){
    activate();
    }
   
    else if (systemActive == true || systemStatus == true)  {
    deactivate();
    }
  
  }
  else {
    invalidCode();
  
  }

}

void activate(){
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SECURITY SYSTEM");
    lcd.setCursor(4,1);
    lcd.print("ACTIVATED");
    digitalWrite(redLED,HIGH);
    digitalWrite(greenLED,LOW);
    systemActive = true;
    password.reset();
    delay(1000);

}

void deactivate() {
  
    systemStatus = false; 
    systemActive = false;
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    digitalWrite(buzzerPin,LOW);
    motionFront = false;
    motionBack = false; 
    motionBed = false;
    motionGarage = false;  
    lcd.clear();
    lcd.print("SECURITY SYSTEM");
    lcd.setCursor(2,1);
    lcd.print("DEACTIVATED");
    password.reset();
    delay(2000);
    displayScreen();

}
void invalidCode(){
  if (systemActive == false){
    lcd.clear(); 
    lcd.setCursor(1,0);
    lcd.print("INVALID CODE!");
    lcd.setCursor(0,1);
    lcd.print("PLEASE TRY AGAIN");
    systemActive = false;
    digitalWrite(redLED,LOW);
    digitalWrite(greenLED,HIGH);
    password.reset();
    delay(1000);
    displayScreen();
    }
    else {
    
    lcd.clear(); 
    lcd.setCursor(1,0);
    lcd.print("INVALID!");
    lcd.setCursor(0,1);
    lcd.print("PLEASE TRY AGAIN");
    motionGarage = false;
    motionBack = false;
    motionFront = false;
    motionBed = false;
    password.reset();
    delay(1000);
    loop();
    }
 }
void displayScreen()    // Dispalying start screen for users to enter PIN
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter PIN:");
  lcd.setCursor(0,1);
  lcd.print("ALWAYS ON GUARD!");
}
void calibrationScreen(){
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("WELCOME!");
  lcd.setCursor(0,1);
  lcd.print("Please Wait...");
  for(int i = 0; i < calibrationTime; i++){
      delay(1000);
      }
   displayScreen();

}



