#include <multi_channel_relay.h>

#define POTAR_PIN_ENTREE A1

#define VERIN_G_PIN_ENTREE 0
#define VERIN_G_RELAIS_ADR_I2C 0x11
#define VERIN_G_ETAT_RELAIS_STOP 0
#define VERIN_G_ETAT_RELAIS_REPLIER CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_G_ETAT_RELAIS_DEPLIER CHANNLE1_BIT | CHANNLE2_BIT | CHANNLE3_BIT | CHANNLE4_BIT

#define VERIN_D_PIN_ENTREE 0
#define VERIN_D_RELAIS_ADR_I2C 0x21
#define VERIN_D_ETAT_RELAIS_STOP 0
#define VERIN_D_ETAT_RELAIS_REPLIER CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_D_ETAT_RELAIS_DEPLIER CHANNLE1_BIT | CHANNLE2_BIT | CHANNLE3_BIT | CHANNLE4_BIT

#define CALIBRATION_INACTIVE 0
#define CALIBRATION_POTAR_CABINE 1
#define CALIBRATION_VERIN_G 2
#define CALIBRATION_VERIN_D 3

// *****************
// Assigner une des valeurs precedentes et recharger le programme sur l'Arduino.
// Puis consulter le moniteur Serie.
// *****************
byte mode = CALIBRATION_INACTIVE;

Multi_Channel_Relay actuator_left_relay;
Multi_Channel_Relay actuator_right_relay;

void calibrateKnob() {
  Serial.println("Valeur du potentiometre de cabine :");
  Serial.println(analogRead(POTAR_PIN_ENTREE));
}

void calibrateActuator(
  int targetPosPin,
  Multi_Channel_Relay *relay,
  int posPin,
  uint8_t stopRelayState,
  uint8_t foldingRelayState,
  uint8_t unfoldingRelayState
) {
  const int LOW_THRESH = 300;
  const int HIGH_THRESH = 700;

  // Si le potentiometre de cabine est "au centre", on n'actionne pas le verin
  int targetPos = analogRead(targetPosPin);
  if (targetPos >= LOW_THRESH && targetPos <= HIGH_THRESH) {
    Serial.println("Potentiometre au centre, on ne fait rien.");
    relay->channelCtrl(stopRelayState); 
  } else if (targetPos < LOW_THRESH) {
    Serial.println("On replie le verin.");
    relay->channelCtrl(foldingRelayState);
  } else if (targetPos > LOW_THRESH) {
    Serial.println("On deplie le verin.");
    relay->channelCtrl(unfoldingRelayState);
  }
  Serial.print("Position du verin : ");
  Serial.println(analogRead(posPin));
}

void setup() {
  Serial.begin(9600);

  pinMode(POTAR_PIN_ENTREE, INPUT);
  pinMode(VERIN_G_PIN_ENTREE, INPUT);
  pinMode(VERIN_D_PIN_ENTREE, INPUT);

  actuator_left_relay.begin(VERIN_G_RELAIS_ADR_I2C);
  actuator_right_relay.begin(VERIN_D_RELAIS_ADR_I2C);
}

void loop() {
  Serial.println();
  Serial.println("--------------------------------------");

  if (mode == CALIBRATION_POTAR_CABINE) {
    calibrateKnob();
    delay(1000);
  } else if (mode == CALIBRATION_VERIN_G) {
    calibrateActuator(
      POTAR_PIN_ENTREE,
      &actuator_left_relay,
      VERIN_G_PIN_ENTREE,
      VERIN_G_ETAT_RELAIS_STOP,
      VERIN_G_ETAT_RELAIS_REPLIER,
      VERIN_G_ETAT_RELAIS_DEPLIER
    );
    delay(1000);
  } else if (mode == CALIBRATION_VERIN_D) {
    calibrateActuator(
      POTAR_PIN_ENTREE,
      &actuator_right_relay,
      VERIN_D_PIN_ENTREE,
      VERIN_D_ETAT_RELAIS_STOP,
      VERIN_D_ETAT_RELAIS_REPLIER,
      VERIN_D_ETAT_RELAIS_DEPLIER
    );
    delay(1000);
  } else {
    Serial.println("Calibration inactive. Modifier la variable 'mode' et televerser le programme.");
    delay(3000);
  }
}
