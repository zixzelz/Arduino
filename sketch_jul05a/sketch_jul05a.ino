#include <ThreeWire.h>  
#include <RtcDS1302.h>

const int kPowerLoadPin = 8;
const int kEnableVoltagePin = 4;
const int kPowerVoltagePin = A0;

ThreeWire myWire(6,7,5); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

const int kDelayStateTime = 10; // sec
const int kActiveStateTime = 5; // sec

const int kVoltageSensorMin = 0; // minimum sensor value
const int kVoltageSensorMax = 1023; // maximum sensor value
const int kVoltageMin = 0; // mV
const int kVoltageMax = 19000; // mV

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

    rfcDelay(kDelayStateTime);

    turnOn();
    Serial.println("on");

    float voltage = getVoltage(kEnableVoltagePin, kPowerVoltagePin);
    Serial.print("Voltage: ");
    Serial.println(voltage);

    rfcDelay(kActiveStateTime);

    turnOff();
    Serial.println("off");

    // logs
    RtcDateTime currentTimeStamp = Rtc.GetDateTime();
    printDateTime(currentTimeStamp);
    Serial.println();
}

void turnOn() {
    digitalWrite(kPowerLoadPin, HIGH);
}

void turnOff() {
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
