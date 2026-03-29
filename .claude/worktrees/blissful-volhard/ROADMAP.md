# ROADMAP - RevenantOps
### Third-Person Shooter | Unreal Engine 5.7 | C++

---

## PHASE 1 : FONDATIONS & CORE GAMEPLAY

### 1.1 - Locomotion & Mouvement (Polish)
- [ ] Interpolation fluide du sprint (lerp entre 500 et 1000 au lieu du changement brut)
- [ ] Système de crouch (accroupi) avec transition d'animation
- [ ] Slide (glissade) : sprint + crouch = slide avec momentum
- [ ] Mantle/Vault : escalade automatique d'obstacles bas et moyens
- [ ] Système de couverture (cover system) - appui contre les murs
- [ ] Roll/Dodge (esquive avec i-frames)
- [ ] Ladder climbing (échelles)
- [ ] Swimming (nage) si applicable au level design
- [ ] Stamina system lié au sprint, dodge, et actions physiques

### 1.2 - Caméra (Polish)
- [ ] Transition fluide entre caméra exploration et caméra visée (ADS)
- [ ] Camera shake contextuel (tirs, explosions, chute)
- [ ] Dead zone ajustable pour le stick droit
- [ ] Sensibilité séparée pour hip-fire et ADS
- [ ] Système de caméra cinématique pour cutscenes
- [ ] FOV dynamique (sprint = FOV plus large, ADS = FOV réduit)
- [ ] Camera collision améliorée (éviter les clips dans les murs)

### 1.3 - Enhanced Input System (Complétion)
- [ ] Rebinding des touches en jeu
- [ ] Support complet manette (Xbox, PlayStation, Steam Deck)
- [ ] Vibration/Haptics contextuels (manette)
- [ ] Support clavier/souris avec icônes dynamiques
- [ ] Profils d'input sauvegardables
- [ ] Dead zones configurables

---

## PHASE 2 : SYSTÈME DE COMBAT (ÉVOLUTION)

### 2.1 - Combat Melee (Amélioration du système existant)
- [ ] Ajout de la mécanique de parry/block (parade/blocage)
- [ ] Counter-attack après un parry réussi
- [ ] Finishers/Executions sur ennemis stun
- [ ] Weapon-switching pour armes de mêlée (couteau, matraque, etc.)
- [ ] Lock-on target (verrouillage de cible)
- [ ] Hitstop (micro-freeze à l'impact pour le "game feel")
- [ ] Stagger system (accumulation de stun)
- [ ] Système de combo amélioré avec branches (light/heavy attack tree)
- [ ] Attaques aériennes (jump attack, plunging attack)
- [ ] Attaques environnementales (pousser ennemi du bord, utiliser objets)

### 2.2 - Système de Tir (NOUVEAU - Coeur du TPS)
- [ ] Base shooting : hitscan et/ou projectile system
- [ ] Aiming Down Sights (ADS) avec transition caméra
- [ ] Recoil system avec pattern par arme
- [ ] Spread/Bloom (dispersion des tirs)
- [ ] Système de rechargement (reload) avec animations
- [ ] Headshot multiplier et zones de dégâts (hitbox par limb)
- [ ] Bullet penetration (traversée de matériaux)
- [ ] Ricochet (optionnel, selon direction artistique)
- [ ] Système de munitions (ammo types, ammo management)
- [ ] Tir depuis la couverture (blind fire, lean peek)
- [ ] Bullet time / slow-mo ability (signature gameplay ?)

### 2.3 - Arsenal & Armes
- [ ] Catégories d'armes : Pistol, SMG, Assault Rifle, Shotgun, Sniper, LMG
- [ ] Arme secondaire (sidearm) toujours disponible
- [ ] Grenades (frag, flash, smoke, incendiary)
- [ ] Système d'attachments (silenceur, scope, laser, grip)
- [ ] Arme de mêlée équipable (couteau, machette)
- [ ] Weapon wheel ou quick-swap system
- [ ] Animations uniques par arme (idle, reload, ADS, sprint)
- [ ] Weapon sway et bobbing
- [ ] Système de surchauffe pour certaines armes (laser rifle existant)
- [ ] Armes ramassables au sol (améliorer le pickup existant)

### 2.4 - Système de Dégâts (Amélioration)
- [ ] Damage types : Balistique, Explosif, Feu, Électrique, Melee
- [ ] Résistances/Faiblesses par type d'ennemi
- [ ] Armor system (réduction de dégâts, armure destructible)
- [ ] Damage numbers flottants (optionnel, configurable)
- [ ] Kill feed / hit markers visuels
- [ ] Friendly fire configurable

---

## PHASE 3 : INTELLIGENCE ARTIFICIELLE

### 3.1 - IA Ennemie (Évolution du StateTree existant)
- [ ] Behavior profiles : Aggressive, Defensif, Flanker, Sniper, Rusher
- [ ] Squad AI : coordination entre ennemis (flanking, suppression)
- [ ] Communication entre IA (alerte, appel de renforts)
- [ ] IA de couverture : ennemis se mettent à couvert, blind fire, peek
- [ ] Réaction aux grenades (fuite, kick back)
- [ ] Patrol routes dynamiques avec points d'intérêt
- [ ] Investigation de bruits/impacts (système de perception)
- [ ] Sens multiples : vue (cone), ouïe (rayon), toucher
- [ ] Réaction au danger améliorée (dodge, roll, retreat)
- [ ] Boss AI avec phases et patterns uniques
- [ ] IA qui utilise l'environnement (retourner tables, fermer portes)

### 3.2 - Types d'Ennemis
- [ ] Grunt/Fodder : ennemi de base, faible mais nombreux
- [ ] Heavy : blindé, lent, gros dégâts
- [ ] Flanker : rapide, contourne le joueur
- [ ] Sniper : longue distance, laser de visée
- [ ] Grenadier : lance des explosifs
- [ ] Shield bearer : bouclier frontal, vulnérable de dos
- [ ] Melee Rusher : charge le joueur (améliorer l'ennemi existant)
- [ ] Drone/Tech : ennemi volant ou tourelle
- [ ] Medic : soigne les autres ennemis
- [ ] Mini-boss : version renforcée avec mécaniques spéciales
- [ ] Boss : combats scriptés avec phases

### 3.3 - Spawning & Encounter Design
- [ ] Wave system (vagues d'ennemis) - améliorer le CombatEnemySpawner
- [ ] Encounter volumes avec difficulté configurable
- [ ] Dynamic difficulty adjustment (DDA)
- [ ] Spawn points intelligents (hors champ de vision du joueur)
- [ ] Reinforcement triggers (renforts si le joueur traîne)
- [ ] Arena lock-in (portes qui se ferment pendant un combat)

---

## PHASE 4 : LEVEL DESIGN & MONDE

### 4.1 - Environnements
- [ ] Hub central / Base d'opérations
- [ ] Niveau urbain (ville, rues, bâtiments)
- [ ] Niveau industriel (usine, entrepôt)
- [ ] Niveau souterrain (tunnels, bunker, laboratoire)
- [ ] Niveau extérieur (forêt, montagne, ruines)
- [ ] Niveau vertical (gratte-ciel, tour, ascenseur)
- [ ] Arène de boss dédiée par boss

### 4.2 - Level Mechanics
- [ ] Portes interactives (ouvrir, forcer, hacker)
- [ ] Ascenseurs et plateformes mobiles
- [ ] Objets destructibles (murs, vitres, barils explosifs)
- [ ] Pièges environnementaux (mines, tourelles, lasers)
- [ ] Éclairage dynamique (casser des lumières = obscurité)
- [ ] Secrets et zones cachées
- [ ] Points de non-retour avec autosave
- [ ] Raccourcis déverrouillables (type Soulslike/RE)
- [ ] Éléments interactifs : ordinateurs, terminaux, leviers

### 4.3 - Système de Navigation
- [ ] Minimap ou boussole en jeu
- [ ] Waypoint system
- [ ] Marqueurs d'objectifs
- [ ] Carte du monde accessible via menu

---

## PHASE 5 : PROGRESSION & SYSTÈMES RPG

### 5.1 - Personnage
- [ ] Système d'XP et de niveaux
- [ ] Skill tree (arbre de compétences) : Combat, Furtivité, Tech
- [ ] Abilities actives débloquables (ralentir le temps, scan, bouclier)
- [ ] Perks passifs (plus de dégâts, plus de HP, reload plus rapide)
- [ ] Customisation visuelle (skins, armures, accessoires)

### 5.2 - Inventaire & Loot
- [ ] Système d'inventaire (grille ou liste)
- [ ] Loot drops sur ennemis et dans l'environnement
- [ ] Rareté des objets (Common, Rare, Epic, Legendary)
- [ ] Crafting basique (munitions spéciales, kits de soin)
- [ ] Shop/Vendor pour acheter/vendre
- [ ] Ressources collectables (matériaux, crédits)

### 5.3 - Mission System
- [ ] Mission principale (story quests)
- [ ] Missions secondaires (side quests)
- [ ] Objectifs optionnels avec récompenses bonus
- [ ] Journal de missions consultable
- [ ] Système de choix avec conséquences (optionnel)
- [ ] Mission briefing et debriefing

---

## PHASE 6 : NARRATION & UNIVERS

### 6.1 - Story
- [ ] Définir le lore de "RevenantOps" (qui sont les Revenants ?)
- [ ] Scénario principal avec actes/chapitres
- [ ] Personnages PNJ alliés avec dialogues
- [ ] Antagoniste principal et ses motivations
- [ ] Twists narratifs et moments forts
- [ ] Cinématiques in-engine (Sequencer UE5)

### 6.2 - Système de Dialogues
- [ ] Dialogue tree system
- [ ] Barks contextuels (lignes courtes pendant le gameplay)
- [ ] Radio comms (voix dans l'oreillette pendant les missions)
- [ ] Sous-titres avec localisation
- [ ] Logs/Documents trouvables dans le monde (lore building)

### 6.3 - Mise en scène
- [ ] Cutscenes scriptées (UE5 Sequencer)
- [ ] Transitions gameplay-cutscene fluides
- [ ] Set pieces interactifs (moments scriptés jouables)
- [ ] Moments de calme (exploration, dialogues entre combats)

---

## PHASE 7 : INTERFACE UTILISATEUR (UI/UX)

### 7.1 - HUD In-Game
- [ ] Barre de vie joueur (améliorer le CombatLifeBar existant)
- [ ] Indicateur d'armure/bouclier
- [ ] Crosshair dynamique (taille selon spread, couleur selon cible)
- [ ] Compteur de munitions + type de munition
- [ ] Indicateur de grenade
- [ ] Indicateur de direction des dégâts reçus
- [ ] Indicateur d'objectif actif
- [ ] Minimap ou boussole
- [ ] Barres de vie ennemis (pour boss et elites)
- [ ] Indicateurs d'interaction contextuelle (touche E pour interagir)
- [ ] Écran rouge/vignette quand HP bas

### 7.2 - Menus
- [ ] Main Menu (New Game, Continue, Settings, Quit)
- [ ] Pause Menu
- [ ] Settings Menu (Audio, Video, Gameplay, Controls, Accessibilité)
- [ ] Écran de chargement avec tips
- [ ] Écran de mort / Game Over avec options
- [ ] Menu d'inventaire
- [ ] Menu de compétences
- [ ] Menu de carte

### 7.3 - Feedback Joueur
- [ ] Hit markers visuels et sonores
- [ ] Kill confirmation
- [ ] Notifications de loot/pickup
- [ ] Tutorials contextuels (premières heures)
- [ ] Prompts d'action contextuels

---

## PHASE 8 : AUDIO

### 8.1 - Sound Design
- [ ] Sons d'armes à feu (tir, reload, dry fire, switch)
- [ ] Sons de mêlée (impact, swing, parry)
- [ ] Sons de pas contextuels (béton, métal, terre, eau)
- [ ] Sons d'ambiance par environnement
- [ ] Sons d'UI (navigation menu, confirmation, erreur)
- [ ] Atténuation et occlusion audio (murs, distance)
- [ ] Reverb par zone (intérieur, extérieur, grotte)

### 8.2 - Musique
- [ ] Thème principal
- [ ] Musique de combat (dynamique, layered)
- [ ] Musique d'exploration (calme, atmosphérique)
- [ ] Musique de boss (unique par boss)
- [ ] Stingers (courts morceaux sur événements : mort, victoire, découverte)
- [ ] Système de musique adaptative (MetaSounds UE5)

### 8.3 - Voix
- [ ] Voice acting personnage principal (efforts, barks)
- [ ] Voice acting PNJ et ennemis
- [ ] Radio comms
- [ ] Lignes d'ennemis (détection, combat, mort)

---

## PHASE 9 : EFFETS VISUELS (VFX)

### 9.1 - VFX Combat
- [ ] Muzzle flash par type d'arme
- [ ] Impact particles (béton, métal, chair, bois)
- [ ] Traceurs de balles
- [ ] Blood splatter (sang)
- [ ] Explosions (grenades, barils)
- [ ] Slash trails pour melee
- [ ] Effets de stun/electrocution

### 9.2 - VFX Environnement
- [ ] Particules de poussière et débris
- [ ] Fumée et brouillard volumétrique
- [ ] Feu et flammes (Niagara)
- [ ] Éclairs et effets électriques
- [ ] Pluie et météo dynamique
- [ ] Destruction (Chaos Destruction UE5)

### 9.3 - VFX UI/Feedback
- [ ] Vignette de dégâts (écran rouge)
- [ ] Motion blur sélectif
- [ ] Screen shake
- [ ] Post-process : bloom, lens flare
- [ ] Effet de slow-motion (si bullet time)
- [ ] Death screen effect

---

## PHASE 10 : SYSTÈMES TECHNIQUES

### 10.1 - Save System
- [ ] Autosave aux checkpoints
- [ ] Save/Load manuel (slots multiples)
- [ ] Sauvegarde de progression (quêtes, inventaire, stats)
- [ ] Sauvegarde de position dans le niveau
- [ ] Cloud save (optionnel)

### 10.2 - Optimisation & Performance
- [ ] Profiling CPU/GPU (Unreal Insights)
- [ ] LOD system pour les meshes
- [ ] Occlusion culling
- [ ] Streaming de niveaux (Level Streaming)
- [ ] Object pooling pour projectiles et VFX
- [ ] Texture streaming
- [ ] Nanite pour les meshes haute densité
- [ ] Lumen pour l'éclairage global (ou baked selon performance)
- [ ] Budget mémoire et optimisation assets

### 10.3 - Settings & Accessibilité
- [ ] Options graphiques (Low, Medium, High, Ultra, Custom)
- [ ] Résolution et mode fenêtré
- [ ] V-Sync, Frame Rate cap
- [ ] Sous-titres avec taille ajustable
- [ ] Daltonisme (filtres)
- [ ] Aim assist (configurable)
- [ ] Difficulté ajustable (Story, Normal, Hard, Revenant)
- [ ] Remapping complet des contrôles

---

## PHASE 11 : POLISH & GAME FEEL

### 11.1 - Animations
- [ ] Motion matching ou advanced locomotion (UE5.7 features)
- [ ] IK pieds (foot placement)
- [ ] IK mains (weapon handling)
- [ ] Animation layers (tirer en courant, recharger en marchant)
- [ ] Procédural animations (recoil, breathing)
- [ ] Facial animations pour dialogues
- [ ] Ragdoll to get-up transitions
- [ ] Animation de mort variées (headshot, explosion, melee)

### 11.2 - Juice & Game Feel
- [ ] Hitstop sur les impacts importants
- [ ] Screen shake calibré (pas trop, pas trop peu)
- [ ] Rumble manette contextuel
- [ ] Time dilation sur kills importants
- [ ] Slow-mo sur le dernier ennemi d'une vague
- [ ] Satisfying reload animations
- [ ] Poids et momentum des mouvements
- [ ] Sound design impactant (basses sur explosions)

---

## PHASE 12 : CONTENU & REJOUABILITÉ

### 12.1 - Modes de Jeu
- [ ] Campagne (mode principal, story-driven)
- [ ] New Game+ (recommencer avec équipement)
- [ ] Horde/Survival mode (vagues infinies, leaderboard)
- [ ] Challenge rooms (arènes avec objectifs)
- [ ] Speedrun mode (timer, classement)

### 12.2 - Collectibles & Achievements
- [ ] Trophées/Succès
- [ ] Collectibles cachés dans les niveaux
- [ ] Statistiques de jeu (kills, précision, temps de jeu)
- [ ] Galerie de concept art débloquable
- [ ] Model viewer

### 12.3 - Post-Launch (Vision Long Terme)
- [ ] DLC / Chapitres additionnels
- [ ] Mode coop (2 joueurs)
- [ ] PvP arena (optionnel)
- [ ] Workshop/Mod support
- [ ] Seasonal content

---

## PHASE 13 : PUBLICATION

### 13.1 - Build & Distribution
- [ ] Packaging UE5 optimisé
- [ ] Steam integration (Steamworks SDK)
- [ ] Epic Games Store (si applicable)
- [ ] Console ports (PlayStation, Xbox) si visé
- [ ] Anti-cheat (si multijoueur)

### 13.2 - QA & Testing
- [ ] Playtesting interne (chaque feature)
- [ ] Beta testing avec groupe fermé
- [ ] Bug tracking et résolution
- [ ] Performance testing sur configurations variées
- [ ] Compatibility testing (GPU, CPU, OS)

### 13.3 - Marketing & Communauté
- [ ] Page Steam avec trailer
- [ ] Presskit et screenshots
- [ ] Community Discord
- [ ] Dev blog / Devlog vidéo
- [ ] Demo jouable

---

> **Note** : Cette roadmap est un document vivant. Les priorités peuvent être réorganisées
> selon la direction artistique et les décisions de game design. Les variantes existantes
> (Platforming, SideScrolling) peuvent être intégrées comme des sections/modes du jeu final
> ou conservées comme prototypes séparés.
