# Carnet de Bord - RevenantOps

## Session 1 : Initialisation & Caméra Tactique
**Date :** 17/01/2026
**Durée :** 1h30
**Moteur :** Unreal Engine 5.7
**Langage :** C++ & Blueprints

### 🚀 Réalisations
1. **Infrastructure du Projet**
   - Création du projet "RevenantOps" (Template ThirdPerson).
   - Configuration du dépôt Git (GitHub) avec `.gitignore` spécifique Unreal.
   - Résolution des conflits d'initialisation.

2. **Système de Caméra (Over-the-Shoulder)**
   - Transition vers une vue "Tactique/RE4" (50cm).
   - Modification C++ (`RevenantOpsCharacter.cpp`) pour les valeurs par défaut.
   - **Calibration validée :**
     - `TargetArmLength` : **50.0f**
     - `SocketOffset` : **FVector(-146.1f, -127.2f, 14.3f)**
   - *Fix Technique :* Transfert des offsets sur le `CameraBoom` pour les collisions.
   - Nettoyage du Blueprint pour forcer l'héritage C++.

3. **Inputs (Enhanced Input)**
   - Création de l'action `IA_Sprint`.
   - Mapping de la touche **Left Shift**.S