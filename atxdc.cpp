//---------------------------------------------------------------------------
//ATXDC.CPP
//WRITTEN BY DIEGO VISO (C) 2014
//REQUIRES ARDUINO AND ADAFRUIT MOTOR CARD

//CODE NOT COMPLETE HOWEVER CAN BE USED AS A BASE FOR YOUR OWN APPLICATION



/*THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE*/


#pragma hdrstop

#include "Unit1.h"

#include "fsuipc.h"
#include <time.h>
#include <IniFiles.hpp>
#include <system.math.hpp>



#include <stdio.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)

#pragma link "CPort"
#pragma resource "*.dfm"

#define SetPortA SetPort

#define MAX_THROTTLES 2
#define PROGRAMVERSION 1.0

TFormMain *FormMain;

ENGINE_VARIABLES engineVariables;
SPOILER_VARIABLES spoilerVariables;
SERVO_SETTINGS servoSettings[8];
DVATX_SETTINGS dvatxSettings;
TQ_VARIABLES tqVariables;
TRIM_VARIABLES trimVariables;

int ArduinoAnalogSensorValue[8];

//ServoSettings

//Global Variables;
int PhidgetsServoAdvancedCardConnected = 0;

int PhidgetsInterfaceKitCardConnected = 0;
int PhidgetsHCMotorCardConnected = 0;

int Activated = 0;

int trimWheelWaitIntervalCount=0;
int trimWheelDisabled=0;

int SensorTarget1LastPercentage=0;
int SensorTarget2LastPercentage=0;


 //i need to know this globaly: 0 = off, 1=on

bool SerialReadTimeOut = false;

int hysteresis = 5; //used to ensure we update only when there is a big enough change in analogsensor





//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{

}

void __fastcall TFormMain::TimerFSUIPCProcessTimer(TObject *Sender)
{
/*
short int    2      16          -32,768 -> +32,767          (32kb)
   unsigned short int    2      16                0 -> +65,535          (64Kb)
		 unsigned int    4      32                0 -> +4,294,967,295   ( 4Gb)
				  int    4      32   -2,147,483,648 -> +2,147,483,647   ( 2Gb)
			 long int    4      32   -2,147,483,648 -> +2,147,483,647   ( 2Gb)
		  signed char    1       8             -128 -> +127
		unsigned char    1       8                0 -> +255
				float    4      32
			   double    8      64
		  long double   12      96
*/



if (LabelFSUIPC->Caption == "FSUIPC Ver: ")
	return;


StopAllTimers();
//TimerFSUIPCProcess->Enabled=false; //lets disable the timer until we finish this process

//check if we are connected to fsuipc



//MessageBox(NULL,"Error","TEST",MB_OK);
	char  c_string[32], chTime[3];



	BOOL fProcessOK = TRUE;



//PROCESSING IS DONE WITH TIME REQUEST
if (!dwResult)
	{

	if (!FSUIPC_Read(0x238, 3, chTime, &dwResult) || 		// If we wanted other reads/writes at the same time, we could put them here
		!FSUIPC_Process(&dwResult)) // Process the request(s)
		fProcessOK = FALSE;

	// Now display all the knowledge we've accrued:
	if (fProcessOK) {

				sprintf(c_string, "%02d:%02d:%02d", chTime[0], chTime[1], chTime[2]);
				LabelFSTime->Caption = "Flight Sim Time: " + String(c_string);
		}
		else {
				LabelFSTime->Caption = "Flight Sim Time: Failed";
		}

  }



//SPOILER HANDLE
  UpdateSpoilerServoPosition();

//Update analog sensor readings

//no fsuipc_process done in this routine so REV value is able to ovewrite
//normal thrust value and allows for updatedigitalinputstatus to be used for REV
//thrust
 UpdateAnalogSensors();


 // Update Buttons

UpdateDigitalInputStatus();

//Update trim wheel
UpdateTrimWheel();


//update LED lights

UpdateDigitalOutputStatus();


 //returns current flightsim values and updates labels
UpdateFSUIPCReturnedOffsetValuesLabels();

//  TimerFSUIPCProcess->Enabled=true;
StartAllTimers();

	}

//---------------------------------------------------------------------------

void __fastcall TFormMain::FormActivate(TObject *Sender)
{
if (Activated==1)
		return;
Activated=1;


FormMain->InitializeApp();
	}
//----------------------------------------------------------------------------

void TFormMain::InitializeApp()
{
const char *version;

TIniFile *ini;

//load settings

 ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));

			FormMain->TimerFSUIPCProcess->Interval = ini->ReadInteger("SETTINGS", "FSUIPCREFRESH", 100 );
			dvatxSettings.TQAcceleration  =  ini->ReadInteger("SETTINGS", "ACCELERATION", 5000 );

			dvatxSettings.MCVelocity   =  ini->ReadInteger("SETTINGS", "MCVELOCITY", 100 );
			dvatxSettings.MCAcceleration  =  ini->ReadInteger("SETTINGS", "MCACCELERATION", 10 );




	   //read servo 0
			servoSettings[0].MinPos   =  ini->ReadInteger("SERVO_0", "MINPOS", 100 );
			servoSettings[0].MaxPos   =  ini->ReadInteger("SERVO_0", "MAXPOS", 101 );
			servoSettings[0].Inverted   =  ini->ReadInteger("SERVO_0", "INVERTED", 0 );
	   //read servo 1
			servoSettings[1].MinPos   =  ini->ReadInteger("SERVO_1", "MINPOS", 100 );
			servoSettings[1].MaxPos   =  ini->ReadInteger("SERVO_1", "MAXPOS", 101 );
			servoSettings[1].Inverted   =  ini->ReadInteger("SERVO_1", "INVERTED", 1 );
	   //read servo 2
			servoSettings[2].MinPos   =  ini->ReadInteger("SERVO_2", "MINPOS", 100 );
			servoSettings[2].MaxPos   =  ini->ReadInteger("SERVO_2", "MAXPOS", 101 );
			servoSettings[2].Inverted   =  ini->ReadInteger("SERVO_2", "INVERTED", 1 );
		//read servo 3
			servoSettings[3].MinPos   =  ini->ReadInteger("SERVO_3", "MINPOS", 100 );
			servoSettings[3].MaxPos   =  ini->ReadInteger("SERVO_3", "MAXPOS", 101 );
			servoSettings[3].Inverted   =  ini->ReadInteger("SERVO_3", "INVERTED", 1 );

 delete ini;

//UPDATE LABELS


InitializeArduino();

//we assume Arduino is available so lets start the timer
TimerScanSensorInputs->Enabled = true;



//OPEN FSUIPC
char  c_string[32], chMsg[128];

if (FSUIPC_Open(SIM_ANY, &dwResult)) {

			// Okay, we're linked, and already the FSUIPC_Open has had an initial
			// exchange with FSUIPC to get its version number and to differentiate
			// between FS's.

				sprintf(c_string, "%c.%c%c%c%c",
						'0' + (0x0f & (FSUIPC_Version >> 28)),
					'0' + (0x0f & (FSUIPC_Version >> 24)),
					'0' + (0x0f & (FSUIPC_Version >> 20)),
					'0' + (0x0f & (FSUIPC_Version >> 16)),
						(FSUIPC_Version & 0xffff) ? 'a' + (FSUIPC_Version & 0xff) - 1 : ' ');

				LabelFSUIPC->Caption = String("FSUIPC Ver: ") + c_string;
				LabelFSUIPC->Color = clLime;
				LabelFSVer->Caption = "FS Ver: " + String(pFS[FSUIPC_FS_Version - 1]);
				LabelFSVer->Color = clLime;

				StatusBar1->SimpleText = "Link established to FSUIPC";
				TimerFSUIPCAutoReconnect->Enabled=false;
				TimerFSUIPCProcess->Enabled=true;

	}

	else {

				StatusBar1->SimpleText = "Failed to open link to FSUIPC: " + String(pszErrors[dwResult]) ;

	}






}

//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonReconnectClick(TObject *Sender)
{

 char  string[32], chMsg[128];
 FSUIPC_Close();

if (FSUIPC_Open(SIM_ANY, &dwResult)) {

			// Okay, we're linked, and already the FSUIPC_Open has had an initial
			// exchange with FSUIPC to get its version number and to differentiate
			// between FS's.

				sprintf(string, "%c.%c%c%c%c",
						'0' + (0x0f & (FSUIPC_Version >> 28)),
					'0' + (0x0f & (FSUIPC_Version >> 24)),
					'0' + (0x0f & (FSUIPC_Version >> 20)),
					'0' + (0x0f & (FSUIPC_Version >> 16)),
						(FSUIPC_Version & 0xffff) ? 'a' + (FSUIPC_Version & 0xff) - 1 : ' ');

				LabelFSUIPC->Caption = String("FSUIPC Ver: ") + string;
				LabelFSUIPC->Color = clLime;
				LabelFSVer->Caption = "FS Ver: " + String(pFS[FSUIPC_FS_Version - 1]);
				LabelFSVer->Color = clLime;

				StatusBar1->SimpleText = "Link established to FSUIPC";
				TimerFSUIPCProcess->Enabled=true;
				TimerFSUIPCAutoReconnect->Enabled=false;
	}

	else {

				StatusBar1->SimpleText = "Failed to open link to FSUIPC: " + String(pszErrors[dwResult]) ;

	}

}
//---------------------------------------------------------------------------

void __fastcall TFormMain::Exit1Click(TObject *Sender)
{
Application->Terminate();
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------
void __fastcall TFormMain::TimerDisableStalledServoTimer(TObject *Sender)
{
int StopState;



}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormDestroy(TObject *Sender)
{
StopAllTimers();
FSUIPC_Close();

ArduinoPort->Close();




}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonManualCalibrateClick(TObject *Sender)
{
 int StopState1;
 int StopState2;

}
//---------------------------------------------------------------------------

void TFormMain::UpdateFSUIPCReturnedOffsetValuesLabels()
{
  //update labels

  //READ ENGINE POSITIONS
  FSUIPC_Read(0x88C, 2, &engineVariables.Engine1CurrentValue, &dwResult);
  FSUIPC_Read(0x88C, 2, &engineVariables.Engine2CurrentValue, &dwResult);

  //READ TRIM
  FSUIPC_Read(0x0BC0, 2, &trimVariables.TrimCurrentValue, &dwResult);

  // spoiler

   FSUIPC_Read(0x0BD4, 4, &spoilerVariables.SpoilerCurrentValue, &dwResult);

   //flaps

   FSUIPC_Read(0x0BDC, 4, &tqVariables.FlapLastFSValue, &dwResult);

	LabelEngine1->Caption = "ENG1: " + String(engineVariables.Engine1CurrentValue);
	LabelEngine2->Caption = "ENG2: " + String(engineVariables.Engine2CurrentValue);
	LabelSpoiler->Caption = "Spoiler: " + String(spoilerVariables.SpoilerCurrentValue);
	LabelFlap->Caption = "Flap: "+ String(tqVariables.FlapLastFSValue);
	LabelTrim->Caption = "Trim: " +	String(trimVariables.TrimCurrentValue);

	}

//-----------------------------------------------------------------
void TFormMain::UpdateThrottlePosition()
{

	int DC_MOTOR_COUNT=2;
	int SensorInputNumber=0;
	int SensorMinPos=0;
	int SensorMaxPos=0;
	int sensorValue=0;
	int SensorTolerance = 5;    // specify the value sensor must change before dc motors are moved. helps with tq hunting back and forth

	int TQ1TargetPercentage=0;
	int TQ2TargetPercentage=0;
	int Sensor1CurrentPercentage=0;
	int Sensor2CurrentPercentage=0;
	int DcMotor1PositionReached=0;
	int DcMotor2PositionReached=0;
	int TQ1offset = 0;
	int TQ2offset = 0;
	int CommandedThrottleOffset=0; //this fsuipc offset is used to determine whether autothrottle should be powered



	float SensorPercentage = 0;
	float ServoPos = 0;

	int ATPoweredStatus = 0;  //AutoThrottle Powered Status 0=off 1=On (ie power the servos)

	//if commanded throtttle is greater than 0 then DC motors are powered
	int CommandedThrottle=0;

	int ATswitchArmed = 0;

	int AirSpeedHold=0;
	int ToGa=0;
	int N1Hold=0;

	TIniFile *ini;

	ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));

	SensorTolerance = ini->ReadInteger("SETTINGS", "SENSORMOTORTOLERANCE", 5 );



   if (!dwResult)
	{


	TQ1offset=StrToInt("0x" + ini->ReadString("MOTOR_0","OFFSET","088C"));
	TQ2offset=StrToInt("0x" + ini->ReadString("MOTOR_1","OFFSET","088C"));
	CommandedThrottleOffset=StrToInt("0x" + ini->ReadString("SETTINGS","COMMANDEDTHROTTLEREF","0"));

  //READ ENGINE POSITIONS
  FSUIPC_Read(TQ1offset, 2, &engineVariables.Engine1CurrentValue, &dwResult);
  FSUIPC_Read(TQ2offset, 2, &engineVariables.Engine2CurrentValue, &dwResult);


  FSUIPC_Read(CommandedThrottleOffset, 1, &CommandedThrottle, &dwResult);

  }

   if (!FSUIPC_Process(&dwResult))
		  return;

 //---------------------------------------------------------------------------------

 // check autopilot status



	if(CommandedThrottle > 0)
	{
				ATPoweredStatus=1;
				LabelAT->Caption="A/T ON";
	}



		//IF a/t is not on, lets disable the servo
		if(ATPoweredStatus==0)
			{
			 LabelAT->Caption="A/T Disconnected";
			SendArduinoData("m0s");
			SendArduinoData("m1s");
			LabelMotor1Status->Caption = "Lever 1: Idle";
			LabelMotor2Status->Caption = "Lever 2: Idle";


			 }









	//update Engine Variables with current values
	engineVariables.Engine1LastValue=engineVariables.Engine1CurrentValue;
	engineVariables.Engine2LastValue=engineVariables.Engine2CurrentValue;

	TQ1TargetPercentage = (engineVariables.Engine1CurrentValue / 16384.0) * 1000.0;

	//Ensure Servo Percentage is does not go below 0
	if(TQ1TargetPercentage < 0)
		TQ1TargetPercentage=0;

	if(TQ1TargetPercentage > 1000)
		TQ1TargetPercentage=1000;


	//position sensor 2 target pos
	TQ2TargetPercentage = (engineVariables.Engine2CurrentValue / 16384.0) * 1000.0;

	//Ensure Servo Percentage is does not go below 0

	if(TQ2TargetPercentage < 0)
		TQ2TargetPercentage=0;

	if(TQ2TargetPercentage > 1000)
		TQ2TargetPercentage=1000;




	//actual power dc mototr



	if(ATPoweredStatus==1)
		{

	// lets check current position of the pot relative to percentage
	//Calculate Sensor % in relation to the Min and Max Pos

	//DC MOTOR 1 AND SENSOR 1
			SensorMinPos=ini->ReadInteger("SENSOR_A0","MINPOS",0);
			SensorMaxPos=ini->ReadInteger("SENSOR_A0","MAXPOS",0);
			sensorValue = GetAnalogInputSensorStatus(0);



			if(sensorValue>SensorMaxPos)
				sensorValue=SensorMaxPos;
			if(sensorValue<SensorMinPos)
				sensorValue=SensorMinPos;

			Sensor1CurrentPercentage = ((sensorValue-SensorMinPos) / float(SensorMaxPos-SensorMinPos)  * 1000);



	//COMPARE TQ TARGET TO POSITION OF CURRENT SENSOR

	if(	Sensor1CurrentPercentage==TQ1TargetPercentage)
		DcMotor1PositionReached=1;
	else
		{
		DcMotor1PositionReached=0;
//		SensorTarget1LastPercentage=Sensor1TargetPercentage;
		}



	
  
	//DC MOTOR 2 AND SENSOR 2
			SensorMinPos=ini->ReadInteger("SENSOR_A1","MINPOS",0);
			SensorMaxPos=ini->ReadInteger("SENSOR_A1","MAXPOS",0);
			sensorValue = GetAnalogInputSensorStatus(1);


			if(sensorValue>SensorMaxPos)
				sensorValue=SensorMaxPos;
			if(sensorValue<SensorMinPos)
				sensorValue=SensorMinPos;



			Sensor2CurrentPercentage = ((sensorValue-SensorMinPos) / float(SensorMaxPos-SensorMinPos)  * 1000);


//MOVE DC MOTOR 1

		Label1TQTarget->Caption = "TQ1 Target: " + String(TQ1TargetPercentage);
		Label1TQCurrent->Caption = "TQ1 Current: " + String(Sensor1CurrentPercentage);


//		if (Sensor1CurrentPercentage==TQ1TargetPercentage || Sensor1CurrentPercentage==TQ1TargetPercentage-1 || Sensor1CurrentPercentage==TQ1TargetPercentage+1)
		if(InRange(Sensor1CurrentPercentage,TQ1TargetPercentage-SensorTolerance,TQ1TargetPercentage+SensorTolerance))
		{
		SendArduinoData("m0s");
		DcMotor1PositionReached=1;
		LabelMotor1Status->Caption = "Lever 1: Idle";
		}
		else if ((Sensor1CurrentPercentage<TQ1TargetPercentage)&&DcMotor1PositionReached==0)
		{
		SendArduinoData("m0f:"+ ini->ReadString("MOTOR_0","SPEEDHIGH","0"));
		LabelMotor1Status->Caption = "Lever 1: FWD";
		}
		else if ((Sensor1CurrentPercentage>TQ1TargetPercentage)&&DcMotor1PositionReached==0)
		{
		SendArduinoData("m0b:"+ini->ReadString("MOTOR_0","SPEEDHIGH","0"));
		LabelMotor1Status->Caption = "Lever 1: REV";
		}

//MOVE DC MOTOR 2

		Label2TQTarget->Caption = "TQ2 Target: " + String(TQ2TargetPercentage);
		Label2TQCurrent->Caption = "TQ2 Current: " + String(Sensor2CurrentPercentage);

//		if (Sensor2CurrentPercentage==TQ2TargetPercentage || Sensor2CurrentPercentage==TQ2TargetPercentage-1 || Sensor2CurrentPercentage==TQ2TargetPercentage+1)
		if(InRange(Sensor2CurrentPercentage,TQ2TargetPercentage-SensorTolerance,TQ2TargetPercentage+SensorTolerance))
		{
		SendArduinoData("m1s");
		DcMotor2PositionReached=1;
		LabelMotor2Status->Caption = "Lever 2: Idle";
		}
		else if ((Sensor2CurrentPercentage<TQ2TargetPercentage)&&DcMotor2PositionReached==0)
		{
		SendArduinoData("m1f:"+ ini->ReadString("MOTOR_1","SPEEDHIGH","0"));
		LabelMotor2Status->Caption = "Lever 2: FWD";
		}
		else if ((Sensor2CurrentPercentage>TQ2TargetPercentage)&&DcMotor2PositionReached==0)
		{
		SendArduinoData("m1b:"+ini->ReadString("MOTOR_1","SPEEDHIGH","0"));
		LabelMotor2Status->Caption = "Lever 2: REV";
		}
	}




Update();
Application->ProcessMessages();
  delete ini;
}

//-------------------------------------------------
void __fastcall TFormMain::ButtonSpoilerCalibrateClick(TObject *Sender)
{

}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonSensorCalibrationClick(TObject *Sender)
{


TFormAnalogSensorCalibration * FormAnalogSensorCalibration1 = new TFormAnalogSensorCalibration(this);

StopAllTimers();

FormAnalogSensorCalibration1->ShowModal();

//upload min max values to arduino

ArduinoUpdateMinMaxSensorSettings();

StartAllTimers();

delete FormAnalogSensorCalibration1;

}
//---------------------------------------------------------------------------
void TFormMain::UpdateSpoilerServoPosition()
{


  //with the spoiler we will actually command the position of the fsuipc
  //based on ranges

	int ServoPercentage = 0;
	int ServoPos = 0;
	short ENG1 = 0;
	int PlaneOnGround=0;
	String SpoilerMinPos = "";
	String SpoilerMaxPos = "";



	int SpoilerArmed = 0;

	TIniFile *ini;

	ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));
	SpoilerMinPos=ini->ReadString("SERVO_1","MINPOS","0");
	SpoilerMaxPos=ini->ReadString("SERVO_1","MAXPOS","0");


   if (!dwResult)
	{
	//READ BOTH
	// FSUIPC_Read(0x0BD0, 4, &spoilerVariables.SpoilerCurrentValue, &dwResult);
	 //READ LEFT SPOILER
	 FSUIPC_Read(0x0BD4, 4, &spoilerVariables.SpoilerCurrentValue, &dwResult);
	 FSUIPC_Read(0x0BCC, 4, &SpoilerArmed, &dwResult);
	 FSUIPC_Read(0x088C, 2, &ENG1, &dwResult);
	 FSUIPC_Read(0x0366, 2, &PlaneOnGround, &dwResult);


	}

   if (!FSUIPC_Process(&dwResult))
		  return;







//first check if plane has just landed and we are powering throttles
//if so retract the flaps
	if(spoilerVariables.SpoilerCurrentValue>16383 &&PlaneOnGround==1 && ENG1 > 200)
		{
		spoilerVariables.Status = 0;
		LabelServo1->Caption = "Servo 1: " + SpoilerMinPos;
		SendArduinoData("SA1");
		SendArduinoData("S1:"+SpoilerMinPos);

		TimerDisableServo1->Enabled=true;


		}


//OTHERWISE LETS DEPLOY SPOILER

// HAS THE PLA

	if(SpoilerArmed==1 && spoilerVariables.SpoilerCurrentValue>16383 && ENG1<180)
		{
		LabelServo1->Caption = "Servo 1: " + SpoilerMaxPos;
		SendArduinoData("SA1");
		SendArduinoData("S1:"+SpoilerMaxPos);
		TimerDisableServo1->Enabled=true;;
		spoilerVariables.Status = 4;
		}





}

//-------------------------------------------------

void TFormMain::UpdateAnalogSensors()
{

//updates throttle, spoiler and flap values to fsuipc

unsigned long offset = 0;
unsigned long offset2 = 0;

int SensorMinPos=0;
int SensorMaxPos=0;

short int MinFSValue=0;
short int MaxFSValue=0;

int size = 0;
short int fsValue=0;
int sensorValue=0;
int SensorPercentage=0;
String SenorValueTemp = "";

if (!dwResult&&LabelArduinoStatus->Caption=="Status: Connected")
	{
	TIniFile *ini;

	ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));


	for(int i=0; i < 6; i++)
		{

		offset=StrToInt("0x" + ini->ReadString("SENSOR_A"+String(i),"OFFSET","0"));
		offset2=StrToInt("0x" + ini->ReadString("SENSOR_A"+String(i),"OFFSET2","0"));

		//find out whether sensor has something assigned to it
		if (offset==0)
			continue;

		size=ini->ReadInteger("SENSOR_A"+String(i),"SIZE",2);
		SensorMinPos=ini->ReadInteger("SENSOR_A"+String(i),"MINPOS",0);
		SensorMaxPos=ini->ReadInteger("SENSOR_A"+String(i),"MAXPOS",0);

		MinFSValue=ini->ReadInteger("SENSOR_A"+String(i),"MINFSVALUE",0);
		MaxFSValue=ini->ReadInteger("SENSOR_A"+String(i),"MAXFSVALUE",0);



		//ensure sensor limits arent exceeded
	   //check to see that limits are not exceeded

		  if((sensorValue = GetAnalogInputSensorStatus(i)) == -1)
			continue;

		   if(sensorValue>SensorMaxPos)
				sensorValue=SensorMaxPos;
		   if(sensorValue<SensorMinPos)
				sensorValue=SensorMinPos;


			 
		if(ini->ReadString("SENSOR_A"+String(i),"TYPE","")=="FLAPS")
			{
				if(tqVariables.FlapLastSensorValue!=sensorValue)
				{
					tqVariables.FlapLastSensorValue=sensorValue;

					if(ini->ReadInteger("SENSOR_A"+String(i),"INVERTED",0)==1)
						sensorValue=999-sensorValue;

					WriteFlapSettings("SENSOR_A"+String(i),sensorValue);
				}
				continue;
			}
		if(ini->ReadString("SENSOR_A"+String(i),"TYPE","")=="SPOILER")
			{
				if(tqVariables.SpoilerLastSensorValue!=sensorValue)
				{
					tqVariables.SpoilerLastSensorValue=sensorValue;
					WriteSpoilerSettings("SENSOR_A"+String(i),sensorValue,ini->ReadInteger("SENSOR_A"+String(i),"INVERTED",0));
				}
				continue;
			}


		//Calculate Sensor % in relation to the Min and Max Pos

		if(ini->ReadInteger("SENSOR_A"+String(i),"INVERTED",0)==0)
			SensorPercentage= ((sensorValue-SensorMinPos) / float(SensorMaxPos-SensorMinPos)  * 100);

		else
			SensorPercentage= ((SensorMaxPos - sensorValue) / float(SensorMaxPos-SensorMinPos)  * 100);


		//calculator fsvalue as a percentage of sensor value
		//need to calculate values diffently when minFSvalue is in the negative
		if(MinFSValue>-1)
			fsValue = ((MaxFSValue+MinFSValue)/100.0) * SensorPercentage;
		else
			{
			fsValue = ((MaxFSValue-(MinFSValue))/100.0) * SensorPercentage;
			fsValue =  (MaxFSValue-fsValue);
			}
		//if A/T is on and offset is for throttle, set offset to null to
		//disable the throttle input
		if(LabelAT->Caption=="A/T ON" && (offset==0x089A || offset==0x0932))
			offset=NULL;

		//write to offset 1
		if(offset!=0)
			FSUIPC_Write(offset, size, &fsValue, &dwResult);
		//write to offset 2 for ganged configs
		if(offset2!=0)
			FSUIPC_Write(offset2, size, &fsValue, &dwResult);

		if(dwResult!=0)
			MessageDlg("Error with offset: " +String(fsValue),mtError, TMsgDlgButtons()<<mbOK,0)  ;


		}
	delete ini;

	}
}

//------------------------------

void TFormMain::UpdateDigitalInputStatus()
{

//special code for Trim switch has been written in this section.
//if the offset is 0BC0

unsigned long offset = 0;
unsigned long offset2 = 0;
int buttonState=0;
int fsLength=0;
int fsValue=1;
int fsOffValue=0;

TIniFile *ini;

ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));


// we need to start from digital input 8 ie I=8
for(int i=4; i < 9; i++)
		{

		offset=StrToInt("0x" + ini->ReadString("DIGITAL_INPUT_"+String(i),"OFFSET","0"));
		offset2=StrToInt("0x" + ini->ReadString("DIGITAL_INPUT_"+String(i),"OFFSET2","0"));
		fsValue=ini->ReadInteger("DIGITAL_INPUT_"+String(i),"ONVALUE",0);
		fsLength=ini->ReadInteger("DIGITAL_INPUT_"+String(i),"LENGTH",0);
		fsOffValue=ini->ReadInteger("DIGITAL_INPUT_"+String(i),"OFFVALUE",-1);

		  if((buttonState = GetDigitalInputSensorStatus(i)) == -1)
			continue;

	   /////hacked trim code
		if (offset==0x0BC00 && buttonState==1)
			{
			trimVariables.TrimCurrentValue=trimVariables.TrimCurrentValue+200;
			if(trimVariables.TrimCurrentValue>16384)
					trimVariables.TrimCurrentValue=16384;
			FSUIPC_Write(0x0BC0, 2, &trimVariables.TrimCurrentValue, &dwResult);

			}
		else if (offset==0x0BC01 && buttonState==1)
				{
				trimVariables.TrimCurrentValue=trimVariables.TrimCurrentValue-200;
				if(trimVariables.TrimCurrentValue<-16384)
				   trimVariables.TrimCurrentValue=-16384;
				FSUIPC_Write(0x0BC0, 2, &trimVariables.TrimCurrentValue, &dwResult);
				 }

		////////////////////////end of hacked trim code

		else if (offset!=0 && buttonState==1)
			 {
			 FSUIPC_Write(offset, fsLength, &fsValue, &dwResult);
			 if (offset2!=0)
				 FSUIPC_Write(offset2, fsLength, &fsValue, &dwResult);

			 }
		else if (offset!=0 && buttonState==0 && fsOffValue != -1)
			 {
			 FSUIPC_Write(offset, fsLength, &fsOffValue, &dwResult);
			 if (offset2!=0)
				 FSUIPC_Write(offset2, fsLength, &fsValue, &dwResult);
			 }
		}




delete ini;


}

//---------
// ONLY used for parking break light so it is hardcoded

void TFormMain::UpdateDigitalOutputStatus()
{


int fsValue=0;

FSUIPC_Read(0x0BC8, 2, &fsValue, &dwResult);
   if (!FSUIPC_Process(&dwResult))
		  return;

/*if (fsValue==32767)
 CPhidgetInterfaceKit_setOutputState  (ifKit,0,PTRUE);
else
 CPhidgetInterfaceKit_setOutputState  (ifKit,0,PFALSE);*/
}

//--------------TRIM WHEEL OBC0

void TFormMain::UpdateTrimWheel()
{

   //get flap values to determine speed of the stab trim wheel

int velValue = 0;
double curVelValue=0;
int flapValue=0;



	FSUIPC_Read(0x0BDC, 4, &flapValue, &dwResult);
	FSUIPC_Read(0x0BC0, 2, &trimVariables.TrimCurrentValue, &dwResult);

	  if (!FSUIPC_Process(&dwResult))
			  return;


	if((trimVariables.TrimCurrentValue == trimVariables.TrimLastValue) && (TimerTrimWheelSuppressRotation->Enabled==false))
		SendArduinoData("m2s");
	if(trimVariables.TrimCurrentValue > trimVariables.TrimLastValue)
		{
		trimVariables.TrimLastValue=trimVariables.TrimCurrentValue;
		SendArduinoData("m2b:100");

		}
	if(trimVariables.TrimCurrentValue < trimVariables.TrimLastValue)
		{
		trimVariables.TrimLastValue=trimVariables.TrimCurrentValue;
		SendArduinoData("m2f:100");

		}

	//turn on the interval timer if the wheel is spining

//		if(curVelValue!=0)
		TimerTrimWheelRunTime->Enabled = true;


}
//--------------------
void __fastcall TFormMain::TimerTrimWheelRunTimeTimer(TObject *Sender)
{
//When we get to this timer... the wheel has been spining for at least 2 second.
//now we will stop the wheel from running for either 500ms or 3 seconds depending
//on the trimWheelWaitIntervalCount


if(trimWheelWaitIntervalCount==0 || trimWheelWaitIntervalCount==1)
{
	trimWheelWaitIntervalCount++;
	TimerTrimWheelSuppressRotation->Interval = 500;
}
else if(trimWheelWaitIntervalCount==2)
{
	trimWheelWaitIntervalCount=0;
	TimerTrimWheelSuppressRotation->Interval = 1000;
}

TimerTrimWheelRunTime->Enabled=false;
TimerTrimWheelSuppressRotation->Enabled=true;

}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TimerTrimWheelSuppressRotationTimer(TObject *Sender)
{
TimerTrimWheelSuppressRotation->Enabled=false;
}

//---------------------------------------------------------------------------

void  WriteFlapSettings(String SensorInput,int SensorValue)
{
	TIniFile *ini;

	int flaplow, flaphigh;
	int fsValue=0;
	int FoundFlapRange = 0;
	ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));

	//remember we need to cycle through 10 notches

	for(int i=0;i<9 && FoundFlapRange !=1 ;i++)
	{
	flaplow = ini->ReadInteger(SensorInput,"FLAP"+String(i)+"MIN",0);
	flaphigh = ini->ReadInteger(SensorInput,"FLAP"+String(i)+"MAX",100);

	if(SensorValue >= flaplow && SensorValue <= flaphigh)
		{
		 if(i==0)
			fsValue = 0;
		 else if(i==8)
			fsValue = 16384;

			fsValue = ((2048 * i));

		}

	}


	FSUIPC_Write(0x0BDC, 4, &fsValue, &dwResult);
	FSUIPC_Read(0x0BDC, 4, &tqVariables.FlapLastFSValue, &dwResult);
	FSUIPC_Process(&dwResult);


	delete ini;
}

//----------------------------------------------------------------------

void  WriteSpoilerSettings(String SensorInput,int SensorValue, int Inverted)
{
	TIniFile *ini;
	int SensorPercentage = 0;
	int SensorMinPos=0;
	int SensorMaxPos=0;
	int RangeValue=0;
	int armlow, armhigh;
	int fsValue=0;
	int SpoilerArmed = 0;
	int MinFSValue, MaxFSValue;


	ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));

	//remember we need to cycle through 4 notches

	armlow = ini->ReadInteger(SensorInput,"ARMMIN",0);
	armhigh = ini->ReadInteger(SensorInput,"ARMMAX",100);
	MinFSValue=ini->ReadInteger(SensorInput,"MINFSVALUE",0);
	MaxFSValue=ini->ReadInteger(SensorInput,"MAXFSVALUE",0);
	SensorMinPos=ini->ReadInteger(SensorInput,"MINPOS",0);
	SensorMaxPos=ini->ReadInteger(SensorInput,"MAXPOS",0);

	if(Inverted==1)
		RangeValue = 999-SensorValue;
	else
		RangeValue = SensorValue;

	if(RangeValue < armlow)
	{
		fsValue = 0;
		spoilerVariables.Status = 0;
	}

	if(RangeValue >= armlow && RangeValue <= armhigh)
	 {
	fsValue = 4800;
	 spoilerVariables.Status = 1; //arm spoiler
	 }
	if(RangeValue > armhigh)
	{
		MinFSValue=5620; //configure for values between 5620 -16384
	   if(Inverted==0)
		   {
			SensorMinPos=armhigh+1;
			SensorPercentage= ((SensorValue-SensorMinPos) / float(SensorMaxPos-SensorMinPos)  * 100.0);
		   }
		else
		   {
			SensorMaxPos=999-armhigh-1;
			SensorPercentage= ((SensorMaxPos - SensorValue) / float(SensorMaxPos-SensorMinPos)  * 100.0);
		   }
		fsValue = ((MaxFSValue-MinFSValue)/100.0) * SensorPercentage + 5620;
   //		fsValue = fsValue + ((MaxFSValue+MinFSValue)/7);
	   spoilerVariables.Status = 3;
	}



//lets read to see if we are armed.. we need to do this because the sensors may
//fluctiate constantly forcing a rearm.. When we send the arm bit to FS, FS will
//disarm the spoiler if it is already armed

//read current armed value

	FSUIPC_Read(0x0BCC, 4, &SpoilerArmed, &dwResult);
	FSUIPC_Process(&dwResult);

	if(spoilerVariables.Status==1 && SpoilerArmed!=1)
	{
	SpoilerArmed=1;
	FSUIPC_Write(0x0BCC, 4, &SpoilerArmed, &dwResult);
	}
	if(spoilerVariables.Status!=1)
	{
	SpoilerArmed=0;
	FSUIPC_Write(0x0BCC, 4, &SpoilerArmed, &dwResult);
	}

	//display commanded value in UI

  //	FormMain->LabelCommandedSpoiler->Caption = "Spoiler:" + fsValue;
	FSUIPC_Write(0x0BD0, 4, &fsValue, &dwResult);
	FSUIPC_Read(0x0BD0, 4, &spoilerVariables.SpoilerLastValue, &dwResult);
	FSUIPC_Process(&dwResult);


	delete ini;
}


void __fastcall TFormMain::TimerFSUIPCAutoReconnectTimer(TObject *Sender)
{
 char  string[32], chMsg[128];
 FSUIPC_Close();
if (FSUIPC_Open(SIM_ANY, &dwResult)) {

			// Okay, we're linked, and already the FSUIPC_Open has had an initial
			// exchange with FSUIPC to get its version number and to differentiate
			// between FS's.

				sprintf(string, "%c.%c%c%c%c",
						'0' + (0x0f & (FSUIPC_Version >> 28)),
					'0' + (0x0f & (FSUIPC_Version >> 24)),
					'0' + (0x0f & (FSUIPC_Version >> 20)),
					'0' + (0x0f & (FSUIPC_Version >> 16)),
						(FSUIPC_Version & 0xffff) ? 'a' + (FSUIPC_Version & 0xff) - 1 : ' ');

				LabelFSUIPC->Caption = String("FSUIPC Ver: ") + string;
				LabelFSUIPC->Color = clLime;
				LabelFSVer->Caption = "FS Ver: " + String(pFS[FSUIPC_FS_Version - 1]);
				LabelFSVer->Color = clLime;

				StatusBar1->SimpleText = "Link established to FSUIPC";
				TimerFSUIPCAutoReconnect->Enabled=false;
				TimerFSUIPCProcess->Enabled=true;

	}

	else {

				StatusBar1->SimpleText = "Failed to open link to FSUIPC: " + String(pszErrors[dwResult]) ;

	}


}
//---------------------------------------------------------------------------

void __fastcall TFormMain::About1Click(TObject *Sender)
{

TFormAbout * FormAbout1 = new TFormAbout(this);

FormAbout1->ShowModal();
delete FormAbout1;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonMotorCalibrateClick(TObject *Sender)
{

 TIniFile *ini;

ini = new TIniFile(AnsiString(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));


TFormMotorCalibration * FormMotorCalibration1 = new TFormMotorCalibration(this);


FormMotorCalibration1->CSpinEditMotor1High->Value = ini->ReadInteger("MOTOR_0", "SPEEDHIGH", 100);
FormMotorCalibration1->CSpinEditMotor1Low->Value = ini->ReadInteger("MOTOR_0", "SPEEDLOW", 100);

FormMotorCalibration1->CSpinEditMotor2High->Value = ini->ReadInteger("MOTOR_1", "SPEEDHIGH", 100);
FormMotorCalibration1->CSpinEditMotor2Low->Value = ini->ReadInteger("MOTOR_1", "SPEEDLOW", 100);

FormMotorCalibration1->CSpinEditMotor3High->Value = ini->ReadInteger("MOTOR_2", "SPEEDHIGH", 100);
FormMotorCalibration1->CSpinEditMotor3Low->Value = ini->ReadInteger("MOTOR_2", "SPEEDLOW", 100);








//assign change events

FormRollCalibration1->TrackBarRollMin->OnChange = FormRollCalibration1->TrackBarRollMinChange;
FormRollCalibration1->TrackBarRollMax->OnChange = FormRollCalibration1->TrackBarRollMaxChange;
FormRollCalibration1->TrackBarRollPos->OnChange = FormRollCalibration1->TrackBarRollPosChange;
FormRollCalibration1->TrackBarRollPos->OnChange = FormRollCalibration1->TrackBarRollPosChange;



  */
StopAllTimers();
FormMotorCalibration1->ShowModal();



StartAllTimers();

delete ini;
delete FormMotorCalibration1;
}
//---------------------------------------------------------------------------

void TFormMain::InitializeArduino()
{
			TIniFile * ini;
			String defaultComPort = "";
			Screen->Cursor = crHourGlass;

			try
			{

				ArduinoPort->Close();

			}
			catch (EAccessViolation &AccessViolation)
			 { }

			Memo1->Clear();
			Memo1->Lines->Add("Detecting Arduino...");

			Application->ProcessMessages();


			//LETS ENUMARE THE AVAILABLE COMPORTS

			TStringList * ports = new TStringList;

			EnumComPorts(ports);

			if (ports->Count == 0)
			{
				Beep();
				MessageDlg("There are no COM ports available on this system.",mtError, TMsgDlgButtons()<<mbOK,0)  ;
				Application->Terminate();
			}

			// SET PORT DEFAULT SETTINGS
			ArduinoPort->BaudRate = br115200;
			ArduinoPort->FlowControl->ControlDTR=dtrEnable;
			ArduinoPort->DataBits = dbEight;
			ArduinoPort->Parity->Bits = prNone;
			ArduinoPort->StopBits = sbOneStopBit;

		   //	ArduinoPort.NewLine = "\n";



		//load com port from inifile first

		//LOAD DEFAULT COM PORT SETTINGS AND USE IT IF DEFINED

			ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));


			//if com port isnt defined in the ini file lets try autodetect

			if((defaultComPort = ini->ReadString("SETTINGS", "COMPORT", "")) == "")
				defaultComPort = AutoDetectArduinoPortMethod();



		if (defaultComPort!="" && defaultComPort != NULL)
			{
				try
				{
					LabelComPort->Caption = "COM Port: " + defaultComPort;
					ArduinoPort->Port = defaultComPort;
					ArduinoPort->Open();
				}
				catch (Exception &ex)
				{
					Beep();
					MessageDlg("Could not open COM port.",mtError, TMsgDlgButtons()<<mbOK,0) ;
				   Memo1->Clear();
				   Memo1->Lines->Append("Failed to connect to Arduino.");
				Application->Terminate();


				}
			}
			else
				{
					Beep();
					MessageDlg("COM Port AutoDetect failed to fine a valid COM port.",mtError, TMsgDlgButtons()<<mbOK,0) ;
					Application->Terminate();

				}


			if (ArduinoPort->Connected == true)
			{
				LabelArduinoStatus->Caption = "Status: Connected";
				String FWVersion = "";
				Memo1->Clear();
				Memo1->Lines->Add("Arduino Firmware: ");
				if((FWVersion = GetArduinoData("atxdcversion")) == "")
					{
					MessageDlg("Could not open COM port.",mtError, TMsgDlgButtons()<<mbOK,0) ;
					ArduinoPort->Close();
					Application->Terminate();
				}

				Memo1->Lines->Append(FWVersion);

				if (!FWVersion.Pos("ATXDC"))
				{

					MessageDlg("Your Arduino does not contain the DVA2FSX firmware.\nYou must upload the correct firmware to use this program.",mtError, TMsgDlgButtons()<<mbOK,0) ;
					ArduinoPort->Close();
				}
				else
				{

					FWVersion = FWVersion.TrimRight() ;      //remove the extra \r \n characters
					LabelFWVersion->Caption = FWVersion.Delete(1,6);


				  if ( LabelFWVersion->Caption.ToDouble() < PROGRAMVERSION)
					{
					MessageDlg("Your Arduino firmware is out of date!\nYou must upload the correct firmware to use this program.",mtError, TMsgDlgButtons()<<mbOK,0) ;

					}


				   //enable recieving arduino events and start scanning inputs



				}




						Screen->Cursor = crArrow;



//Set the maximum and minimum pot values on the arduino card
					ArduinoUpdateMinMaxSensorSettings();


						delete ini;
}
}

//---------------------------------------------------------------------------

String TFormMain::AutoDetectArduinoPortMethod()
		{
			TStringList * ports = new TStringList;
			int count = 0 ;
			EnumComPorts(ports);


			String inString = "";

			while (count < ports->Count )
			{
				ArduinoPort->Port = ports->Strings[count];
				try
				{

					ArduinoPort->Close();
					ArduinoPort->Open();

					if (ArduinoPort->Connected == true)
					{

						inString = GetArduinoData("atxdcversion");

						ArduinoPort->Close();

						if(inString.Pos("ATXDC")>0)
							{

							return ports->Strings[count];
							}
					}

				}
				catch (Exception &ex)
				{

				}
				ArduinoPort->Close();
				count ++;
			}

			return "";
		}

//---------------------------------------------------------------------------

String TFormMain::ArduinoPortReadLine ()
{

clock_t init, final;

int bufferSize=0;
int TimerCount=0;
int TimerMax = 1;
bool eol = false;
char tempChar[1] = "";
String SerialString = "";

if(ArduinoPort->Connected == false)
  return "";

  //reset timer and bool value
  //wait for buffer to fill with something


	init=clock();

	while (ArduinoPort->InputCount() == 0 && TimerCount < TimerMax )
	TimerCount = (double)(clock()-init) / ((double)CLOCKS_PER_SEC);

	if(TimerCount > TimerMax-1)
	   {
		Memo2->Lines->Add("Timed Out Filling Buffer\n");
		return "";
	   }

   //if we didnt timeout we should have stuff in the buffer.


	init=clock();
	TimerCount = 0;

	//lets read the buffer up to the newline

	while(eol!=true && TimerCount < TimerMax)
	{


	 //only read if there is something in the buffer otherwise we skip the cycle and wait
	 if(ArduinoPort->InputCount() > 0)
	   {
		 ArduinoPort->Read(tempChar,1);
		  if(strcmp(tempChar,"\n")==0)
		   {
		   //get rid of left over characters
		   SerialString = SerialString.TrimRight();
		   eol=true;
		   }
		else
			SerialString += tempChar;
	  }
	  /* else
		  Memo2->Lines->Add("Buffer ran out looping\n");*/
   //	TimerCount++;
	TimerCount = (double)(clock()-init) / ((double)CLOCKS_PER_SEC);

	}

	//reset timer

	//are we hear because we time out or did we not reach end of line?
	if(eol==false  || TimerCount > TimerMax-1)
	   {

		TimerCount=0;
		Memo2->Lines->Add("Timed Out Retrieving EOL\n");
		return "";
	   }

	//otherwise return the full string
	return SerialString;

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonReconnectArduinoClick(TObject *Sender)
{
ArduinoPort->Close();
StopAllTimers();
LabelArduinoStatus->Caption = "Status: Reconnecting";
InitializeArduino();
StartAllTimers();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonDigitalCalibrationClick(TObject *Sender)
{
Beep();
MessageDlg("Not implented as yet", mtError, TMsgDlgButtons() << mbOK, 0);
}
//---------------------------------------------------------------------------

void TFormMain::SendArduinoData(String payload)
		{

			try
			{

				if (ArduinoPort->Connected == true)
					ArduinoPort->WriteStr(payload+"\n");


			}
			catch (Exception &ex )
			{}

		}
//---------------------------------------------------------------------------
void __fastcall TFormMain::TimerScanSensorInputsTimer(TObject *Sender)
{
	TimerScanSensorInputs->Enabled = false; //disable until we finish this process


	LabelInput0->Caption =  GetAnalogInputSensorStatus(0);
	LabelInput1->Caption =  GetAnalogInputSensorStatus(1);
	LabelInput2->Caption =  GetAnalogInputSensorStatus(2);
	LabelInput3->Caption =  GetAnalogInputSensorStatus(3);
	LabelInput4->Caption =  GetAnalogInputSensorStatus(4);
	LabelInput5->Caption =  GetAnalogInputSensorStatus(5);
	LabelDigitalInput8->Caption = GetDigitalInputSensorStatus(4);
	LabelDigitalInput9->Caption = GetDigitalInputSensorStatus(5);
	LabelDigitalInput10->Caption = GetDigitalInputSensorStatus(6);
	LabelDigitalInput11->Caption = GetDigitalInputSensorStatus(7);
	LabelDigitalInput12->Caption = GetDigitalInputSensorStatus(8);
	LabelDigitalInput13->Caption = GetDigitalInputSensorStatus(13);

	UpdateThrottlePosition();    // moved this function here to ensure minimal latency when updating throttle inputs

	//Application->ProcessMessages();

	TimerScanSensorInputs->Enabled = true;

}
//---------------------------------------------------------------------------
int  TFormMain::GetAnalogInputSensorStatus(int Sensor)
{

			  String inString = "";
			  int SensorValue = 0;
			  int pos =0;
			  String TempString = "";

			  TIniFile *ini;

			  ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));


			  hysteresis = ini->ReadInteger("SETTINGS", "HYSTERESIS", 10 );

			  delete ini;


			  if((inString = GetArduinoData("A"+String(Sensor)))=="")
				return -1;

			  pos = inString.Pos("A" + String(Sensor)+":");

			   if (pos == 1)
			   {
				   TempString = inString.Delete(1,3);
				   SensorValue = TempString.ToInt();


				   if (SensorValue > ArduinoAnalogSensorValue[Sensor] + hysteresis || SensorValue < ArduinoAnalogSensorValue[Sensor] - hysteresis)
						   ArduinoAnalogSensorValue[Sensor] = SensorValue;

			   }

  return ArduinoAnalogSensorValue[Sensor];

}


//-------------------------------------------------

int  TFormMain::GetDigitalInputSensorStatus(int Sensor)
{

			  String inString = "";
			  int SensorValue = 0;
			  int pos =0;
			  String TempString = "";

/*			  if(ArduinoPort->Connected == false)
				  return -1;
			  try
			  {
			  ArduinoPort->ClearBuffer(true,false);
			  } catch (Exception &ex) {}

			  //Application->ProcessMessages();
			  SendArduinoData("A"+String(Sensor));




			  if((inString = ArduinoPortReadLine())=="")
				return -1;*/

			  if((inString = GetArduinoData("D"+String(Sensor)))=="")
				return -1;

			  pos = inString.Pos("D" + String(Sensor)+":");

			   if (pos == 1)
			   {
				   TempString = inString.Delete(1,inString.Length()-1);
				   SensorValue = TempString.ToInt() ^ 1;


			   }

  return SensorValue;

}
//---Sends data and returns payload


String  TFormMain::GetArduinoData(String payload)
{

			  String inString = "";

			 if(ArduinoPort->Connected == false)
				  return -1;
			  try
			  {
			  ArduinoPort->ClearBuffer(true,false);
			  } catch (Exception &ex) {}

			  //Application->ProcessMessages();
			  SendArduinoData(payload);

			  if((inString = ArduinoPortReadLine())=="")
				return "";

  return inString;

}
//---------------------------------------------------------------------------
void TFormMain::StartAllTimers()
{

	if(ArduinoPort->Connected == true)
		TimerScanSensorInputs->Enabled = true;

	TimerFSUIPCProcess->Enabled = true;
}
//-------------------------------

void TFormMain::StopAllTimers()
{

	TimerScanSensorInputs->Enabled = false;
  try
	  {
		  ArduinoPort->ClearBuffer(true,false);
	  } catch (Exception &ex) {}

	TimerFSUIPCProcess->Enabled = false;

	Application->ProcessMessages();
}
//--------------------------------------

void __fastcall TFormMain::ExitExecute(TObject *Sender)
{
//exit(0);
Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormClose(TObject *Sender, TCloseAction &Action)
{
Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ActionAboutExecute(TObject *Sender)
{
TFormAbout * FormAbout1 = new TFormAbout(this);

FormAbout1->ShowModal();
delete FormAbout1;
}
//---------------------------------------------------------------------------
void TFormMain::ArduinoUpdateMinMaxSensorSettings()
{
			TIniFile * ini;
				ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));

				SendArduinoData("m0minvalue:"+ini->ReadString("SENSOR_A0","MINPOS","0"));
				SendArduinoData("m0maxvalue:"+ini->ReadString("SENSOR_A0","MAXPOS","0"));
				SendArduinoData("m1minvalue:"+ini->ReadString("SENSOR_A1","MINPOS","0"));
				SendArduinoData("m1maxvalue:"+ini->ReadString("SENSOR_A1","MAXPOS","0"));
				SendArduinoData("showvar");
				Memo2->Lines->Add(ArduinoPortReadLine());

				delete ini;
}
//-------------------------------------------

void __fastcall TFormMain::ButtonSpoilerCalibrationClick(TObject *Sender)
{

TFormSpoilerCalibration * FormSpoilerCalibration1 = new TFormSpoilerCalibration(this);
TIniFile * ini;
ini = new TIniFile(String(ExtractFilePath(Application->ExeName)+ "atxdc.ini"));





StopAllTimers();



FormSpoilerCalibration1->EditSpoilerMinPos->Text = ini->ReadString("SERVO_1","MINPOS","0");
FormSpoilerCalibration1->EditSpoilerMaxPos->Text = ini->ReadString("SERVO_1","MAXPOS","0");

SendArduinoData("SA1");
FormSpoilerCalibration1->ShowModal();
SendArduinoData("SD1");


ini->WriteString("SERVO_1","MINPOS",FormSpoilerCalibration1->EditSpoilerMinPos->Text);
ini->WriteString("SERVO_1","MAXPOS",FormSpoilerCalibration1->EditSpoilerMaxPos->Text);

StartAllTimers();

delete ini;

delete FormSpoilerCalibration1;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TimerDisableServo1Timer(TObject *Sender)
{
SendArduinoData("SD1");
TimerDisableServo1->Enabled = false;
}
//---------------------------------------------------------------------------

