#include "BaseActuator.h"

BaseActuator::BaseActuator(
  int lenAccuracy,
  int foldedInputVal,
  int unfoldedInputVal,
  byte lenInputPin,
  byte isTotallyFoldedInputPin,
  byte isTotallyUnfoldedInputPin,
  int minSpeedAlert,
  unsigned int checkPeriod
) {
    _lenAccuracy = lenAccuracy;
  _foldedInputVal = foldedInputVal;
  _unfoldedInputVal = unfoldedInputVal;
  _lenInputPin = lenInputPin;

  _minSpeedAlert = minSpeedAlert;
  _checkPeriod = checkPeriod;

  _isTotallyFoldedInputPin = isTotallyFoldedInputPin;
  _isTotallyUnfoldedInputPin = isTotallyUnfoldedInputPin;

  pinMode(_lenInputPin, INPUT);
  pinMode(_isTotallyFoldedInputPin, INPUT);
  pinMode(_isTotallyUnfoldedInputPin, INPUT);
}

int BaseActuator::_readLen() {
  /*
   * Selon les branchements du potentiometre, les valeurs basses du capteur
   * correspondent a une position repliee ou depliee du verin.
   */
  const int inputVal = analogRead(_lenInputPin);
 
  // float pour les divisions plus bas
  float scale = abs(_unfoldedInputVal - _foldedInputVal);

  if (_foldedInputVal < _unfoldedInputVal) {
    // Les valeurs basses du potentiometre correspondent a un verin replie.
    if (inputVal <= _foldedInputVal) {
      return 0;
    }
    if (inputVal >= _unfoldedInputVal) {
      return 1000;
    }
    const float distToFolded = inputVal - _foldedInputVal;
    return distToFolded / scale * 1000;
  }

  // Les valeurs hautes du potentiometre correspondent a un verin replie.
  if (inputVal >= _foldedInputVal) {
    return 0;
  }
  if (inputVal <= _unfoldedInputVal) {
    return 1000;
  }
  const float distToUnfolded = inputVal - _unfoldedInputVal;
  return (1 - distToUnfolded / scale) * 1000;
}

bool BaseActuator::isFolding() {
  return _moving && _folding;
}

bool BaseActuator::isUnfolding() {
  return _moving && !_folding;
}

bool BaseActuator::isTotallyFolded() {
  if (!_isTotallyFoldedInputPin) {
    return false;
  }
  return digitalRead(_isTotallyFoldedInputPin) == HIGH;
}

bool BaseActuator::isTotallyUnfolded() {
  if (!_isTotallyUnfoldedInputPin) {
    return false;
  }
  return digitalRead(_isTotallyUnfoldedInputPin) == HIGH;
}

bool BaseActuator::_looksBlocked() {
  if (!_moving) {
    return false;
  }
  if (_lastCheckLen == -1) {
    _lastCheckLen = _readLen();
    _lastCheckTime = millis();
    return false;
  }

  int len = _readLen();
  unsigned long now = millis();
  unsigned long duration = now - _lastCheckTime;

  // On attend que le deplacement ait dure un certain temps avant de calculer
  // la vitesse, sinon on est trop soumis aux petits aleas de deplacement.
  if (duration < _checkPeriod) {
    return false;
  }

  float movingTime = (float)duration / 1000;
  int speed = abs(len - _lastCheckLen) / movingTime;
  
  _lastCheckLen = len;
  _lastCheckTime = now;

  return speed > _minSpeedAlert;
}

bool BaseActuator::check() {
  if(_looksBlocked()) {
    stop();
    return false;
  }
  int lenDelta = _targetLen - _readLen();
  bool isAtLen = abs(lenDelta) < _lenAccuracy;
  bool cannotStep = (lenDelta < 0 && isTotallyFolded()) || (lenDelta > 0 && isTotallyUnfolded());
  if (isAtLen || cannotStep) {
    stop();
  }
  return true;
}

void BaseActuator::startMovingTo(int target) {
  _targetLen = target;
  int lenDelta = _targetLen - _readLen();
  bool isAtLen = abs(lenDelta) < _lenAccuracy;
  bool cannotStep = (lenDelta < 0 && isTotallyFolded()) || (lenDelta > 0 && isTotallyUnfolded());
  if (isAtLen || cannotStep) {
    return;
  }

  if (lenDelta > 0 && !isUnfolding()) {
    _startUnfolding();
    _folding = false;
  } else if (lenDelta < 0 && !isFolding()) {
    _startFolding();
    _folding = true;
  }  

  _moving = true;
}

void BaseActuator::stop() {
  _moving = false;
  _stop();
}
