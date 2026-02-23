#include <multi_channel_relay.h>

#define POTAR_PIN A2

#define VERIN_G_PIN_POS A3
#define VERIN_G_PIN_COURANT 0
#define VERIN_G_RELAIS_ADR_I2C 0x11
#define VERIN_G_ETAT_RELAIS_STOP 0
#define VERIN_G_ETAT_RELAIS_REPLIER CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_G_ETAT_RELAIS_DEPLIER CHANNLE1_BIT | CHANNLE2_BIT | CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_G_COURANT_MAX_VAL 460

#define VERIN_D_PIN_POS A1
#define VERIN_D_PIN_COURANT 0
#define VERIN_D_RELAIS_ADR_I2C 0x21
#define VERIN_D_ETAT_RELAIS_STOP 0
#define VERIN_D_ETAT_RELAIS_DEPLIER CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_D_ETAT_RELAIS_REPLIER CHANNLE1_BIT | CHANNLE2_BIT | CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_D_COURANT_MAX_VAL 460

#define CALIBRATION_INACTIVE 0
#define CALIBRATION_POTAR_CABINE 1
#define CALIBRATION_VERIN_G 2
#define CALIBRATION_VERIN_D 3

// *****************
// Assigner une des valeurs precedentes et recharger le programme sur l'Arduino.
// Puis consulter le moniteur Serie.
// *****************
byte mode = CALIBRATION_POTAR_CABINE;

Multi_Channel_Relay actuator_left_relay;
Multi_Channel_Relay actuator_right_relay;

const int LOW_THRESH = 300;
const int HIGH_THRESH = 700;

unsigned long lastPrintPosMillis = 0;
const int PRINT_POS_PERIOD = 2000;

unsigned long lastPrintCurrentMillis = 0;
const int PRINT_CURRENT_PERIOD = 200;

int maxCurrent = 0;

void calibrateKnob() {
  Serial.println();
  Serial.println("--------------------------------------");
  Serial.println("Valeur du potentiometre de cabine :");
  Serial.println(analogRead(POTAR_PIN));
}

void calibrateActuator(
  char name,
  int targetPosPin,
  Multi_Channel_Relay *relay,
  int posPin,
  int currentPin,
  uint8_t stopRelayState,
  uint8_t foldingRelayState,
  uint8_t unfoldingRelayState,
  int currentStopVal
) {
  bool printPos = false;
  if (lastPrintPosMillis == 0 || (millis() - lastPrintPosMillis) > PRINT_POS_PERIOD) {
    printPos = true;
    lastPrintPosMillis = millis();
  }
  bool printCurrent = false;
  if (lastPrintCurrentMillis == 0 || (millis() - lastPrintCurrentMillis) > PRINT_CURRENT_PERIOD) {
    printCurrent = true;
    lastPrintCurrentMillis = millis();
  }
  if (currentPin == 0) {
    printCurrent = false;
  }

  if (printPos) {
    Serial.println();
    Serial.println("--------------------------------------");
    Serial.print("VERIN ");
    Serial.println(name);
    Serial.println("");
  }

  // Si le potentiometre de cabine est "au centre", on n'actionne pas le verin
  int targetPos = analogRead(targetPosPin);
  int current =  analogRead(currentPin);

  if (currentPin != 0 && current >= currentStopVal) {
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("COURANT IMPORTANT, STOP.");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!");
  }

  if (targetPos >= LOW_THRESH && targetPos <= HIGH_THRESH) {
    if (printPos) {
      Serial.println("Potentiometre de cabine au centre, on ne fait rien.");
    }
    relay->channelCtrl(stopRelayState); 
  } else if (targetPos < LOW_THRESH) {
    if (printPos) {
      Serial.println("Potentiometre de cabine en position basse, on deplie le verin.");
    }
    relay->channelCtrl(unfoldingRelayState);
  } else if (targetPos > HIGH_THRESH) {
    if (printPos) {
      Serial.println("Potentiometre de cabine en position haute, on replie le verin.");
    }
    relay->channelCtrl(foldingRelayState);
  }

  if (current > maxCurrent) {
    maxCurrent = current;
  }

  if (printCurrent) {
    Serial.print("Courant max : ");
    Serial.println(maxCurrent);
    maxCurrent = 0;
  }

  if (printPos) {
    Serial.print("Position du verin : ");
    Serial.println(analogRead(posPin));
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(POTAR_PIN, INPUT);
  pinMode(VERIN_G_PIN_POS, INPUT);
  pinMode(VERIN_D_PIN_POS, INPUT);
  pinMode(VERIN_G_PIN_COURANT, INPUT);
  pinMode(VERIN_D_PIN_COURANT, INPUT);

  actuator_left_relay.begin(VERIN_G_RELAIS_ADR_I2C);
  actuator_right_relay.begin(VERIN_D_RELAIS_ADR_I2C);

  actuator_left_relay.channelCtrl(VERIN_G_ETAT_RELAIS_STOP);
  actuator_right_relay.channelCtrl(VERIN_D_ETAT_RELAIS_STOP);
}

void loop() {
  if (mode == CALIBRATION_POTAR_CABINE) {
    calibrateKnob();
    delay(1000);
  } else if (mode == CALIBRATION_VERIN_G) {
    calibrateActuator(
      'G',
      POTAR_PIN,
      &actuator_left_relay,
      VERIN_G_PIN_POS,
      VERIN_G_PIN_COURANT,
      VERIN_G_ETAT_RELAIS_STOP,
      VERIN_G_ETAT_RELAIS_REPLIER,
      VERIN_G_ETAT_RELAIS_DEPLIER,
      VERIN_G_COURANT_MAX_VAL
    );
  } else if (mode == CALIBRATION_VERIN_D) {
    calibrateActuator(
      'D',
      POTAR_PIN,
      &actuator_right_relay,
      VERIN_D_PIN_POS,
      VERIN_D_PIN_COURANT,
      VERIN_D_ETAT_RELAIS_STOP,
      VERIN_D_ETAT_RELAIS_REPLIER,
      VERIN_D_ETAT_RELAIS_DEPLIER,
      VERIN_D_COURANT_MAX_VAL
    );
  } else {
    Serial.println("Calibration inactive. Modifier la variable 'mode' et televerser le programme.");
    delay(3000);
  }
}
