# Carnet de Bord - RevenantOps

Programmeur : Noureddine Houichi    

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


### 🏃‍♂️ Suite Session 1 : Implémentation du Sprint
**Temps additionnel :** 30 min (Total cumulé : 2h00)
**État :** Fonctionnel ✅ (À AMÉLIORER DANS LE FUTUR) 
**Détails Techniques :**
1. **Architecture C++ :**
   - **Header (.h) :** Déclaration de `SprintAction` et des fonctions `StartSprint`/`StopSprint`.
   - **Body (.cpp) :** Logique dynamique modifiant `MaxWalkSpeed` via `GetCharacterMovement()`.
     - Vitesse de base : **500** (au lieu de 600).
     - Vitesse de sprint : **1000**.
   - **Binding :** Utilisation des événements `Triggered` (pendant l'appui) et `Completed` (au relâchement).

2. **Intégration Unreal :**
   - Assignation de l'asset `IA_Sprint` dans le Blueprint `BP_ThirdPersonCharacter` (Variable exposée via UPROPERTY).
   - Test validé : Le personnage accélère et décélère correctement avec la touche Shift.


### 🏃‍♂️ Suite Session 1 : Implémentation du Sprint
**Temps additionnel :** 30 min (Total cumulé : 2h00)
**État :** Fonctionnel ✅ (À AMÉLIORER DANS LE FUTUR)

**Détails Techniques :**
1. **Architecture C++ :**
   - **Header (.h) :** Déclaration de `SprintAction` et des fonctions.
   - **Body (.cpp) :** Logique `MaxWalkSpeed` (500 -> 1000).
   - **Binding :** Triggered / Completed.

2. **Intégration Unreal :**
   - Assignation `IA_Sprint` dans le Blueprint.
   - **Note :** Le changement de vitesse est brusque et les animations de course ne sont pas encore reliées (le personnage "glisse" vite).
   - **TODO :** Ajouter la transition d'animation et lissage de la vitesse (Interpolation).

### Session 2 : Character Locomotion & Animation ( Vitesse & BlendSpace)
**Temps passé :** 1h15

**Détails :**
- Création du BlendSpace `BS_IdleRun` pour gérer la transition Idle/Walk/Run.
- Configuration des axes de vitesse (0 à 1000).
- Mise à jour de l'Animation Blueprint (`ABP_Unarmed`) pour utiliser la variable `Ground Speed`.
- Ajout d'un lissage (Smoothing Time: 0.4s) pour donner de l'inertie au mouvement.
- Test et validation : Le personnage sprinte correctement avec Shift.


📅 Journal de bord - 18 Janvier 2026

Objectif : Créer un système de ramassage (Pickup) pour l'arme BP_LaserRifle_Pickup.

2h00

Réussites :

Mise en place de l'événement BeginOverlap avec le Player Character.

Utilisation de Attach Actor To Component pour lier l'arme au Mesh du joueur via un WeaponSocket.

Correction du bug de "vol infini" en désactivant la collision de l'acteur via Set Actor Enable Collision.

Problèmes rencontrés :

Difficultés d'alignement des axes de rotation (Relative Rotation) du Socket.

L'arme pointait initialement vers l'arrière ou le sol.

Solution trouvée : Pivotement du Socket à 180° sur l'axe Z dans le Squelette SK_Mannequin. ( Pas tout a fais ca )