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

const byte C_LedPin = 13;

const byte C_SDCSPin = 53;

const char C_LogFilename[] = "log.txt";

File G_LogFile;

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

void setup() {
    Serial.begin(115200);

    pinMode(C_LedPin, OUTPUT);

    Serial.print("Initializing SD card... ");
    pinMode(C_SDCSPin, OUTPUT);
    if (!SD.begin()) {
        Serial.println("Card failed, or not present.");
        errorHalt();
    }
    Serial.println("Card initialized.");

    G_LogFile = SD.open(C_LogFilename, FILE_WRITE);
    if(!G_LogFile) {
        Serial.println("Error opening log file.");
        errorHalt();
    }
}

void loop() {
    String dataString = "";
    char key = 0;
    unsigned long time = 0;

    while(1) {
        key = 0;
        while(key == 0) {
            key = G_Keypad.getKey();
        }

        time = millis();

        dataString = "Key: ";
        dataString += key;
        dataString += " Time: ";
        dataString += String(time);

        Serial.println(dataString);
        G_LogFile.println(dataString);
        G_LogFile.flush();
    }
}
