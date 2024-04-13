/**
 * Code pour le jeu "Simon says".
 *
 * 12 avril 2024.
 * 
 * Par:
 * Thomas Lagacé [https://github.com/ThomasLagace]
 * Schneider Pierre [https://github.com/Shnei92]
 * Logan Bessette [https://github.com/LoganBess]
 * Kouassi Fidele Tanoh [https://github.com/fideletanoh]
 */

/* Définition des constantes globales et des structures utilisées. */
#define SCL_PIN 26
#define SDI_PIN 25
#define CS_PIN 27

#define ALLOCATE_ARRAY_CHUNK_SIZE 16
const int LED_PINS[] = {33, 15, 32, 14};
const int BUTTON_PINS[] = {5, 18, 19, 16};
const int BUZZER_PIN = 13;

// Structure qui permet de stocker une séquence.
typedef struct {
  byte *arrayPointer;
  unsigned int allocatedSize; /// < Largeur en byte de la mémoire allouée pour la séquence.
  unsigned int length;        /// < Le nombre d'éléments dans la liste.
} SequenceList;

/* ------- */

void setup() {
  Serial.begin(9600);

  for(byte i = 0; i < 4; i++)
    pinMode(LED_PINS[i], OUTPUT);

  for(byte i = 0; i < 4; i++)
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  // Initialisation de l'écran LCD
  initLCD_SPI(SCL_PIN, SDI_PIN, CS_PIN);

  // writeString((unsigned char*)"Newhaven Display----");
  // setCursor(0x40);
  // writeString((unsigned char*)" - Character LCD");
  // setCursor(0x14);
  // writeString((unsigned char*)" - Serial LCD");
  // setCursor(0x54);
  // writeString((unsigned char*)"  -> I2C, SPI, RS232");
}
 
void loop() {
  // Creation d'une nouvelle séquence
  SequenceList sequence;
  sequence.arrayPointer = emptyArray(ALLOCATE_ARRAY_CHUNK_SIZE);
  sequence.allocatedSize = ALLOCATE_ARRAY_CHUNK_SIZE;
  sequence.length = 0;

  // Loop infinie pour le jeu
  // Une itération == un round
  for(;;) {
    addToSequence(random(1, sizeof(LED_PINS) / sizeof(LED_PINS[0]) + 1), &sequence);
    readSequence(&sequence);

    if (!usersTurnToPressButtons(&sequence)) {
      loseAnimation();
      break;
    }

    delay(500);
  }

  free(sequence.arrayPointer);
  sequence.arrayPointer = NULL;
}

/**
 * @brief Ajoute un chiffre à une séquence.
 * 
 * Ajoute un chiffre à une séquence,
 * si la séquence est trop petite, l'agrandi pour le prochain chiffre.
 *
 * @param numberToAdd Le chiffre à ajouter à la séquence.
 * @param sequence Un pointeur vers une séquence.
 * @return rien
 */
void addToSequence(byte numberToAdd, SequenceList *sequence) {
  if (sequence->length >= sequence->allocatedSize - 1) {
    int newSize = ALLOCATE_ARRAY_CHUNK_SIZE * (sequence->allocatedSize / ALLOCATE_ARRAY_CHUNK_SIZE + 1);
    sequence->arrayPointer = realocateArrayToNewSize(sequence->arrayPointer, sequence->allocatedSize, newSize);
    sequence->allocatedSize += ALLOCATE_ARRAY_CHUNK_SIZE;
  }

  sequence->arrayPointer[sequence->length] = numberToAdd;
  sequence->length++;
}

/**
 * @brief Lit une séquence et active la DEL associée.
 * 
 * @param sequence Pointeur vers une séquence.
 * @return Rien
 */
void readSequence(SequenceList *sequence) {
  for (int i = 0; i < sequence->length; i++) {
    byte currentInSequence = sequence->arrayPointer[i] - 1;

    digitalWrite(LED_PINS[currentInSequence], HIGH);
    tone(BUZZER_PIN, 1000);
    delay(500);

    digitalWrite(LED_PINS[currentInSequence], LOW);
    noTone(BUZZER_PIN);
    delay(500);
  }
}

/**
 * @brief Tour au joueur de faire une séquence dans l'ordre.
 * 
 * Itère à travers toute une séquence.
 * À chaque chiffre, attend l'input du joueur,
 * ou le joueur prend trop de temps à répondre et retourne faux.
 *
 * @param sequence Pointeur vers une séquence.
 * @return Vrai si les bouttons sont appuyés dans l'ordre, sinon faux.
 */
bool usersTurnToPressButtons(SequenceList *sequence) {
  for(int i = 0; i < sequence->length; i++) {
    unsigned long maxTime = 5000;

    int pressedButton = 0;

    pressedButton = waitForButtonPressOrMaxTime(maxTime);

    if (pressedButton == 0 || pressedButton != sequence->arrayPointer[i]) {
      return false;
    }

    digitalWrite(LED_PINS[pressedButton - 1], HIGH);
    tone(BUZZER_PIN, 1000);
    delay(500);

    digitalWrite(LED_PINS[pressedButton - 1], LOW);
    noTone(BUZZER_PIN);
  }

  writeString((unsigned char*)"GAGNER!");

  return true;
}

/**
 * @brief Attend l'input d'un boutton.
 *
 * @param maxTime Temps maximum pour répondre.
 * @return Le numéro du boutton ou "0" si le temps est expiré.
 */
int waitForButtonPressOrMaxTime(long maxTime) {
  unsigned long startTime = millis();
  unsigned long currentTime;

  for(;;) {
    for(int i = 0; i < sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]); i++) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) 
        return i + 1;
    }

    currentTime = millis();

    if (currentTime - startTime >= maxTime)
      return 0;
  }
}

void loseAnimation() {
  for(int i = 0; i < sizeof(LED_PINS) / sizeof(LED_PINS[0]); i++)
    digitalWrite(LED_PINS[i], HIGH);
  
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 200);
    delay(200);

    noTone(BUZZER_PIN);
    delay(200);
  }

  for(int i = 0; i < sizeof(LED_PINS) / sizeof(LED_PINS[0]); i++)
    digitalWrite(LED_PINS[i], LOW);
  
  delay(1000);
}

/* Manipulation d'arrays */

/**
 * @brief Copie une ancienne array dans une plus grande partie allouée.
 *
 * @param oldArray Pointeur vers l'ancienne array.
 * @param oldSize Ancienne largeur de l'ancienne array.
 * @param newSize Nouvelle largeur de l'array.
 * @return Pointeur vers la nouvelle array.
 */
byte* realocateArrayToNewSize(byte *oldArray, int oldSize, int newSize) {
  byte *newArray = emptyArray(newSize);
  memcpy(newArray, oldArray, oldSize);
  free(oldArray);
  return newArray;
}

/**
 * @brief Retourne un pointeur vers une array alloué vide.
 *
 * @param size Largeur de l'array en byte.
 * @return Pointeur vers l'array vide.
 */
byte* emptyArray(int size) {
  byte *array = (byte *)malloc(size * sizeof(byte));
  for(int i = 0; i < size; i++)
    array[i] = 0;
  
  return array;
}

/* -------- */

/***********************************************************
 * Serial_LCD.ino
 * This code was written to interface and Arduino UNO with NHD serial LCDs.
 * 
 * Program Loop:
 * 1. Write "Newhaven Display--" on line 1
 * 2. Write " - 4x20  Characters" on line 2
 * 3. Write " - Serial LCD"
 * 4. Write "  -> I2C, SPI, RS232"
 * 
 * (c)2022 Newhaven Display International, LLC.
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 ***********************************************************/

/**
 * I2C Wiring Reference:
 * 
 * - Arduino Pin 5 (SCL) to LCD J2 Pin 3 (SCL)
 * - Arduino Pin 4 (SDA) to LCS J2 Pin 4 (SDA)
 * - GND to LCD J2 Pin 5 (VSS)
 * - 5V to LCD J2 Pin 6 (VDD)
 */

/**
 * SPI Wiring Reference:
 * 
 * - Arduino Pin 5 (SCL) to LCD J2 Pin 3 (SCK)
 * - Arduino Pin 4 (SDI) to LCD J2 Pin 4 (SDI)
 * - Arduino Pin 3 (/CS) to LCD J2 Pin 1 (SPISS)
 * - GND to LCD J2 Pin 5 (VSS)
 * - 5V to LCD J2 Pin 6 (VDD)
 */

/**
 * RS232 Wiring Reference:
 * 
 * - Arduino Pin 2 (TX) to LCD J1 Pin 1 (RX)
 * - GND to LCD J1 Pin 2 (VSS)
 * - 5V to LCD J1 Pin 3 (VDD)
 */

/*

initLCD_SPI(22, 23, 21);

writeString((unsigned char*)"Newhaven Display----");
setCursor(0x40);
writeString((unsigned char*)" - Character LCD");
setCursor(0x14);
writeString((unsigned char*)" - Serial LCD");
setCursor(0x54);
writeString((unsigned char*)"  -> I2C, SPI, RS232");

*/
#include <stdint.h>
#include <stdlib.h>

#define STARTUP_DELAY 500

#define RS232_DELAY 100

#define I2C_DELAY 100
#define SLAVE_ADDRESS 0x28

// SPI Interface
uint8_t _SCL; // 5
uint8_t _SDI; // 4
uint8_t _CS; // 3

// RS232 Interface
uint8_t _TX; // 2

//I2C Interface
uint8_t _SDA; // 4

enum Interface{
  I2C,
  SPI,
  RS232
};

Interface _interface;

/**
 * @brief Initialize selected IO ports for I2C.
 * 
 * @param SCL Serial clock pin assigment.
 * @param SDA Serial data pin assignment.
 * @return none
 */
void initLCD_I2C(uint8_t SCL, uint8_t SDA)
{
  _interface = I2C;

  // Store pin assigmnents globally
  _SCL = SCL;
  _SDA = SDA;

  // Set IO modes
  pinMode(SCL, OUTPUT);
  pinMode(SDA, OUTPUT);
  
  // Set starting pin states
  digitalWrite(SCL, HIGH);
  digitalWrite(SDA, HIGH);
  
  // Wait for display to power ON
  delay(STARTUP_DELAY);
  clearScreen();
}

/**
 * @brief Initialize selected IO ports for SPI
 * 
 * @param SCL Serial clock pin assignment.
 * @param SDI Serial data pin assignment.
 * @param CS Chip/Slave select pin assignment.
 * @return none
 */
void initLCD_SPI(uint8_t SCL, uint8_t SDI, uint8_t CS)
{
  _interface = SPI;

  // Store pin assignments globally
  _SCL = SCL;
  _SDI = SDI;
  _CS = CS;

  // Set IO modes
  pinMode(CS, OUTPUT);
  pinMode(SCL, OUTPUT);
  pinMode(SDI, OUTPUT);

  // Set pin states
  digitalWrite(CS, HIGH);
  digitalWrite(SCL, HIGH);

  // Wait for display to power ON
  delay(STARTUP_DELAY);
  clearScreen();
}

/**
 * @brief Initalize selected IO ports for RS232.
 * 
 * @param TX Data transmit pin assignment.
 * @return none
 */
void initLCD_RS232(uint8_t TX)
{
  _interface = RS232;

  // Store pin assignments globally
  _TX = TX;

  // Set IO modes
  pinMode(TX, OUTPUT);
  digitalWrite(TX, HIGH);

  // Wait for display to power ON
  delay(STARTUP_DELAY);
  clearScreen();
}

/**
 * @brief Set chip/slave select HIGH and wait for 1ms.
 * 
 * @return none
 */
void setCS()
{
  digitalWrite(_CS, HIGH);
  delay(1);
}

/**
 * @brief Clear chip/slave select and wait for 1ms.
 * 
 * @return none
 */
void clearCS()
{
  digitalWrite(_CS, LOW);
  delay(1);
}

/**
 * @brief Clear the RX pin on the RS232 bus.
 * 
 * @return none
 */
void startBit()
{
  digitalWrite(_TX, LOW);
  delayMicroseconds(RS232_DELAY);
}

/**
 * @brief Set the RX pin on the RS232 bus.
 * 
 * @return none
 */
void stopBit()
{
  digitalWrite(_TX, HIGH);
  delayMicroseconds(RS232_DELAY);
}

/**
 * @brief Send a start condition on the I2C bus.
 * 
 * @return none
 */
void startCondition()
{
  clearSDA();
  clearSCL();
}

/**
 * @brief Send a stop condition on the I2C bus.
 * 
 * @return none
 */
void stopCondition()
{
  setSCL();
  setSDA();
}

/**
 * @brief Set the SDA/SDI pin high on the I2C/SPI bus.
 * 
 * @return none
 */
void setSDA()
{
  digitalWrite(_SDA, HIGH);
  delayMicroseconds(I2C_DELAY);
}

/**
 * @brief Clear the SDA/SDI pin on the I2C/SPI bus.
 * 
 * @return none
 */
void clearSDA()
{
  digitalWrite(_SDA, LOW);
  delayMicroseconds(I2C_DELAY);
}

/**
 * @brief Set the SCL/SCK pin on the I2C/SPI bus.
 * 
 * @return none
 */
void setSCL()
{
  digitalWrite(_SCL, HIGH);
  if(_interface == I2C)
  {
    delayMicroseconds(I2C_DELAY);
  }
}

/**
 * @brief Clear the SCL/SCK pin on the I2C/SPI bus.
 * 
 * @return none
 */
void clearSCL()
{
  digitalWrite(_SCL, LOW);
  if(_interface == I2C)
  {
    delayMicroseconds(I2C_DELAY);
  }
}

/**
 * @brief Set the I2C bus to write mode.
 * 
 * @return none
 */
void setWriteMode()
{
  putData_I2C((SLAVE_ADDRESS << 1) | 0x00);
}

/**
 * @brief Set the I2C bus to read mode.
 * 
 * @return none
 */
void setReadMode()
{
  putData_I2C((SLAVE_ADDRESS << 1) | 0x01);
}

/**
 * @brief Check if an ACK/NACK was received on the I2C bus.
 * 
 * @return uint8_t The ACK/NACK read from the display.
 */
uint8_t getACK()
{
  pinMode(_SDA, INPUT);
  setSCL();

  uint8_t ACK = digitalRead(_SDA);

  pinMode(_SDA, OUTPUT);
  clearSCL();

  return ACK;
}

/**
 * @brief Write 1 byte of data to the display.
 * 
 * @param data Byte of data to be written.
 * @return none
 */
void write(uint8_t data)
{
  switch(_interface)
  {
    case I2C:
      startCondition();
      setWriteMode();
      putData_I2C(data);
      stopCondition();
      break;
    case SPI:
      clearCS();
      putData_SPI(data);
      setCS();
      break;
    case RS232:
      startBit();
      putData_RS232(data);
      stopBit();
      break;
    default:
      break;
  }
  delayMicroseconds(150);
}

/**
 * @brief Write an array of characters to the display.
 * 
 * @param data Pointer to the array of characters.
 * @return none
 */
void writeString(unsigned char* data)
{
  // Iterate through data until null terminator is found.
  while(*data != '\0')
  {
    write(*data);
    data++; // Increment pointer.
  }
}

/**
 * @brief Clock each bit of data on the I2C bus and read ACK.
 * 
 * @param data Byte of data to be put on the I2C data bus.
 * @return none
 */
void putData_I2C(uint8_t data)
{
  for(int i = 7; i >= 0; i--)
  {
    digitalWrite(_SDA, (data >> i) & 0x01);

    setSCL();
    clearSCL();
  }

  getACK();
}

/**
 * @brief Put each bit of data on the SPI data bus.
 * This function sends MSB (D7) first and LSB (D0) last.
 * 
 * @param data Byte of data to be put on the SPI data bus.
 * @return none
 */
void putData_SPI(uint8_t data)
{
  // Write data byte MSB first -> LSB last
  for(int i = 7; i >= 0; i--)
  {
    clearSCL();

    digitalWrite(_SDI, (data >> i) & 0x01);
    
    setSCL();
  }
}

/**
 * @brief Put each bit of data on the RS232 data bus.
 * This function sends LSB (D0) first and MSB (D7) last.
 * 
 * @param data Byte of data to be put on the RS232 data bus.
 * @return none
 */
void putData_RS232(uint8_t data)
{
  // Write data byte LSB first -> MSB last
  for(int i = 0; i <= 7; i++)
  {
    digitalWrite(_TX, (data >> i) & 0x01);
    delayMicroseconds(RS232_DELAY);
  }
}

/**
 * @brief Send the prefix data byte (0xFE).
 * 
 * @return none
 */
void prefix()
{
  write(0xFE);
}

/**
 * @brief Turn the display ON.
 * Display is turned ON by default.
 * 
 * @return none
 */
void displayON()
{
  prefix();
  write(0x41);
}

/**
 * @brief Turn the display OFF.
 * Display is turned ON by default.
 * 
 * @return none
 */
void displayOFF()
{
  prefix();
  write(0x42);
}

/**
 * @brief Set the display cursor position via DDRAM address.
 * 
 * @param position Desired DDRAM address.
 * @return none
 */
void setCursor(uint8_t position)
{
  prefix();
  write(0x45);
  write(position);
}

/**
 * @brief Move the cursor to line 1, column 1.
 * 
 * @return none
 */
void home()
{
  prefix();
  write(0x46);
}

/**
 * @brief Clear the display screen.
 * 
 * @return none
 */
void clearScreen()
{
  prefix();
  write(0x51);
  delay(2);
}

/**
 * @brief Set the display's contrast.
 * 0x00 <= contrast <= 0x32
 * Default: 0x28
 * 
 * @param contrast Desired contrast setting.
 * @return none 
 */
void setContrast(uint8_t contrast)
{
  prefix();
  write(0x52);
  write(contrast);
}

/**
 * @brief Set the display's brightness.
 * 0x01 <= brightness <= 0x08
 * brightness = 0x01 | Backlight OFF
 * brightness = 0x08 | Backlight ON (100%)
 * 
 * @param brightness Desired brightness setting.
 * @return none
 */
void setBrightness(uint8_t brightness)
{
  prefix();
  write(0x53);
  write(brightness);
}

/**
 * @brief Turn the underline cursor ON.
 * 
 * @return none
 */
void underlineCursorON()
{
  prefix();
  write(0x47);
}
