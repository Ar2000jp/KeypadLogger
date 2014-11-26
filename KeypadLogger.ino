/**
 * Copyright (C) 2014 Ahmad Draidi <ar2000jp@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Keypad.h>
#include <SD.h>
#include <DS1307.h>
#include <I2Cdev.h>
#include <Wire.h>

const byte C_LedPin = 13;

const byte C_SDCSPin = 53;

const char C_TimeFilename[] = "time.txt";
const char C_LogFilename[] = "log.txt";
File G_LogFile;

DS1307 G_RTC;

// Keypad pins
const byte C_KeypadCol1Pin = A0;
const byte C_KeypadCol2Pin = A1;
const byte C_KeypadCol3Pin = A2;
const byte C_KeypadCol4Pin = A3;

const byte C_KeypadRow1Pin = A4;
const byte C_KeypadRow2Pin = A5;
const byte C_KeypadRow3Pin = A6;
const byte C_KeypadRow4Pin = A7;

// Keypad size
const byte C_KeypadRows = 4;
const byte C_KeypadCols = 4;

const char C_Keys[C_KeypadRows][C_KeypadCols] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}
};

const byte C_RowPins[C_KeypadRows] = {
    C_KeypadRow1Pin, C_KeypadRow2Pin, C_KeypadRow3Pin, C_KeypadRow4Pin
};

const byte C_ColPins[C_KeypadCols] = {
    C_KeypadCol1Pin, C_KeypadCol2Pin, C_KeypadCol3Pin, C_KeypadCol4Pin
};

Keypad G_Keypad = Keypad(makeKeymap(C_Keys), (byte*)C_RowPins,
                         (byte*)C_ColPins, C_KeypadRows, C_KeypadCols);

void errorHalt() {
    Serial.println("Fatal error occurred. Halting.");

    while(1) {
        digitalWrite(C_LedPin, HIGH);
        delay(1000);
        digitalWrite(C_LedPin, LOW);
        delay(1000);
    }
}

void rtcWorkaround() {
    byte seconds;
    for (int i = 0; i < 4; i++) {
        seconds = G_RTC.getSeconds();
        delay(300);
        if (seconds != G_RTC.getSeconds()) {
            // Clock is running
            return;
        }
        delay(400);
        if (seconds != G_RTC.getSeconds()) {
            // Clock is running
            return;
        }
        delay(400);
        if (seconds != G_RTC.getSeconds()) {
            // Clock is running
            return;
        }

        // Toggle clock
        G_RTC.setClockRunning(false);
        delay(10);
        G_RTC.setClockRunning(true);
        delay(10);
    }

    Serial.println("Unable to start RTC clock.");
    errorHalt();
}

void setup() {
    Serial.begin(115200);

    // Busy/Error LED
    pinMode(C_LedPin, OUTPUT);

    Serial.print("Initializing SD card... ");
    pinMode(C_SDCSPin, OUTPUT);
    if (!SD.begin()) {
        Serial.println("Card failed, or not present.");
        errorHalt();
    }
    Serial.println("Card initialized.");

    Serial.println("Initializing RTC... ");
    Wire.begin();
    G_RTC.initialize();
    if (G_RTC.testConnection() == false) {
        Serial.println("DS1307 connection failed.");
        errorHalt();
    }
    G_RTC.setSquareWaveEnabled(0);

    if (G_RTC.getClockRunning() != true) {
        G_RTC.setClockRunning(true);
    }

    // Switch to 24 Hour mode
    if (G_RTC.getMode() != 0) {
        G_RTC.setMode(0);
    }

    // Hardware bug workaround.
    rtcWorkaround();

    // Open time file
    File timeFile = SD.open(C_TimeFilename, FILE_READ);
    if(timeFile) {
        Serial.println("Setting RTC time from time file.");

        // Time string format: 2014 12 30 23 59 59
        unsigned int temp;
        temp = timeFile.parseInt();
        G_RTC.setYear(temp);
        temp = timeFile.parseInt();
        G_RTC.setMonth(temp);
        temp = timeFile.parseInt();
        G_RTC.setDay(temp);
        temp = timeFile.parseInt();
        G_RTC.setHours24(temp);
        temp = timeFile.parseInt();
        G_RTC.setMinutes(temp);
        temp = timeFile.parseInt();
        G_RTC.setSeconds(temp);

        timeFile.close();
        SD.remove((char*)C_TimeFilename);
    }

    // Open log file
    G_LogFile = SD.open(C_LogFilename, FILE_WRITE);
    if(!G_LogFile) {
        Serial.println("Error opening log file.");
        errorHalt();
    }

    String dateTimeString;
    dateTimeString += G_RTC.getYear();
    dateTimeString += "/";
    dateTimeString += G_RTC.getMonth();
    dateTimeString += "/";
    dateTimeString += G_RTC.getDay();
    dateTimeString += " ";
    byte h = G_RTC.getHours24();
    if (h < 10) {
        dateTimeString += "0";
    }
    dateTimeString += h;
    dateTimeString += ":";
    byte m = G_RTC.getMinutes();
    if (m < 10) {
        dateTimeString += "0";
    }
    dateTimeString += m;
    dateTimeString += ":";
    byte s = G_RTC.getSeconds();
    if (s < 10) {
        dateTimeString += "0";
    }
    dateTimeString += s;
    G_LogFile.print("Log start date/time: ");
    G_LogFile.println(dateTimeString);
    Serial.print("Log start date/time: ");
    Serial.println(dateTimeString);
}

void loop() {
    String dataString;
    char key;
    unsigned long startTime = millis();

    while(1) {
        digitalWrite(C_LedPin, LOW);
        key = G_Keypad.waitForKey();
        digitalWrite(C_LedPin, HIGH);

        dataString = "Key: ";
        dataString += key;
        dataString += " relTime: ";
        dataString += String(millis() - startTime);

        Serial.println(dataString);
        G_LogFile.println(dataString);
        G_LogFile.flush();
    }
}
