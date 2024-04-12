#define ALLOCATE_ARRAY_CHUNK_SIZE 16
// Pins des LEDs
const int ledPins[] = {5, 18, 19, 21};
 
// Pins des boutons
const int buttonPins[] = {12, 14, 27, 26};
 
// Pin du buzzer
const int buzzerPin = 4;

int currentLengthOfSequence = 0;
 
void printWholeArray(byte *arr) {

  int i = 0;

  while(arr[i] != 0) {

    Serial.print("Index ");

    Serial.print(i);

    Serial.print(": ");

    Serial.print(arr[i]);

    Serial.println();
 
    i++;

  }

}
 
byte* addToArray(byte numberToAdd, byte *arr, int *currentArraySize) {

  if (currentLengthOfSequence >= *currentArraySize - 1) {

    int newSize = ALLOCATE_ARRAY_CHUNK_SIZE * (*currentArraySize / ALLOCATE_ARRAY_CHUNK_SIZE + 1);

    arr = realocateArrayToNewSize(arr, *currentArraySize, newSize);

    *currentArraySize += ALLOCATE_ARRAY_CHUNK_SIZE;

  }
 
  arr[currentLengthOfSequence] = numberToAdd;

  currentLengthOfSequence++;
 
  return arr;

}
 
void readSequence(byte *arr) {

  for (int i = 0; i < currentLengthOfSequence; i++) {

    byte currentInSequence = arr[i] - 1;

    digitalWrite(ledPins[currentInSequence], HIGH);

    tone(buzzerPin, 1000);

    delay(500);

    digitalWrite(ledPins[currentInSequence], LOW);

    noTone(buzzerPin);

    delay(500);

  }

}
 
byte* realocateArrayToNewSize(byte *oldArray, int oldSize, int newSize) {

  byte *newArray = emptyArray(newSize);

  memcpy(newArray, oldArray, oldSize);

  free(oldArray);

  return newArray;

}
 
byte* emptyArray(int size) {

  byte *array = (byte *)malloc(size * sizeof(byte));

  for(int i = 0; i < size; i++)

    array[i] = 0;

  return array;

}
 
bool usersTurnToPressButtons(byte *arr) {

  for(int i = 0; i < currentLengthOfSequence; i++) {

    unsigned long startTime = millis();

    unsigned long currentTime;

    unsigned long maxTime = 5000;
 
    int pressedButton = 0;

    for(;;) {

      for(int j = 0; j < 4; j++) {

        if (digitalRead(buttonPins[j]) == LOW) {

          pressedButton = j + 1;

          digitalWrite(ledPins[j], HIGH);

          tone(buzzerPin, 1000);

          delay(500);

          digitalWrite(ledPins[j], LOW);

           noTone(buzzerPin);

          break;

        }

      }
 
      if (pressedButton != 0) break;
 
      currentTime = millis();
 
      if (currentTime - startTime >= maxTime)

        return false;

    }

    if (pressedButton != arr[i]) {

      return false;

    }

  }

  return true;

}
 
void setup() {

  pinMode(buzzerPin, OUTPUT); // Définir la broche du buzzer comme sortie

  Serial.begin(9600);

  for(byte i = 0; i < 4; i++)

    pinMode(ledPins[i], OUTPUT);

  for(byte i = 0; i < 4; i++)

    pinMode(buttonPins[i], INPUT_PULLUP);

}

 
void loop() {

  byte *arr = emptyArray(ALLOCATE_ARRAY_CHUNK_SIZE);
  int currentArraySize = ALLOCATE_ARRAY_CHUNK_SIZE;

  for(;;) {
    arr = addToArray(random(1, sizeof(ledPins)/sizeof(ledPins[0]) + 1), arr, &currentArraySize);
    readSequence(arr);

    if (!usersTurnToPressButtons(arr)) {
      // Allumer toutes les LEDs et jouer un son de désolation
      for(int i = 0; i < sizeof(ledPins)/sizeof(ledPins[0]); i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      for (int i = 0; i < 3; i++) {
        tone(buzzerPin, 200);
        delay(200);
        noTone(buzzerPin);
        delay(200);
      }
      for(int i = 0; i < sizeof(ledPins)/sizeof(ledPins[0]); i++) {
        digitalWrite(ledPins[i], LOW);
      }
      delay(1000);
      break;
    }
    delay(500);
  }

  free(arr);

  currentLengthOfSequence = 0;

}

