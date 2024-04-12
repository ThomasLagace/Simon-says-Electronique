#define ALLOCATE_ARRAY_CHUNK_SIZE 16
const byte LEDS[] = {12, 27, 33, 15};
const byte BUTTONS[] = {13, 14, 32, 21};

const unsigned short DELAY_PUSH_BUTTONS = 500;
const unsigned short DELAY_BETWEEN_ROUNDS = 1000;
const unsigned short DELAY_BETWEEN_READING_SEQUENCE = 500;
const unsigned long MAX_TIME_TO_RESPOND = 5000;

typedef struct {
  byte *arrayPointer;
  unsigned int allocatedSize;
  unsigned int length;
} SequenceList;

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

void addToSequence(byte numberToAdd, SequenceList *sequence) {
  if (sequence->length >= sequence->allocatedSize - 1) {
    int newSize = ALLOCATE_ARRAY_CHUNK_SIZE * (sequence->allocatedSize / ALLOCATE_ARRAY_CHUNK_SIZE + 1);
    sequence->arrayPointer = realocateArrayToNewSize(sequence->arrayPointer, sequence->allocatedSize, newSize);
    sequence->allocatedSize += ALLOCATE_ARRAY_CHUNK_SIZE;
  }

  sequence->arrayPointer[sequence->length] = numberToAdd;
  sequence->length++;
}

void readSequence(SequenceList *sequence) {
  for (int i = 0; i < sequence->length; i++) {
    byte currentInSequence = sequence->arrayPointer[i] - 1;
    digitalWrite(LEDS[currentInSequence], HIGH);
    delay(DELAY_BETWEEN_READING_SEQUENCE);
    digitalWrite(LEDS[currentInSequence], LOW);
    delay(DELAY_BETWEEN_READING_SEQUENCE);
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

bool usersTurnToPressButtons(SequenceList *sequence) {
  for(int i = 0; i < sequence->length; i++) {
    int pressedButton = 0;

    pressedButton = waitForButtonPressOrMaxTime(MAX_TIME_TO_RESPOND);
    
    if (pressedButton == 0 || pressedButton != sequence->arrayPointer[i]) {
      return false;
    }

    digitalWrite(LEDS[pressedButton - 1], HIGH);
    delay(DELAY_PUSH_BUTTONS);
    digitalWrite(LEDS[pressedButton - 1], LOW);
  }

  return true;
}

int waitForButtonPressOrMaxTime(long maxTime) {
  unsigned long startTime = millis();
  unsigned long currentTime;

  for(;;) {
    for(int i = 0; i < sizeof(BUTTONS) / sizeof(BUTTONS[0]); i++) {
      if (digitalRead(BUTTONS[i]) == LOW) 
        return i + 1;
    }

    currentTime = millis();

    if (currentTime - startTime >= maxTime)
      return 0;
  }
}

void setup() {
  Serial.begin(9600);
  for(byte i = 0; i < 4; i++)
    pinMode(LEDS[i], OUTPUT);
  for(byte i = 0; i < 4; i++)
    pinMode(BUTTONS[i], INPUT_PULLUP);
}

void loop() {
  SequenceList sequence;
  sequence.arrayPointer = emptyArray(ALLOCATE_ARRAY_CHUNK_SIZE);
  sequence.allocatedSize = ALLOCATE_ARRAY_CHUNK_SIZE;
  sequence.length = 0;

  for(;;) {
    addToSequence(random(1, 5), &sequence);
    readSequence(&sequence);

    if (!usersTurnToPressButtons(&sequence)) {
      for(int i = 0; i < 4; i++) 
        digitalWrite(LEDS[i], HIGH);
      delay(DELAY_BETWEEN_ROUNDS);
      for(int i = 0; i < 4; i++) 
        digitalWrite(LEDS[i], LOW);
      delay(DELAY_BETWEEN_ROUNDS);
      break;
    }

    delay(500);
  }
  free(sequence.arrayPointer);
  sequence.arrayPointer = NULL;
}
