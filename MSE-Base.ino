/*

 MSE 2202 MSEBot base code for Labs 3 and 4
 Language: Arduino
 Authors: Eugen Porter and Michael Naish
 
 Rev 1 - Initial version 2016
 Rev 2 - Update for MSEduino V0.2
 ...
 Rev 4 - revisit for MSEDuino V4.2 2023

 */

//  To program and use ESP32-S3
//   
//  File->Preferences:
//  Additional Boards Manager URLs: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//
//
//  Tools->:
//  Board: "Adafruit Feather ESP32-S3 No PSRAM"
//  Upload Speed: "921600"
//  USB CDC On Boot: "Enabled"
//  USB Firmware MSC on Boot: "Disabled"
//  USB DFU On Bot: "Disabled"
//  Upload Mode:"UART0/Hardware CDC"
//  SPU Frequency: "240MHz (WiFi)"
//  Flash Mode: "QIO 80MHz"
//  Flash SIze: "4MB (32Mb)"
//  Partition Scheme: "Default 4MB with spiffs (1.2MB app/1.5MB SPIFFS)"
//  Core Debug Level: "Verbose"
//  PSRAM: 'Disabled"
//  Arduino Runs On: "Core 1"
//  Events Run On: "Core 1"
//
//  To program, press and hold the reset button then press and hold program button, release the reset button then 
//  release the program button 
//

#include <Adafruit_NeoPixel.h>
#include <MSE2202_Lib.h>

// Uncomment keywords to enable debugging output
#define DEBUG_DRIVE_SPEED 1
#define DEBUG_ENCODER_COUNT 1

// Port pin constants

#define MODE_BUTTON         0   // GPIO0  pin 27 for Push Button 1
                                   
#define LEFT_MOTOR_A        35  // GPIO35 pin 28 (J35) Motor 1 A
#define LEFT_MOTOR_B        36  // GPIO36 pin 29 (J36) Motor 1 B
#define RIGHT_MOTOR_A       37  // GPIO37 pin 30 (J37) Motor 2 A
#define RIGHT_MOTOR_B       38  // GPIO38 pin 31 (J38) Motor 2 B
                                   
#define SHOULDER_SERVO      41  // GPIO41 pin 34 (J41) Servo 1
#define CLAW_SERVO          42  // GPIO42 pin 35 (J42) Servo 2
                                   
#define ENCODER_LEFT_A      15  // When DIP Switch S1-1 is on, Left encoder A signal is connected to pin 8 GPIO15 (J15)
                                // When DIP Switch S1-1 is off, J15 can be used as analog AD2-4
#define ENCODER_LEFT_B      16  // When DIP Switch S1-2 is on, Left encoder B signal is connected to pin 9 GPIO16 (J16)
                                // When DIP Switch S1-2 is off, J16 can be used as analog AD2-5
#define ENCODER_LEFT_DIR    17  // When DIP Switch S1-3 is on, Left encoder Direction signal is connected to pin 10 GPIO17 (J17)
                                // When DIP Switch S1-3 is off, J17 can be used as analog AD2-6
#define ENCODER_LEFT_SPD    18  // When DIP Switch S1-4 is on, Left encoder Speed signal is connected to pin 11 GPIO18 (J18)
                                // When DIP Switch S1-4 is off, J18 can be used as analog AD2-7
                                   
#define MOTOR_ENABLE_SWITCH 3   // DIP Switch S1-5 pulls Digital pin D3 to ground when on, connected to pin 15 GPIO3 (J3)
                                // When DIP Switch S1-5 is off, J3 can be used as analog AD1-2
                                   
#define ENCODER_RIGHT_A     11  // When DIP Switch S1-7 is on, Right encoder A signal is connected to pin 19 GPIO11 (J11)
                                // When DIP Switch S1-7 is off, J11 can be used as analog AD2-0
#define ENCODER_RIGHT_B     12  // When DIP Switch S1-8 is on, Right encoder B signal is connected to pin 20 GPIO12 (J12)
                                // When DIP Switch S1-8 is off, J12 can be used as analog AD2-1
#define ENCODER_RIGHT_DIR   13  // When DIP Switch S1-9 is on, Right encoder Direction signal is connected to pin 21 GPIO13 (J13)
                                // When DIP Switch S1-9 is off, J13 can be used as analog AD2-2
#define ENCODER_RIGHT_SPD   14  // When DIP Switch S1-10 is on, Right encoder Speed signal is connected to pin 22 GPIO14 (J14)
                                // When DIP Switch S1-10 is off, J14 can be used as analog AD2-3
                                   
#define BRDTST_POT_R1       1   // When DIP Switch S1-12 is on, Analog AD1-0 (pin 39) GPIO1 is connected to Poteniometer R1

#define STEPPER_DIR         39  // GPIO39 pin 32 (J39) STEPPER Motor direction pin
#define STEPPER_CLK         40  // GPIO40 pin 33 (J40) stepper motor clock pin

#define STEPPER2_DIR        41
#define STEPPER2_CLK        42
                                   
#define SMART_LED           21  // When DIP Switch S1-11 is on, Smart LED is connected to pin 23 GPIO21 (J21)
#define SMART_LED_COUNT     1   // Number of SMART LEDs in use

#define IR_DETECTOR         9   // GPIO9 pin 17 (J9) IR detector input

#define LEFT                1   // Indicates left direction for (stepper) motor
#define RIGHT               0   // Indicates right direction for (stepper) motor



// Constants

const int ci_Display_Update = 100;                                            // Update interval for Smart LED in milliseconds

const int trigPin = 13; // Pin for trigger port of ultrasonic sensor
const int echoPin = 14; // Pin for echo port of ultrasonic sensor

const int pwm = 2 ; //initializing pin 2 as pwm
const int in_1 = 41 ;
const int in_2 = 42 ;

//Variables

long duration; // duration of pulse for the ultrasonic sensor
int distance; // distance measured by the bultrasonic sensor


//=====================================================================================================================
//
// IMPORTANT: The constants in this section need to be set to appropriate values for your robot. 
// You will have to experiment to determine appropriate values.

const int ci_Claw_Servo_Open = 1650;                                          // Value for open position of claw
const int ci_Claw_Servo_Closed = 1880;                                        // Value for closed position of claw
const int ci_Shoulder_Servo_Retracted = 690;                                  // Value for shoulder of arm fully retracted
const int ci_Shoulder_Servo_Extended = 1300;                                  // Value for shoulder of arm fully extended

//
//=====================================================================================================================

// Variables

boolean bt_Motors_Enabled = true;                                             // Motors enabled flag
boolean bt_3_S_Time_Up = false;                                               // 3 second timer elapsed flag
boolean bt_2_S_Time_Up = false;                                               // 2 second timer elapsed flag
boolean bt_200_mS_Time_Up = false;                                            // 200 millisecond timer elapsed flag
boolean bt_Direction;                                                         // Stepper motor direction
boolean bt_OppDirection;
boolean bt_Stepper_Step;                                                      // Stepper motor step flag

unsigned char uc_Drive_Speed;                                                 // Motor drive speed (0-255)
unsigned char uc_Drive_Index;                                                 // State index for run mode (1)
unsigned char uc_Stepper_Index;                                               // State index for stepper test mode (2)

int i_MaxStepsFromCentre = 900;                                               // Allowable number of steps from centre point
int i_StepperCentre = i_MaxStepsFromCentre;                                   // Centre point of stepper range, in steps
int i_StepCounter = 0;                                                        // Number of steps to take to reach setpoint
int i_StepperPosition;                                                        // Current position of stepper, in steps
int numGaps = 0;                                                              // Number of gaps the bot detects
int start = 0;                                                                // Variable that tracks the current state of crossing the bot is in

unsigned int ui_CurrentPotValue;                                              // Current value of pot as raw ADC value
unsigned int ui_PreviousPotValue = 0;                                         // Previous value of pot as raw ADC value
unsigned int ui_DeadZone = 50;                                                // Allowable difference between current stepper set point and
                                                                              // value from pot. Accounts for noise in ADC readings.
unsigned int ui_PotArmSetpoint;                                               // Desired position of arm stepper motor read from pot
unsigned int ui_PotClawSetpoint;                                              // Desired position of claw servo read from pot
unsigned int ui_PotShoulderSetpoint;                                          // Desired position of shoulder servo read from pot
unsigned int ui_Mode_PB_Debounce;                                             // Pushbutton debounce timer count

unsigned long ul_3_Second_timer = 0;                                          // 3 second timer count in milliseconds
unsigned long ul_2_Second_timer = 0;                                          // 2 second timer count in milliseconds
unsigned long ul_200_mS_timer = 0;                                            // 200 millisecond timer count in milliseconds
unsigned long ul_Display_Time;                                                // Heartbeat LED update timer
unsigned long ul_Previous_Micros;                                             // Last microsecond count
unsigned long ul_Current_Micros;                                              // Current microsecond count

// Declare SK6812 SMART LED object
//   Argument 1 = Number of LEDs (pixels) in use
//   Argument 2 = ESP32 pin number 
//   Argument 3 = Pixel type flags, add together as needed:
//     NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//     NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//     NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//     NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//     NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel SmartLEDs(SMART_LED_COUNT, SMART_LED, NEO_RGB + NEO_KHZ800);

// Smart LED brightness for heartbeat
unsigned char LEDBrightnessIndex = 0; 
unsigned char LEDBrightnessLevels[] = {5,15,30,45,60,75,90,105,120,135,150,165,180,195,210,225,240,255,
                                       240,225,210,195,180,165,150,135,120,105,90,75,60,45,30,15};

unsigned int  ui_Robot_Mode_Index = 0;                                        // Robot operational state                              
unsigned int  ui_Mode_Indicator[7] = {                                        // Colours for different modes
  SmartLEDs.Color(255,0,0),                                                   //   Red - Stop
  SmartLEDs.Color(0,255,0),                                                   //   Green - Run
  SmartLEDs.Color(0,0,255),                                                   //   Blue - Test stepper
  SmartLEDs.Color(255,255,0),                                                 //   Yellow - Test claw servo
  SmartLEDs.Color(255,0,255),                                                 //   Magenta - Test shoulder servo
  SmartLEDs.Color(0,255,255),                                                 //   Cyan - Test IR receiver
  SmartLEDs.Color(255,165,0)                                                  //   Orange - empty case
};                                                                            

// Motor and encoder objects (classes defined in MSE2202_Lib)
Motion Bot = Motion();                                                        // Instance of Motion for motor control
Encoders driveEncoders = Encoders();                                          // Instance of Encoders for encoder data
IR Scan = IR();                                                               // Instance of IR for beacon detection

// Interrupt Service Routines
void IRAM_ATTR LeftSpd_EncoderISR()                                           // ISR to update left encoder count
{
   driveEncoders.LeftSpd_Encoder_ISR();
}

void IRAM_ATTR RightSpd_EncoderISR()                                          // ISR to update right encoder count
{
	driveEncoders.RightSpd_Encoder_ISR();
}

// Function Declarations
void Indicator();                                                             // For mode/heartbeat on Smart LED

void setup()
{
   Serial.begin(9600);

   Scan.Begin(IR_DETECTOR);                                                    //set up IR Detection


   // Set up motors and encoders
   Bot.driveBegin("D1", LEFT_MOTOR_A, LEFT_MOTOR_B, RIGHT_MOTOR_A, RIGHT_MOTOR_B); // Set up motors as Drive 1
   Bot.driveBegin("D2", 39, 40, 41, 42);
   Bot.servoBegin("S1",CLAW_SERVO);   
   Bot.servoBegin("S2",SHOULDER_SERVO);
   Bot.motorBegin("M1", 2, 1);

   driveEncoders.Begin(0, LeftSpd_EncoderISR, RightSpd_EncoderISR);           // Set up encoders

   // Set up SmartLED
   SmartLEDs.begin();                                                         // Initialize smart LEDs object (REQUIRED)
   SmartLEDs.clear();                                                         // Clear pixel
   SmartLEDs.setPixelColor(0,SmartLEDs.Color(0,0,0));                         // Set pixel colors to 'off'
   SmartLEDs.show();                                                          // Send the updated pixel colors to the hardware

   // Set up mode pushbutton
   pinMode(MOTOR_ENABLE_SWITCH, INPUT_PULLUP);                                // Set up motor enable switch with internal pullup
   ui_Mode_PB_Debounce = 0;                                                   // Reset debounce timer count
   
   //Stepper motor 
   pinMode(STEPPER_CLK, OUTPUT);                                              // Set up stepper step/clock pin as output
   pinMode(STEPPER_DIR, OUTPUT);                                              // Set up stepper direction pin as output
   pinMode(STEPPER2_CLK, OUTPUT);                                              // Set up stepper step/clock pin as output
   pinMode(STEPPER2_DIR, OUTPUT);

   pinMode(MODE_BUTTON, INPUT_PULLUP);                                        //set up mode button with internal pullup
}

void loop()
{
    
   ul_Current_Micros = micros();                                              // Get current time in microseconds
   if ((ul_Current_Micros - ul_Previous_Micros) >= 1000)                      // Enter when 1 ms has elapsed
   {
      ul_Previous_Micros = ul_Current_Micros;                                 // Record current time in microseconds

      // 3 second timer, counts 3000 milliseconds
      ul_3_Second_timer = ul_3_Second_timer + 1;                              // Increment 3 second timer count
      if(ul_3_Second_timer > 3000)                                            // If 3 seconds have elapsed
      {
         ul_3_Second_timer = 0;                                               // Reset 3 second timer count
         bt_3_S_Time_Up = true;                                               // Indicate that 3 seconds have elapsed
      }
   
      // 2 second timer, counts 2000 milliseconds
      ul_2_Second_timer = ul_2_Second_timer + 1;                              // Increment 2 second timer count
      if(ul_2_Second_timer > 2000)                                            // If 2 seconds have elapsed
      {
         ul_2_Second_timer = 0;                                               // Reset 2 second timer count
         bt_2_S_Time_Up = true;                                               // Indicate that 2 seconds have elapsed
      }
   
      // 200 millisecond timer, counts 200 milliseconds
      ul_200_mS_timer = ul_200_mS_timer + 1;                                  // Increment 200 millisecond timer count
      if(ul_200_mS_timer > 200)                                               // If 200 milliseconds have elapsed
      {
         ul_200_mS_timer = 0;                                                 // Reset 200 millisecond timer count
         bt_200_mS_Time_Up = true;                                            // Indicate that 200 milliseconds have elapsed
      }

      // Mode pushbutton debounce and toggle
      if(!digitalRead(MODE_BUTTON))                                           // If pushbutton GPIO goes LOW (nominal push)
      {
         // Start debounce
         if(ui_Mode_PB_Debounce <= 25)                                        // 25 millisecond debounce time
         {
            ui_Mode_PB_Debounce = ui_Mode_PB_Debounce + 1;                    // Increment debounce timer count
            if(ui_Mode_PB_Debounce > 25)                                      // If held for at least 25 mS
            {
               ui_Mode_PB_Debounce = 1000;                                    // Change debounce timer count to 1 second
            }
         }
         if(ui_Mode_PB_Debounce >= 1000)                                      // Maintain 1 second timer count until release
         {
            ui_Mode_PB_Debounce = 1000;
         }
      }
      else                                                                    // Pushbutton GPIO goes HIGH (nominal release)
      {
         if(ui_Mode_PB_Debounce <= 26)                                        // If release occurs within debounce interval
         {
            ui_Mode_PB_Debounce = 0;                                          // Reset debounce timer count
         }
         else
         {
            ui_Mode_PB_Debounce = ui_Mode_PB_Debounce + 1;                    // Increment debounce timer count
            if(ui_Mode_PB_Debounce >= 1025)                                   // If pushbutton was released for 25 mS
            {
               ui_Mode_PB_Debounce = 0;                                       // Reset debounce timer count
               ui_Robot_Mode_Index++;                                         // Switch to next mode
               ui_Robot_Mode_Index = ui_Robot_Mode_Index & 7;                 // Keep mode index between 0 and 7
               ul_3_Second_timer = 0;                                         // Reset 3 second timer count
               bt_3_S_Time_Up = false;                                        // Reset 3 second timer         
            }
         }
      }
  
      // check if drive motors should be powered
      bt_Motors_Enabled = !digitalRead(MOTOR_ENABLE_SWITCH);                  // If SW1-5 is on (low signal), then motors are enabled

      // modes 
      // 0 = Default after power up/reset. Robot is stopped.
      // 1 = Press mode button once to enter. Run robot.
      // 2 = Press mode button twice to enter. Test stepper motor. 
      // 3 = Press mode button three times to enter. Test claw servo. 
      // 4 = Press mode button four times to enter. Test arm shoulder servo.
      // 5 = Press mode button five times to enter. Test IR receiver. 
      // 6 = Press mode button six times to enter.  //add your code to do something 
      switch(ui_Robot_Mode_Index)
      {
         case 0: // Robot stopped
         {
            Bot.Stop("D1");                                                   // Stop Drive 1
            uc_Drive_Index = 0;                                               // Reset drive index
            uc_Stepper_Index = 0;                                             // Reset stepper index
            driveEncoders.clearEncoder();                                     // Clear encoder counts
            bt_2_S_Time_Up = false;                                           // Reset 2 second timer flag
            break;
         }  
      
         case 1: // Run robot
         {
            if(bt_3_S_Time_Up)                                                // Pause for 3 sec before running Case 1 code
            {
               // Read pot to update drive motor speed
               uc_Drive_Speed = map(analogRead(BRDTST_POT_R1), 0, 4096, 150, 255);
#ifdef DEBUG_DRIVE_SPEED 
               Serial.print(F("Drive Speed: Pot R1 = "));
               Serial.print(analogRead(BRDTST_POT_R1));
               Serial.print(F(", mapped = "));
               Serial.println(uc_Drive_Speed);
#endif
#ifdef DEBUG_ENCODER_COUNT
               driveEncoders.getEncoderRawCount();
               driveEncoders.getEncoderRawSpeed();
               Serial.print(F("Left Encoder count = "));
               Serial.print(driveEncoders.lRawEncoderLeftCount);
               Serial.print(F("   Right Encoder count = "));
               Serial.print(driveEncoders.lRawEncoderRightCount);
               Serial.print(F("   Left Encoder speed = "));
               Serial.print(driveEncoders.lRawEncoderLeftSpeed);
               Serial.print(F("   Right Encoder speed = "));
               Serial.println(driveEncoders.lRawEncoderRightSpeed);
#endif
               if(bt_Motors_Enabled)                                          // Run motors only if enabled
               {
                  if (numGaps == 0)
                  {
                    if (start == 0)
                    {
                      // Moves the large arm forward as to not hit the top bar on the first platform
                      Bot.Forward("D2", uc_Drive_Speed, uc_Drive_Speed);
                      delay(1000);
                      Bot.Stop("D2");
                      delay(15);
                      start++;
                    }

                    digitalWrite(trigPin, LOW);
                    delayMicroseconds(2);
                    // Sets the trigPin on HIGH state for 10 micro seconds
                    digitalWrite(trigPin, HIGH);
                    delayMicroseconds(10);
                    digitalWrite(trigPin, LOW);
                    // Reads the echoPin, returns the sound wave travel time in microseconds
                    duration = pulseIn(echoPin, HIGH);
                    // Calculating the distance
                    distance = duration * 0.034 / 2;

                    // check if bot senses a gap
                    if (distance >= 50)
                    {
                      Bot.Stop("D1");
                      delay(15);
                      numGaps++;
                    }
                    else
                    {
                      Bot.Forward("D1", uc_Drive_Speed, uc_Drive_Speed);
                      delay(15);
                    }
                  }

                  // activate once bot senses the first gap
                  else if(numGaps == 1)
                  {
                    distance = 0;
                    // stop moving forward
                    Bot.Stop("D1");
                    delay(15);
                    // position the outer arm to be straight up 
                    Bot.Reverse("D2", uc_Drive_Speed, uc_Drive_Speed);
                    delay(200);
                    Bot.Stop("D2");
                    delay(15);
                    // extend gear motor and inner arm
                    Bot.Forward("M1", uc_Drive_Speed, uc_Drive_Speed);
                    delay(1000);
                    Bot.Stop("M1");
                    delay(15);
                    // lower the entire arm onto the overhead bar
                    Bot.Forward("D2", uc_Drive_Speed, uc_Drive_Speed);
                    delay(200);
                    Bot.Stop("D2");
                    delay(15);
                    // retract gear motor and inner arm partially
                    Bot.Reverse("M1", uc_Drive_Speed, uc_Drive_Speed);
                    delay(500);
                    Bot.Stop("M1");
                    delay(15);
                    // rotate the entire arm to position the bot closer to the secon platform
                    Bot.Reverse("D2", uc_Drive_Speed, uc_Drive_Speed);
                    delay(200);
                    // begin spinning the wheel to catch onto the second platform
                    Bot.Forward("D1", uc_Drive_Speed, uc_Drive_Speed);
                    delay(15);
                    //fully retract gear motor and inner arm
                    Bot.Reverse("M1", uc_Drive_Speed, uc_Drive_Speed);
                    delay(500);
                    Bot.Stop("M1");
                    delay(15);
                    // rotate the outer arm to pull the bot onto the secon platform
                    Bot.Reverse("D2", uc_Drive_Speed / 2, uc_Drive_Speed / 2);
                    delay(800);
                    Bot.Stop("D2");
                    delay(15);

                    // once on the second platform wait until the gap at the end is detected
                    while (distance <= 50)
                    {
                      digitalWrite(trigPin, LOW);
                      delayMicroseconds(2);
                      // Sets the trigPin on HIGH state for 10 micro seconds
                      digitalWrite(trigPin, HIGH);
                      delayMicroseconds(10);
                      digitalWrite(trigPin, LOW);
                      // Reads the echoPin, returns the sound wave travel time in microseconds
                      duration = pulseIn(echoPin, HIGH);
                      // Calculating the distance
                      distance = duration * 0.034 / 2;
                    }
                    Bot.Stop("D1");
                    delay(500);
                    // reverse 50 cm once the end of the table is detected
                    Bot.Reverse("D1", uc_Drive_Speed, uc_Drive_Speed);
                    delay(2000);
                    numGaps++;
                  }
                  else
                  {
                    Bot.Stop("D1");
                    delay(15);
                  }
               }
               else                                                           // Stop when motors are disabled
               {  
                  Bot.Stop("D1");
                  delay(15);
               }
            }
            break;
         } 
      }

      // Update brightness of heartbeat display on SmartLED
      ul_Display_Time++;                                                      // Count milliseconds
      if(ul_Display_Time > ci_Display_Update)                                 // When display update period has passed
      {
         ul_Display_Time = 0;                                                 // Reset display counter
         LEDBrightnessIndex++;                                                // Shift to next brightness level
         if(LEDBrightnessIndex > sizeof(LEDBrightnessLevels))                 // If all defined levels have been used
         {
            LEDBrightnessIndex = 0;                                           // Reset to starting brightness
         }
         SmartLEDs.setBrightness(LEDBrightnessLevels[LEDBrightnessIndex]);    // Set brightness of heartbeat LED
         Indicator();                                                         // Update LED
      }
   }
}   

// Set colour of Smart LED depending on robot mode (and update brightness)
void Indicator()
{
  SmartLEDs.setPixelColor(0, ui_Mode_Indicator[ui_Robot_Mode_Index]);         // Set pixel colors to = mode 
  SmartLEDs.show();                                                           // Send the updated pixel colors to the hardware
}






