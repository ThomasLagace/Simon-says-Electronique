#define ALLOCATE_ARRAY_CHUNK_SIZE 16
const byte LEDS[] = {12, 27, 33, 15};
const byte BUTTONS[] = {13, 14, 32, 21};
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
    digitalWrite(LEDS[currentInSequence], HIGH);
    delay(500);
    digitalWrite(LEDS[currentInSequence], LOW);
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
        if (digitalRead(BUTTONS[j]) == LOW) {
          pressedButton = j + 1;
          digitalWrite(LEDS[j], HIGH);
          delay(500);
          digitalWrite(LEDS[j], LOW);
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
  Serial.begin(9600);
  for(byte i = 0; i < 4; i++)
    pinMode(LEDS[i], OUTPUT);
  for(byte i = 0; i < 4; i++)
    pinMode(BUTTONS[i], INPUT_PULLUP);
}

void loop() {
  byte *arr = emptyArray(ALLOCATE_ARRAY_CHUNK_SIZE);
  int currentArraySize = ALLOCATE_ARRAY_CHUNK_SIZE;
  for(;;) {
    arr = addToArray(random(1, 5), arr, &currentArraySize);
    readSequence(arr);

    if (!usersTurnToPressButtons(arr)) {
      for(int i = 0; i < 4; i++) 
        digitalWrite(LEDS[i], HIGH);
      delay(1000);
      for(int i = 0; i < 4; i++) 
        digitalWrite(LEDS[i], LOW);
      delay(1000);
      break;
    }
    delay(500);
  }
  free(arr);
  currentLengthOfSequence = 0;
}
