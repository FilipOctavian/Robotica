#include <Arduino.h>

const int ledPinR = 6;
const int ledPinB = 5;
const int ledPinG = 4;
const int startStopButtonPin = 2;
const int difficultyButtonPin = 3;

// Game variables
enum GameState { WAITING, COUNTDOWN, PLAYING, GAME_OVER };
GameState gameState = WAITING;

int difficulty = 1; 
unsigned long roundTime = 30000;
unsigned long timeLimit;
bool wordTypedCorrectly = false;

const char *words[] = {"hello", "world", "arduino", "racer", "example", "typing", "speed", "test"};
int totalWordsTyped = 0;
int currentWordIndex = 0;

void resetGame() {
    digitalWrite(ledPinR, LOW);
    digitalWrite(ledPinG, LOW);
    digitalWrite(ledPinB, HIGH);
    totalWordsTyped = 0;
    currentWordIndex = 0;
    gameState = WAITING;
}

void endGame() {
    gameState = GAME_OVER;
    digitalWrite(ledPinG, LOW);
    digitalWrite(ledPinB, LOW);
    digitalWrite(ledPinR, HIGH); // LED red
    Serial.print("Game Over! Total words typed correctly: ");
    Serial.println(totalWordsTyped);
    resetGame();
    
}

void displayNextWord() {
    if (currentWordIndex < sizeof(words) / sizeof(words[0])) {
        Serial.println(words[currentWordIndex]);
    } else {
        endGame();
    }
}

void startGame() {
    gameState = COUNTDOWN;
    for (int i = 3; i > 0; i--) {
        Serial.println(i);
        delay(1000);
    }
    digitalWrite(ledPinG, HIGH); // LED green
    gameState = PLAYING;
    timeLimit = millis() + roundTime;
    displayNextWord();
}

void checkUserInput() {
    static String userInput = ""; // Store user input

    // Check if we have timed out
    if (millis() > timeLimit) {
        endGame();
        return;
    }

    // Check for available input in the Serial buffer
    if (Serial.available() > 0) {
        char incomingChar = Serial.read(); // Read a single character

        // Handle backspace (if needed)
        if (incomingChar == 8) { // ASCII code for backspace
            if (userInput.length() > 0) {
                userInput.remove(userInput.length() - 1); // Remove last character
            }
        } else if (incomingChar == '\n') { // Check for enter key

            // Compare user input to the current word
            if (userInput.equals(words[currentWordIndex])) {
                totalWordsTyped++; // Increment correct count
                currentWordIndex++; // Move to the next word
                userInput = ""; // Clear user input
                displayNextWord(); // Display the next word
            } else {
                userInput = ""; // Clear user input after error
            }
        } else {
            // Append the character to user input if it's valid
            if (isPrintable(incomingChar)) {
                userInput += incomingChar; // Add character to input
            }
        }
    }
}

void changeDifficulty() {
    difficulty = (difficulty + 1) % 3; // Cycle through difficulties
if (difficulty == 0) {
    Serial.print("Easy");
} else if (difficulty == 1) {
    Serial.print("Medium");
} else {
    Serial.print("Hard");
}
    Serial.println(" mode on!");

    switch (difficulty) {
        case 0: roundTime = 30000; break;
        case 1: roundTime = 20000; break;
        case 2: roundTime = 10000; break;
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(ledPinR, OUTPUT);
    pinMode(ledPinG, OUTPUT);
    pinMode(ledPinB, OUTPUT);
    pinMode(startStopButtonPin, INPUT_PULLUP);
    pinMode(difficultyButtonPin, INPUT_PULLUP);
    resetGame();
}

void loop() {
    if (digitalRead(startStopButtonPin) == LOW) {
        if (gameState == PLAYING) {
            endGame();
        } else if (gameState == WAITING) {
            startGame();
        }
        delay(500);
    }

    if (gameState == PLAYING) {
        checkUserInput();
    }

    if (gameState == WAITING && digitalRead(difficultyButtonPin) == LOW) {
        changeDifficulty();
        delay(500); 
    }
}
