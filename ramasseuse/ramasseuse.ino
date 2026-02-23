#include "src/I2CRelayActuator.h"
#include "src/Knob.h"

/*
 * Variables
 */

// ************
// Potar de cabine
// ************
#define POTAR_PIN A2
#define POTAR_VAL_DEPLIE 0
#define POTAR_VAL_REPLIE 1023
#define POTAR_VAL_BRUIT 10
#define POTAR_DELAI_LECTURE 800 // ms

// ************
// Alerte
// ************
// On lance une alerte si la vitesse de deplacement d'un verin est inferieure a
// une certaine valeur.
// Les vitesses sont celles de deplacemenent du verin (et non de la hauteur du
// rouleau), en mm/s.
#define VERIN_G_VITESSE_MIN 2 // mm/s
#define VERIN_D_VITESSE_MIN 2 // mm/s
#define VERIN_VITESSE_MIN_PERIODE_CHECK 2000 // ms
#define BUZZER_PIN_SORTIE 8

// ************
// Verin gauche
// ************
// On inverse les branchements des capteurs par rapport au verin droit pour ne
// pas avoir les 8 relais allumes en meme temps ni les potentiometres de position
// au maximum de leur resistance en meme temps.
#define VERIN_G_LONGUEUR_UTILISEE 90 // mm

#define VERIN_G_PIN_POS A3
#define VERIN_G_VAL_DEPLIE 547
#define VERIN_G_VAL_REPLIE 231

#define VERIN_G_PIN_COURANT 0
#define VERIN_G_VAL_MAX_COURANT 460

#define VERIN_G_PIN_FIN_COURSE_REPLIE 0
#define VERIN_G_PIN_FIN_COURSE_DEPLIE 0

#define VERIN_G_RELAIS_ADR_I2C 0x11
#define VERIN_G_ETAT_RELAIS_STOP 0
#define VERIN_G_ETAT_RELAIS_REPLIER CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_G_ETAT_RELAIS_DEPLIER CHANNLE1_BIT | CHANNLE2_BIT | CHANNLE3_BIT | CHANNLE4_BIT

// ************
// Verin droit
// ************
#define VERIN_D_LONGUEUR_UTILISEE 90 // mm

#define VERIN_D_PIN_POS A1
#define VERIN_D_VAL_DEPLIE 525
#define VERIN_D_VAL_REPLIE 850

#define VERIN_D_PIN_COURANT 0
#define VERIN_D_VAL_MAX_COURANT 460

#define VERIN_D_PIN_FIN_COURSE_REPLIE 0
#define VERIN_D_PIN_FIN_COURSE_DEPLIE 0

#define VERIN_D_RELAIS_ADR_I2C 0x21
#define VERIN_D_ETAT_RELAIS_STOP 0
#define VERIN_D_ETAT_RELAIS_DEPLIER CHANNLE3_BIT | CHANNLE4_BIT
#define VERIN_D_ETAT_RELAIS_REPLIER CHANNLE1_BIT | CHANNLE2_BIT | CHANNLE3_BIT | CHANNLE4_BIT

// Le rouleau des verins a une amplitude verticale d'environ 60mm.
// On veut une precision de sa longueur a 1mm, soit 16pm de 60mm.
#define VERIN_PRECISION_LONGUEUR 16 // pm


class Actuator : public I2CRelayActuator {
  public:
    Actuator(
      int posAccuracy,
      int foldedInputVal,
      int unfoldedInputVal,
      byte posInputPin,
      byte isTotallyFoldedInputPin,
      byte isTotallyUnfoldedInputPin,
      int minSpeedAlert,
      unsigned int minSpeedCheckPeriod,
      int relayI2CAddr,
      uint8_t stopRelayState,
      uint8_t foldingRelayState,
      uint8_t unfoldingRelayState,
      byte currentPin,
      int maxCurrentVal
    ) : I2CRelayActuator(
      posAccuracy,
      foldedInputVal,
      unfoldedInputVal,
      posInputPin,
      isTotallyFoldedInputPin,
      isTotallyUnfoldedInputPin,
      minSpeedAlert,
      minSpeedCheckPeriod,
      relayI2CAddr,
      stopRelayState,
      foldingRelayState,
      unfoldingRelayState
    ) {
      _currentPin = currentPin;
      _maxCurrentVal = maxCurrentVal;

      pinMode(_currentPin, INPUT);
    };

  protected:
    byte _currentPin;
    int _maxCurrentVal;
    // On leve une alerte quand on a plusieurs valeurs au-dessus du maximum
    // pour ignorer les pointes au demarrage des moteurs.
    // Ne doit pas etre trop petit, sinon la hausse de courant due a un demarrage
    // du moteur n'a pas eu le temps de redescendre avant l'alerte.
    unsigned int _maxTimeAbove = 200;
    unsigned long _lastTimeBelow;

    bool _looksBlocked() {
      // On n'utilise plus les capteurs de courant car il n'y a pas assez de
      // pins analogiques sur l'Arduino (A4 et A5 servent a l'I2C).
      return false;

      /*
      if (!_moving || _targetLen == 0) {
        _lastTimeBelow = millis();
        return false;
      }

      int current = analogRead(_currentPin);
      if (current < _maxCurrentVal) {
        _lastTimeBelow = millis();
      } else if (millis() - _lastTimeBelow > _maxTimeAbove) {
        Serial.print("[DEBUG] Courant trop important ! ");
        Serial.print(current);
        Serial.print(" >= ");
        Serial.println(_maxCurrentVal);
        Serial.println("");
        return true;
      }
      return false;
      */
    };
};

Actuator actuatorLeft(
  VERIN_PRECISION_LONGUEUR,
  VERIN_G_VAL_REPLIE,
  VERIN_G_VAL_DEPLIE,
  VERIN_G_PIN_POS,
  VERIN_G_PIN_FIN_COURSE_REPLIE,
  VERIN_G_PIN_FIN_COURSE_DEPLIE,
  (float)VERIN_G_VITESSE_MIN / VERIN_G_LONGUEUR_UTILISEE * 1000,
  VERIN_VITESSE_MIN_PERIODE_CHECK,
  VERIN_G_RELAIS_ADR_I2C,
  VERIN_G_ETAT_RELAIS_STOP,
  VERIN_G_ETAT_RELAIS_REPLIER,
  VERIN_G_ETAT_RELAIS_DEPLIER,
  VERIN_G_PIN_COURANT,
  VERIN_G_VAL_MAX_COURANT
);

Actuator actuatorRight(
  VERIN_PRECISION_LONGUEUR,
  VERIN_D_VAL_REPLIE,
  VERIN_D_VAL_DEPLIE,
  VERIN_D_PIN_POS,
  VERIN_D_PIN_FIN_COURSE_REPLIE,
  VERIN_D_PIN_FIN_COURSE_DEPLIE,
  (float)VERIN_D_VITESSE_MIN / VERIN_D_LONGUEUR_UTILISEE * 1000,
  VERIN_VITESSE_MIN_PERIODE_CHECK,
  VERIN_D_RELAIS_ADR_I2C,
  VERIN_D_ETAT_RELAIS_STOP,
  VERIN_D_ETAT_RELAIS_REPLIER,
  VERIN_D_ETAT_RELAIS_DEPLIER,
  VERIN_D_PIN_COURANT,
  VERIN_D_VAL_MAX_COURANT
);

Knob targetLenKnob(
  POTAR_VAL_REPLIE,
  POTAR_VAL_DEPLIE,
  POTAR_PIN,
  POTAR_VAL_BRUIT,
  POTAR_DELAI_LECTURE
);

bool alertRaised = false;

void raiseAlert() {
  alertRaised = true;
  digitalWrite(BUZZER_PIN_SORTIE, HIGH);
}

void stopAlert() {
  alertRaised = false;
  digitalWrite(BUZZER_PIN_SORTIE, LOW);
}

void setup() {
  // Serial.begin(9600);

  actuatorLeft.stop();
  actuatorRight.stop();

  pinMode(BUZZER_PIN_SORTIE, OUTPUT);

  stopAlert();
}

void loop() {
  // Tout d'abord, on arrete les verins au besoin (cible atteinte, fin de course
  // ou blocage).
  actuator_stop_reason_t stopReasonLeft = actuatorLeft.stopIfNecessary();
  actuator_stop_reason_t stopReasonRight = actuatorRight.stopIfNecessary();

  bool problemLeft = (stopReasonLeft == STOP_BLOCKED) || (stopReasonLeft == STOP_TOO_SLOW);
  bool problemRight = (stopReasonRight == STOP_BLOCKED) || (stopReasonRight == STOP_TOO_SLOW);

  // Si un verin est bloque (probablement parce que quelque chose est coince dessous),
  // on arrete les deux verins et lance une alerte pour que l'usager puisse
  // regler le probleme.
  if (problemLeft || problemRight) {
    /*
    Serial.print("[DEBUG] Raison de l'arret a gauche = ");
    Serial.println(stopReasonLeft);
    Serial.print("[DEBUG] Raison de l'arret a droite = ");
    Serial.println(stopReasonRight);
    Serial.println("");
    */
    actuatorLeft.stop();
    actuatorRight.stop();
    raiseAlert();
  }

  int targetLen = targetLenKnob.readTargetLen();

  // Pour arreter l'alarme, l'usager place le potentiometre de cabine en position
  // replie, de sorte a liberer ce qui est coince sous le verin.
  // Pour que cette instruction fonctionne, il faut que le potentiometre de
  // cabine retourne bien 0 quand il est en position extreme replie, et non pas
  // une petite valeur positive due a une imprecision.
  if (targetLen == 0 || targetLen == 1000) {
    stopAlert();
  }

  // Si la longueur cible a change, on indique aux verins de lancer le deplacement
  // vers la nouvelle cible. Une fois sa cible atteinte, le verin s'arrete et
  // attend une nouvelle position cible.
  //
  // Comme la longueur cible n'est retournee qu'une seule fois par targetLenKnob
  // (le reste du temps, il retourne NO_TARGET_LEN_CHANGE), le verin ne cherchera
  // pas en boucle a atteindre la meme position cible. Cela implique que s'il s'est
  // arrete a cote de sa cible parce que son capteur de position a retourne une
  // valeur invalide, il ne corrigera pas sa position de lui-meme. L'usager devra
  // changer de position cible pour que le verin se deplace de nouveau.
  if (targetLen != NO_TARGET_LEN_CHANGE && !alertRaised) {
    actuatorLeft.startMovingTo(targetLen);
    actuatorRight.startMovingTo(targetLen);
  }
}
