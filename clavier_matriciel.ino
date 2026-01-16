#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// Configuration du clavier matriciel
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Configuration de l'écran LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuration des composants
const int LED_GREEN = 10;
const int LED_RED = 13;  // LED rouge pour accès refusé
const int BUZZER_PIN = 11;
const int SERVO_PIN = 12;

// Configuration du servo
const int SERVO_LOCKED = 90;
const int SERVO_UNLOCKED = 180;

// Code secret et variables
String codeSecret = "2322";// Vous pouvez modiffer votre code d'acces ici
String inputCode = "";
const int MAX_CODE_LENGTH = 6;

// Gestion des tentatives
int failedAttempts = 0;
const int MAX_ATTEMPTS = 3;
unsigned long lockoutTime = 0;
const unsigned long LOCKOUT_DURATION = 30000; // 30 secondes

Servo monservo;

void setup() {
  Serial.begin(9600);
  
  // Initialisation LCD
  lcd.init();
  lcd.backlight();
  
  // Configuration des pins
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialisation du servo
  monservo.attach(SERVO_PIN);
  monservo.write(SERVO_LOCKED);
  
  // Message de bienvenue
  displayWelcome();
}

void loop() {
  // Vérifier si le système est verrouillé
  if (lockoutTime > 0) {
    unsigned long remaining = (lockoutTime - millis()) / 1000;
    if (remaining > 0) {
      lcd.setCursor(0, 1);
      lcd.print("Attente: ");
      lcd.print(remaining);
      lcd.print("s  ");
      delay(1000);
      return;
    } else {
      lockoutTime = 0;
      failedAttempts = 0;
      displayWelcome();
    }
  }
  
  char key = keypad.getKey();
  
  if (key) {
    handleKeyPress(key);
  }
}

void handleKeyPress(char key) {
  if (key == 'A') {
    // Validation du code
    validateCode();
  } else if (key == 'C') {
    // Effacer le code
    clearInput();
  } else if (key == 'D') {
    // Effacer le dernier caractère
    if (inputCode.length() > 0) {
      inputCode.remove(inputCode.length() - 1);
      displayInput();
    }
  } else {
    // Ajouter un chiffre
    if (inputCode.length() < MAX_CODE_LENGTH) {
      inputCode += key;
      displayInput();
      tone(BUZZER_PIN, 1500, 50); // Bip court
    }
  }
}

void validateCode() {
  if (inputCode.length() == 0) {
    return;
  }
  
  if (inputCode == codeSecret) {
    accessGranted();
  } else {
    accessDenied();
  }
  
  inputCode = "";
}

void accessGranted() {
  lcd.clear();
  lcd.print("  ACCES AUTORISE");
  lcd.setCursor(0, 1);
  lcd.print("  Bienvenue!");
  
  // LED verte
  digitalWrite(LED_GREEN, HIGH);
  
  // Son de succès
  tone(BUZZER_PIN, 1000, 200);
  delay(250);
  tone(BUZZER_PIN, 1500, 200);
  
  // Ouvrir la serrure
  monservo.write(SERVO_UNLOCKED);
  delay(3000);
  monservo.write(SERVO_LOCKED);
  
  digitalWrite(LED_GREEN, LOW);
  
  // Réinitialiser les tentatives
  failedAttempts = 0;
  
  delay(1000);
  displayWelcome();
}

void accessDenied() {
  failedAttempts++;
  
  lcd.clear();
  lcd.print("  ACCES REFUSE");
  lcd.setCursor(0, 1);
  lcd.print("Tentative ");
  lcd.print(failedAttempts);
  lcd.print("/");
  lcd.print(MAX_ATTEMPTS);
  
  // LED rouge clignotante
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_RED, HIGH);
    tone(BUZZER_PIN, 400, 200);
    delay(200);
    digitalWrite(LED_RED, LOW);
    delay(200);
  }
  
  // Vérifier le nombre de tentatives
  if (failedAttempts >= MAX_ATTEMPTS) {
    systemLockout();
  } else {
    delay(1500);
    displayWelcome();
  }
}

void systemLockout() {
  lcd.clear();
  lcd.print("SYSTEME BLOQUE!");
  lcd.setCursor(0, 1);
  lcd.print("Trop de tentatives");
  
  // Son d'alarme
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_RED, HIGH);
    tone(BUZZER_PIN, 800, 300);
    delay(300);
    digitalWrite(LED_RED, LOW);
    tone(BUZZER_PIN, 400, 300);
    delay(300);
  }
  
  lockoutTime = millis() + LOCKOUT_DURATION;
  
  lcd.clear();
  lcd.print("Reessayez dans");
}

void clearInput() {
  inputCode = "";
  displayInput();
  tone(BUZZER_PIN, 800, 50);
}

void displayWelcome() {
  lcd.clear();
  lcd.print("Entrez le code:");
  lcd.setCursor(0, 1);
  lcd.print("> ");
}

void displayInput() {
  lcd.setCursor(0, 1);
  lcd.print("> ");
  
  // Afficher des astérisques pour masquer le code
  for (int i = 0; i < inputCode.length(); i++) {
    lcd.print("*");
  }
  
  // Effacer les caractères restants
  for (int i = inputCode.length(); i < MAX_CODE_LENGTH; i++) {
    lcd.print(" ");
  }
}