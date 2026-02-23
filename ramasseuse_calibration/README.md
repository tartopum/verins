# Calibration de la ramasseuse à pommes

Les potentiomètres de cabine et des vérins peuvent se dérégler au cours du temps,
donc on veut pouvoir déterminer facilement quelles sont leurs valeurs aux positions
extrêmes.

Pour cela, il faut téléverser le programme `ramasseuse_calibration.ino` sur l'Arduino
**après l'avoir configuré**.

Le programme dispose de quatre modes.

## Lancer la calibration

### Calibration du potentiomètre de cabine

Pour activer ce mode, mettre la valeur de la variable `mode` à `CALIBRATION_POTAR_CABINE`.

Sera alors affichée sur le moniteur Série (en haut à droite du logiciel Arduino)
la valeur lue sur le potentiomètre.

Il suffit de placer ce dernier aux deux positions extrêmes et de noter les valeurs
correspondantes.

### Calibration des potentiomètres des vérins

Pour activer ces modes, mettre la valeur de la variable `mode` à `CALIBRATION_VERIN_G`
ou à `CALIBRATION_VERIN_D`.

Le potentiomètre de cabine sert à commander le vérin sélectionné (défini par le
mode) :

* Quand il est vers le milieu, le vérin est arrêté
* Quand il est vers la position extrême basse, le vérin se replie
* Quand il est vers la position extrême haute, le vérin se déplie

Le "vers la position" est dû au fait qu'on ne connait pas les valeurs extrêmes
exactes du potentiomètre de cabine et donc qu'on considère des intervalles larges.

Commander le vérin pour le placer à ses positions extrêmes et noter les valeurs
correspondantes affichées dans le moniteur Série (en haut à droite du logiciel Arduino).

**ATTENTION** si vous dépliez ou repliez trop le vérin, le moteur risque de forcer et le
fusible de sauter. Le programme de calibration ne détecte pas cela et n'émettra pas de
signal sonore d'alerte.

### Calibration inactive

Par défaut, le programme est dans le mode `CALIBRATION_INACTIVE`, qui ne fait rien.

## Mettre à jour le programme principal

Le programme principal se situe dans `ramasseuse/ramasseuse.ino`. À partir des
valeurs notées pendant la calibration, mettre à jour les constantes suivantes :

* `POTAR_MIN_VAL` : la valeur minimum du potentiomètre de cabine
* `POTAR_MAX_VAL` : la valeur maximum du potentiomètre de cabine
* `VERIN_G_MIN_VAL` : la valeur du potentiomètre de position du vérin gauche quand il est replié au maximum
* `VERIN_G_MAX_VAL` : la valeur du potentiomètre de position du vérin gauche quand il est déplié au maximum
* `VERIN_D_MIN_VAL` : la valeur du potentiomètre de position du vérin droit quand il est replié au maximum
* `VERIN_D_MAX_VAL` : la valeur du potentiomètre de position du vérin droit quand il est déplié au maximum

Téléverser le programme mis à jour.
