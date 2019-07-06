#include <ThreeWire.h>  
#include <RtcDS1302.h>

const int kPowerLoadPin = 8;
const int kEnableVoltagePin = A1;
const int kPowerVoltagePin = A0;

ThreeWire myWire(6,7,5); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

const int kDelayStateTime = 5; // sec
const int kMaxActiveStateTime = 120; // sec
const int kShutDowtTimeOut = 5; // sec

const int kVoltageSensorMin = 0; // minimum sensor value
const int kVoltageSensorMax = 1023; // maximum sensor value
const int kVoltageMin = 0; // mV
const int kVoltageMax = 19000; // mV

String give_power_level_command = "give_power_level\n";
String shut_dowt_command = "shut_dowt\n";

void setup() {
    Serial.begin(9600);

    setupRtcTime();

    pinMode(kEnableVoltagePin, OUTPUT);
    pinMode(kPowerLoadPin, OUTPUT);
}

void setupRtcTime() {

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid()) {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected()) {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }
}

void loop() {

    turnOn();
    Serial.println("on");

    float voltage = getVoltage(kEnableVoltagePin, kPowerVoltagePin);
    Serial.print("Voltage: ");
    Serial.println(String(voltage));

    handleActiveState();

    turnOff();
    Serial.println("off");

    // logs
    RtcDateTime currentTimeStamp = Rtc.GetDateTime();
    printDateTime(currentTimeStamp);
    Serial.println();

    rfcDelay(kDelayStateTime);
}

void handleActiveState() {
//    rfcDelay(kMaxActiveStateTime);

    RtcDateTime timeTimeStamp = Rtc.GetDateTime();
    RtcDateTime currentTimeStamp = Rtc.GetDateTime();
    bool completed = false;
    while (currentTimeStamp - timeTimeStamp < kMaxActiveStateTime && !completed) {
        completed = checkCommand();
      
        currentTimeStamp = Rtc.GetDateTime();
    }
}

bool checkCommand() {

    bool completed = false;

    if (Serial.available()) {
      String command = readLine();
      if (command.equals(give_power_level_command)) {
          float voltage = getVoltage(kEnableVoltagePin, kPowerVoltagePin);
          Serial.println(String(voltage));
      }
      else if (command.equals(shut_dowt_command)) {
          completed = true;
      }
    }
  
    return completed;
}

String readLine() {
  String readString = "";
  while (Serial.available()) {
    delay(3);  //delay to allow buffer to fill 
    if (Serial.available() > 0) {
      char c = Serial.read();  //gets one byte from serial buffer
      readString += c; //makes the string readString
    }
  }
  return readString;
}

void turnOn() {
    digitalWrite(kPowerLoadPin, HIGH);
}

void turnOff() {
    delay(kShutDowtTimeOut * 1000);
    digitalWrite(kPowerLoadPin, LOW);
}

float getVoltage(int keyPin, int sensorPin) {
    digitalWrite(keyPin, HIGH);
    float sensorValue = analogRead(sensorPin);
    digitalWrite(keyPin, LOW);
  
    Serial.print(sensorValue);
    Serial.print(" > ");

    float voltage = map(sensorValue, kVoltageSensorMin, kVoltageSensorMax, kVoltageMin, kVoltageMax) / 1000.0;
    return voltage;
}

void rfcDelay(int delayTime) {
    RtcDateTime timeTimeStamp = Rtc.GetDateTime();
    RtcDateTime currentTimeStamp = Rtc.GetDateTime();
    
    while (currentTimeStamp - timeTimeStamp < delayTime) {
        currentTimeStamp = Rtc.GetDateTime();
    }
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt) {
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}
