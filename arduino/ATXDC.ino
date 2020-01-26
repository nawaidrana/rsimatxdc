

//ATXDC.INO
//WRITTEN BY DIEGO VISO 2014
//REQUIRES ADAFRUIT MOTOR CARD
/*THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE*/

// NOTE: STRING FROM COMPUTER MUST NOT END WITH A CARRIAGE RETURN FOR THIS CODE TO WORK
//WHEN YOU ARE CONDUCTING THE TEST EXPRESSION YOU MAY NEED TO ADD THE NEW LINE KEY

//#include <AnalogEvent.h>
//#include <ButtonEvent.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <Servo.h>

// Create the motor shield object with the default I2C address
#include "Arduino.h"
void setup();
void loop();

int ConvertStringToInt(String l);

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *dcMotor0 = AFMS.getMotor(1);
Adafruit_DCMotor *dcMotor1 = AFMS.getMotor(2);
Adafruit_DCMotor *dcMotor2 = AFMS.getMotor(3);



Servo Servo1;

String inString = "";
String sensorValue = "";
String atxdcversion = "ATXDCv1.0";

bool eol = false;

int bytesToRead = 0;
int eventsEnabled = 0;
int motorTestEnabled = 0;
int motor0direction = 0 ; //0=foward 1=backwards
int motor1direction = 0 ; //0=foward 1=backwards
int motor2direction = 0 ; //0=foward 1=backwards
int motor0spd = 140;
int motor1spd = 200;
int motor2spd = 140;
//define the min and max value that the motor can move the levers to based on pot inputs
int motor0MaxValue=0;
int motor0MinValue=0;
int motor1MaxValue=0;
int motor1MinValue=0;
int trimWheelCount = 100;

int intTemp;
char carray[6];



void setup() 
{
  

  digitalWrite(4, HIGH); 
  digitalWrite(5, HIGH); 
  digitalWrite(6, HIGH); 
  digitalWrite(7, HIGH); 
  digitalWrite(8, HIGH); 
  digitalWrite(13, HIGH); 



  
    AFMS.begin();  

    dcMotor0->setSpeed(motor0spd);
    dcMotor1->setSpeed(motor1spd);
    dcMotor2->setSpeed(motor2spd);
    
    dcMotor0->run(RELEASE);
    dcMotor1->run(RELEASE);
    dcMotor2->run(RELEASE);

  
  //leonardo wont display Serial.print data in setup function because
  //the serial interface is not reset like the arduino uno
    Serial.begin(115200); // same as in your c# script
    while(!Serial);
    Serial.println(atxdcversion);
    Serial.flush();
    
}

void loop() 
{
 
  
     if(Serial.available() > 0)
      {
      //  delay(10); // DELAY REQUIRED TO FILL THE BUFFER PLUS SLOW DOWN ANALOG READ CYCLE
   
        while (Serial.available() > 0 && !eol)
        { 
          
          inString += (char)Serial.read();
          if(inString.endsWith("\n"))
               eol = true;
       
         }

        
      }
 //----------------------------------------------------------------------------
 //----------Decode incoming scring routine
      if(eol == true)
      {
          
        if (inString == String("atxdcversion\n"))
            Serial.println(atxdcversion);
        
        if (inString == String("A0\n"))
          sensorValue = String("A0:") + String(analogRead(0));
        if (inString == String("A1\n"))
          sensorValue = String("A1:") + String(analogRead(1));
        if (inString == String("A2\n"))
          sensorValue = String("A2:") + String(analogRead(2));
        if (inString == String("A3\n"))
          sensorValue = String("A3:") + String(analogRead(3));
        if (inString == String("A4\n"))
          sensorValue = String("A4:") + String(analogRead(4));
        if (inString == String("A5\n"))
          sensorValue = String("A5:") + String(analogRead(5));
        if (inString == String("D4\n"))
          sensorValue = String("D4:") + String(digitalRead(4));
        if (inString == String("D5\n"))
          sensorValue = String("D5:") + String(digitalRead(5));
        if (inString == String("D6\n"))
          sensorValue = String("D6:") + String(digitalRead(6));
        if (inString == String("D7\n"))
          sensorValue = String("D7:") + String(digitalRead(7));
        if (inString == String("D8\n"))
          sensorValue = String("D8:") + String(digitalRead(8));
        if (inString == String("D13\n"))
          sensorValue = String("D13:") + String(digitalRead(13));
        if (inString == String("SA1\n"))
          Servo1.attach(1);
        if (inString == String("SD1\n"))
          Servo1.detach();
        if (inString.startsWith("S1:")==true && inString.endsWith("\n") == true)
            Servo1.write(ConvertStringToInt(inString.substring(3)));




        if (inString == String("showvar\n"))
           { 
            Serial.print("A0Min:"+String(motor0MinValue));
            Serial.print(" A0Max:"+String(motor0MaxValue));
            Serial.print(" A1Min:"+String(motor1MinValue));
            Serial.println(" A1Max:"+String(motor1MaxValue));
           }

        
        //motor format "m" "motornumber" "f=forward/b=backward" : "speed"
        if (inString.startsWith("m0setspeed:")==true && inString.endsWith("\n") == true)
              motor0spd = ConvertStringToInt(inString.substring(11));
        if (inString.startsWith("m1setspeed:")==true && inString.endsWith("\n") == true)
                 motor1spd = ConvertStringToInt(inString.substring(11));
        if (inString.startsWith("m2setspeed:")==true && inString.endsWith("\n") == true)
                 motor2spd = ConvertStringToInt(inString.substring(11));

        if (inString.startsWith("m0minvalue:")==true && inString.endsWith("\n") == true)
                 motor0MinValue = ConvertStringToInt(inString.substring(11));
        if (inString.startsWith("m1minvalue:")==true && inString.endsWith("\n") == true)
                 motor1MinValue = ConvertStringToInt(inString.substring(11));
        if (inString.startsWith("m0maxvalue:")==true && inString.endsWith("\n") == true)
                 motor0MaxValue = ConvertStringToInt(inString.substring(11));
        if (inString.startsWith("m1maxvalue:")==true && inString.endsWith("\n") == true)
                 motor1MaxValue = ConvertStringToInt(inString.substring(11));

        if (inString.startsWith("m0f:")==true && inString.endsWith("\n") == true)
            {
              dcMotor0->setSpeed(ConvertStringToInt(inString.substring(4)));

              if(analogRead(A0) < motor0MaxValue) 
                dcMotor0->run(FORWARD);
              else
                dcMotor0->run(RELEASE);
            }
        if (inString.startsWith("m0b:")==true && inString.endsWith("\n") == true)
            {
              dcMotor0->setSpeed(ConvertStringToInt(inString.substring(4)));
              if(analogRead(A0) > motor0MinValue) 
                dcMotor0->run(BACKWARD);
              else
                dcMotor0->run(RELEASE);
            }
        if (inString.startsWith("m0s")==true && inString.endsWith("\n") == true)
              dcMotor0->run(RELEASE);
        
        if (inString.startsWith("m1f:")==true && inString.endsWith("\n") == true)
            {
              dcMotor1->setSpeed(ConvertStringToInt(inString.substring(4)));

              if(analogRead(A1) < motor1MaxValue) 
                dcMotor1->run(FORWARD);
              else
                dcMotor1->run(RELEASE);
            }
        if (inString.startsWith("m1b:")==true && inString.endsWith("\n") == true)
            {
              dcMotor1->setSpeed(ConvertStringToInt(inString.substring(4)));
              if(analogRead(A1) > motor1MinValue) 
                dcMotor1->run(BACKWARD);
              else
                dcMotor1->run(RELEASE);
            }
        if (inString.startsWith("m1s")==true && inString.endsWith("\n") == true)
              dcMotor1->run(RELEASE);

        if (inString.startsWith("m2f:")==true && inString.endsWith("\n") == true)
            {
              dcMotor2->setSpeed(ConvertStringToInt(inString.substring(4)));
              dcMotor2->run(FORWARD);
              
            }
        if (inString.startsWith("m2b:")==true && inString.endsWith("\n") == true)
            {
              dcMotor2->setSpeed(ConvertStringToInt(inString.substring(4)));
              dcMotor2->run(BACKWARD);
              
            }
        if (inString.startsWith("m2s")==true && inString.endsWith("\n") == true)
              dcMotor2->run(RELEASE);

    
        if(sensorValue != "")
         {  
         //  Serial.flush();
           Serial.print(sensorValue + "\n");
           sensorValue = "";
         }
        
        inString = "";
        eol = false;
      }
 

        
 delay(1); //needed to slow down leonardo from filling serial  

 }



int ConvertStringToInt(String l)
{
   l.toCharArray(carray, sizeof(carray));
   intTemp = atoi(carray);
   return intTemp;
}

