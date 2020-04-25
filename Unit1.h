//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include "phidget21.h"
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.ActnCtrls.hpp>
#include <Vcl.ActnMan.hpp>
#include <Vcl.ActnMenus.hpp>
#include <Vcl.ToolWin.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.PlatformDefaultStyleActnCtrls.hpp>
#include <Vcl.StdActns.hpp>
#include <Vcl.XPStyleActnCtrls.hpp>
#include <Vcl.StdStyleActnCtrls.hpp>

#include "CPort.hpp"


//---------------------------------------------------------------------------
class TFormMain : public TForm
{
__published:	// IDE-managed Components
	TButton *ButtonReconnect;
	TStatusBar *StatusBar1;
	TTimer *TimerFSUIPCProcess;
	TGroupBox *GroupBox1;
	TPanel *Panel1;
	TGroupBox *GroupBox8;
	TLabel *LabelInput0;
	TLabel *LabelInput1;
	TLabel *LabelInput2;
	TButton *ButtonSensorCalibration;
	TLabel *LabelInput3;
	TLabel *LabelInput4;
	TLabel *LabelInput5;
	TTimer *TimerTrimWheelRunTime;
	TTimer *TimerTrimWheelSuppressRotation;
	TTimer *TimerFSUIPCAutoReconnect;
	TImage *Image1;
	TLabel *Label16;
	TGroupBox *GroupBox3;
	TLabel *LabelFSUIPC;
	TLabel *LabelFSTime;
	TLabel *LabelFSVer;
	TButton *Button1;
	TGroupBox *GroupBox4;
	TLabel *LabelComPort;
	TLabel *Label23;
	TLabel *LabelArduinoStatus;
	TButton *ButtonReconnectArduino;
	TMemo *Memo1;
	TComPort *ArduinoPort;
	TLabel *LabelFWVersion;
	TMemo *Memo2;
	TGroupBox *GroupBox5;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TGroupBox *GroupBox2;
	TLabel *LabelAT;
	TLabel *LabelEngine1;
	TLabel *LabelEngine2;
	TLabel *LabelFlap;
	TLabel *LabelSpoiler;
	TLabel *LabelTrim;
	TGroupBox *GroupBox6;
	TLabel *LabelMotor1Status;
	TLabel *LabelMotor2Status;
	TLabel *Label15;
	TTimer *TimerScanSensorInputs;
	TLabel *Label7;
	TLabel *Label20;
	TLabel *Label21;
	TLabel *Label22;
	TLabel *Label24;
	TLabel *Label25;
	TActionMainMenuBar *ActionMainMenuBar1;
	TActionManager *ActionManager1;
	TButton *Button2;
	TAction *ActionExit;
	TAction *ActionAbout;
	TLabel *Label1TQCurrent;
	TLabel *Label1TQTarget;
	TLabel *Label2TQCurrent;
	TLabel *Label2TQTarget;
	TLabel *LabelDigitalInput8;
	TLabel *LabelDigitalInput9;
	TLabel *LabelDigitalInput10;
	TLabel *LabelDigitalInput11;
	TLabel *LabelDigitalInput12;
	TLabel *LabelDigitalInput13;
	TGroupBox *GroupBox7;
	TLabel *LabelServo1;
	TLabel *Label9;
	TButton *ButtonSpoilerCalibration;
	TTimer *TimerDisableServo1;
	void __fastcall TimerFSUIPCProcessTimer(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall ButtonReconnectClick(TObject *Sender);
	void __fastcall Exit1Click(TObject *Sender);
	void __fastcall TimerDisableStalledServoTimer(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall ButtonManualCalibrateClick(TObject *Sender);
	void __fastcall ButtonSpoilerCalibrateClick(TObject *Sender);
	void __fastcall ButtonSensorCalibrationClick(TObject *Sender);
	void __fastcall TimerTrimWheelRunTimeTimer(TObject *Sender);
	void __fastcall TimerTrimWheelSuppressRotationTimer(TObject *Sender);
	void __fastcall TimerFSUIPCAutoReconnectTimer(TObject *Sender);
	void __fastcall About1Click(TObject *Sender);
	void __fastcall ButtonMotorCalibrateClick(TObject *Sender);
	void __fastcall ButtonReconnectArduinoClick(TObject *Sender);
	void __fastcall ButtonDigitalCalibrationClick(TObject *Sender);
	void __fastcall TimerScanSensorInputsTimer(TObject *Sender);
	void __fastcall ExitExecute(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall ActionAboutExecute(TObject *Sender);
	void __fastcall ButtonSpoilerCalibrationClick(TObject *Sender);
	void __fastcall TimerDisableServo1Timer(TObject *Sender);


private:	// User declarations
public:		// User declarations






	__fastcall TFormMain(TComponent* Owner);

	void InitializeApp();
	void InitializeArduino();
	void UpdateFSUIPCReturnedOffsetValuesLabels();
	void UpdateThrottlePosition();
	void SendArduinoData(String payload);
	int  GetAnalogInputSensorStatus(int Sensor);
	int  GetDigitalInputSensorStatus(int Sensor);
	String AutoDetectArduinoPortMethod();
	void StartAllTimers();
	void StopAllTimers();
	String ArduinoPortReadLine ();
	String GetArduinoData(String payload);
	void UpdateSpoilerServoPosition();
	void UpdateRollServoPosition();
	void UpdateAnalogSensors();
	void UpdateDigitalInputStatus();
	void UpdateDigitalOutputStatus();
	void UpdateTrimWheel();
	void ArduinoUpdateMinMaxSensorSettings();
	};
//-----------------------------
class C{
int x() { return _x; }
void x(int x_) { _x = x_; }
private:
int _x;
};
//--------------------------------------------------------------------------
/*class  CengineVariables
{

private:
int FCurrentEngineValue1;
int FCurrentEngineValue2;



public:

int   CurrentEngineValue1() const {return FCurrentEngineValue1; }
void  CurrentEngineValue1(int value) {FCurrentEngineValue1=value;}


};*/

struct ENGINE_VARIABLES
{
 short Engine1CurrentValue;
 short Engine2CurrentValue;
 short Engine1LastValue;
 short Engine2LastValue;

};

struct TQ_VARIABLES      //variables assoicated with the physical inputs
{
 int Engine1CurrentValue;
 int Engine2CurrentValue;
 int ParkBrakeLight;
 int ParkBrakeSwitch;
 int FlapLastSensorValue;
 int SpoilerLastSensorValue;
 int FlapLastFSValue;
 int TrimLastValue;

};

struct SPOILER_VARIABLES
{
 int SpoilerCurrentValue;
 int SpoilerLastValue;
 int Status; //0=down, 1=armed, 2 = transition , 3= up/deployed, 4=autodeployed

};

struct TRIM_VARIABLES
{
short int TrimCurrentValue;
short int TrimLastValue;

};
struct SERVO_SETTINGS
{

int MinPos;
int MaxPos;
int Inverted;

};


struct BITS16
{

BYTE bit0: 1;
BYTE bit1: 1;
BYTE bit2: 1;
BYTE bit3: 1;
BYTE bit4: 1;
BYTE bit5: 1;
BYTE bit6: 1;
BYTE bit7: 1;
BYTE bit8: 1;
BYTE bit9: 1;
BYTE bit10: 1;
BYTE bit11: 1;
BYTE bit12: 1;
BYTE bit13: 1;
BYTE bit14: 1;
BYTE bit15: 1;

};

struct BITS8
{

BYTE bit0: 1;
BYTE bit1: 1;
BYTE bit2: 1;
BYTE bit3: 1;
BYTE bit4: 1;
BYTE bit5: 1;
BYTE bit6: 1;
BYTE bit7: 1;

};

struct DVATX_SETTINGS
{
	int ATBITETEST; //A/T Built In Test Equipment.
	int TQVelocity;
	int TQAcceleration;
	int MCVelocity;
	int MCAcceleration;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMain *FormMain;
extern ENGINE_VARIABLES engineVariables;
extern TQ_VARIABLES tqVariables;
extern SERVO_SETTINGS servoSettings[8];
extern DVATX_SETTINGS dvatxSettings;
extern SPOILER_VARIABLES spoilerVariables;
extern TRIM_VARIABLES trimVariables;
//---------------------------------------------------------------------------

void  WriteFlapSettings(String SensorInput,int SensorValue);
void WriteSpoilerSettings(String SensorInput,int SensorValue,int inverted);
/*public: static AnsiString GlobalVar()
{
get { return m_globalVar; }
set { m_globalVar = value; }
}


} */

#endif
