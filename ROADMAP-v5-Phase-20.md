# ROADMAP RevenantOps v5 — Récap audit + Phase 20 (2026-05-21)

> Cette roadmap **remplace** la planification GSD théorique du `ROADMAP.md` (qui ne reflète plus l'état réel). Elle a été établie par audit interactif de chaque feature avec Noureddine. Lecture obligatoire pour Jilani avant de démarrer la phase 20.

## Contexte
On fait le point sur l'état réel du projet (audit confirmé par Noureddine, pas extrapolé des fichiers d'état) avant de poser la prochaine roadmap. Objectif : finir une démo solo polie sans deadline fixe ("on polit jusqu'à être fier"), puis basculer sur le coop online en post-démo. Le découpage des phases est conçu pour que Noureddine et Jilani puissent bosser en **parallèle sans toucher aux mêmes fichiers ni aux mêmes assets binaires** (pas de merge conflict, pas de doublon de travail).

**Cible démo confirmée** : solo, 1 seul level, win condition = kill 150 ennemis (drip feed), pas de coop, pas de tutoriel pour l'instant, pas de date fixe.

**Note audio — enregistrement maison** : Noureddine a un micro correct et de l'inspiration. Pour les sons player (cris, douleur, mort, effort) et les sons zombies (growl, attaque, mort), on garde l'option de **les enregistrer nous-mêmes au cas par cas** plutôt que d'aller chercher du CC0. Décision prise son par son : "j'enregistre" ou "je trouve en CC0". Si on enregistre : raw → Audacity (clean, EQ, comp léger) → WAV → import UE5 → SoundCue avec attenuation/modulation.

**Note charge de travail** : aucune pression, on avance à notre rythme — mais on essaye de garder une répartition à peu près équilibrée entre N et J à chaque phase, en quantité ET en variété (pas un dev qui fait que du C++ pendant que l'autre fait que des matériaux).

---

## État réel — Audit confirmé (2026-05-21)

### Légende
- ✅ Fait et OK
- 🟡 À améliorer maintenant (avant démo)
- 🔵 À améliorer plus tard (post-démo ou polish tardif)
- ❌ Pas encore fait
- 🗑️ À retirer

### Core gameplay
- ✅ Combat de base (tir, dégâts, kill, score)
- ✅ Spawn ennemis — **système CHANGÉ** : 150 ennemis drip feed (plus de 3 waves)
- 🔵 Système d'armes (6 armes, switch, reload, recoil/spread)
- 🔵 Système d'ennemis (5 types zombies, IA)

### UI / HUD
- ✅ HUD principal (Health, Stamina, Score, Timer, Ammo, KillFeed)
- ✅ Inventaire RE5 (Tab, 9 slots, ZQSD, E)
- 🔵 Viseur dynamique (4 traits)
- 🔵 Menus (Title, Loadout, Character Select, GameOver, Leaderboard)

### Level / Environnement
- ❌ Level démo (rien fait, à construire)
- ❌ Éclairage / ambiance
- ❌ NavMesh + spawn points
- ❌ Placement pickups + caisses dans le level

### Animations
- 🔵 Locomotion joueur
- 🔵 Anims armes (Fire, Reload, Equip, Holster)
- 🔵 Hand IK (main gauche sur arme)
- 🔵 Anims ennemis (5 types zombies)

### Audio
- ✅ Sons armes (Silver pack)
- ❌ Sons player (damage / death / footsteps / jump-land) — requiert UPROPERTY C++
- 🔵 Sons ennemis
- 🔵 Sons UI / pickups / musique / ambiance

### Systèmes spécifiques
- ✅ Pickups (Health / AmmoBonus / TimeBonus + IPickupInterface)
- 🟡 Caisses destructibles (polish drops + état endommagé)
- 🔵 Talents (UTalentDefinition)
- 🔵 Character Select (portrait 3D + grille + talents)

### VFX & feedback combat
- ✅ Camera shake (UCS_WeaponFire + UCS_TakeDamage)
- 🗑️ Hit flash matériau ennemi (à retirer du code + du matériau)
- 🟡 Hit Marker + Damage Direction + Kill Notification
- 🔵 VFX armes (NS_MuzzleFlash, NS_Impact_Surface, NS_Impact_Blood)

### Architecture / Code
- ✅ Stabilité build/runtime (compile clean, crash ~Character fixé)
- ✅ DataTables (DT_WeaponStats + DT_EnemyStats)
- ✅ Save/Load Leaderboard (FScoreEntry)
- ✅ Game flow complet (Title → Loadout → Match → GameOver → Leaderboard → Replay)

### Vision démo
- Win condition : kill 150 ennemis
- Mode : solo (coop = post-démo)
- Tutoriel : pas encore fait
- 1 seul level (en dernier)
- Pas de deadline

---

## Roadmap découpée

### Phase 20 — Polish combat solo (N + J en parallèle, ZÉRO conflit, charge équilibrée)

**Démarre depuis main à jour. Deux branches indépendantes.**

#### Lot Noureddine — Audio player + Feedback HUD
Branche : `noureddine/phase-20-feedback`

| Tâche | Fichiers touchés | Note |
|---|---|---|
| Sons player : enregistrement maison (douleur, mort, effort, jump, land) | Studio improvisé → Audacity → `Content/Mercenaires/Audio/Player/*.wav` + SoundCues | Activité créative |
| Sons player : UPROPERTY + branchement C++ | `RevenantOpsCharacter.h/.cpp` (USoundBase* DamageSound, DeathSound, FootstepSound, JumpSound, LandSound) | Branchement dans HandleTakeDamage / HandleDeath / AnimNotify footstep |
| Hit Marker polish (timing + animation) | `RevenantOpsHUD.h/.cpp` + `WBP_Mercenaires_HUD` (widget `HitMarkerImage`) | Tweak fade-out, son d'impact léger optionnel |
| Damage Direction Indicator polish | `RevenantOpsHUD.h/.cpp` + `WBP_Mercenaires_HUD` (widget `DamageDirectionImage`) | Interp angle + fade |
| Kill Notification polish (font/anim/son) | `RevenantOpsHUD.h/.cpp` + `WBP_Mercenaires_HUD` (widget `KillNotificationText`) | Texte + anim apparition |

#### Lot Jilani — Caisses + nettoyage hit flash
Branche : `jilani/phase-20-caisses`

| Tâche | Fichiers touchés | Note |
|---|---|---|
| Caisses : valider modes loot (Independent / PickOne) + AmmoTypeFilter | `DestructibleObject.h/.cpp` + `FCrateLootEntry` (struct) | Test que chaque mode donne le bon drop |
| Caisses : état endommagé (matériau + threshold) | Matériau `M_CrateDamaged` (ou MID instancié) + `DestructibleObject.cpp` (DamagedThreshold) | Activité créative (visuel) |
| Caisses : peupler les BPs (`BP_LootCrate_Ammo`, `_Health`, `_Weapon`...) | BPs caisses + valeurs `LootTable` + chances | Setup pondéré |
| Retirer hit flash matériau ennemi | `EnemyBase.h/.cpp` (supprimer `HitFlashMaterials`, `HitFlashTimer`, MID, appel dans `OnTakeDamage`) + matériaux zombies (param scalaire à enlever si présent) | Nettoyage |

#### Règles d'ownership pour cette phase
- N ne touche **pas** à `DestructibleObject.*`, BPs caisses, matériaux caisses, `EnemyBase.*`, matériaux zombies
- J ne touche **pas** à `RevenantOpsCharacter.*`, `RevenantOpsHUD.*`, `WBP_Mercenaires_HUD`, Cues player
- Personne ne touche à `ABP_Mercenaire` (réservé phase 22)
- Personne ne touche au `.umap` (réservé phase 23)

#### Équilibre vérifié
- N : 1 grosse tâche créative (record audio) + 1 grosse tâche C++ (UPROPERTY/branch sons) + 3 polish UI moyens
- J : 1 grosse tâche C++/setup (caisses : 3 sous-tâches) + 1 nettoyage moyen (retirer hit flash) + 1 activité créative (matériau endommagé)
- Variété équilibrée : chacun a du C++, du visuel/créatif, et du setup BP/asset

### Phase 21 — Tutoriel + audio + nettoyage (parallèle N + J, indépendants)

Tâches sans dépendances, à répartir équilibré au démarrage de la phase :

- **Tutoriel** : `WBP_Tutorial` overlay au lancement de match (touches, objectif "kill 150 zombies"). WBP isolé, peut être pris par n'importe qui.
- **Sons ennemis** : option **enregistrement maison** (growls, attaques, mort, hits par zombie type) — activité créative équivalente au record player. Cues + branchement dans BPs zombies. Décision son par son : record ou CC0.
- **Sons UI / pickups / ambiance** : Cues + branchement WBP menus + pickups + ambiance level. Option record pour certains éléments (UI clicks, pickup beep, etc.).

Suggestion équilibrée (à valider au moment) :
- N : Tutoriel + Sons UI/pickups/ambiance
- J : Sons ennemis (record possible)

Aucun fichier commun garanti : Tutoriel = nouveau WBP isolé, Sons UI = WBP menus existants (différents de ceux touchés en P20), Sons ennemis = BPs zombies + Cues `Audio/Enemies/`.

### Phase 22 — Polish "plus tard" sélectionnés

Découpage proposé (toujours sans dépendances) :
- **N** : Viseur dynamique polish + Menus polish (Title/Loadout/GameOver) — UI focus, fichiers WBP différents par menu
- **J** : Polish anims (Locomotion → armes → Hand IK) — **ABP_Mercenaire = J only durant cette phase**, N ne touche pas
- À répartir en plus selon dispo : VFX armes polish, Talents, Character Select

Règle : tant qu'un dev a `ABP_Mercenaire` ouvert, l'autre ne le touche pas (annonce Discord/WhatsApp obligatoire).

### Phase 23 — Level démo (UN SEUL dev à la fois)

`.umap` = asset binaire non-mergeable → **un seul dev actif sur le level**.

Travail :
- Construction layout arena (1 seul level)
- Lighting / ambiance (PointLights + skylight + post-process)
- NavMeshBoundsVolume couvrant toute la jouabilité
- Spawn points (TargetPoints) pour les 150 ennemis (drip feed)
- Placement pickups (Health / AmmoBonus / TimeBonus) + caisses
- Décor + props

Propriété level à décider en début de phase. L'autre dev peut bosser sur post-démo en parallèle (Talents, Character Select, VFX armes polish).

### Phase 24+ — Post-démo

- **Coop online** (gros chantier : replication, RPCs, session/lobby, validation de tout le code existant)
- Système d'armes polish (recoil, spread, switch fluide)
- Système d'ennemis polish (IA + 5 types comportement)
- Talents finalize + UI
- Character Select polish
- VFX armes polish

---

## Fichiers critiques à modifier (phase 20)

### Côté Noureddine
- `Source/RevenantOps/RevenantOpsCharacter.h` — déclarer UPROPERTY `DamageSound`, `DeathSound`, `FootstepSound`, `JumpSound`, `LandSound` (USoundBase*)
- `Source/RevenantOps/RevenantOpsCharacter.cpp` — brancher dans `HandleTakeDamage`, `HandleDeath`, anim notify footstep (à connecter via AnimNotify_PlaySound déjà dispo)
- `Source/RevenantOps/UI/RevenantOpsHUD.h/.cpp` — fonctions `ShowHitMarker`, `ShowDamageDirection`, `ShowKillNotification` déjà présentes : polish timing + visuels via les widgets bindés
- `Content/Mercenaires/UI/WBP_Mercenaires_HUD` — widgets `HitMarkerImage`, `DamageDirectionImage`, `KillNotificationText`
- `Content/Mercenaires/Audio/Player/` — fichiers WAV enregistrés + SoundCues à créer

### Côté Jilani
- `Source/RevenantOps/DestructibleObject.h/.cpp` — valider `ELootMode::Independent` / `ELootMode::PickOne`, `AmmoTypeFilter` + état endommagé (DamagedThreshold)
- `FCrateLootEntry` (struct) — config DropChance + Weight + AmmoTypeFilter
- BPs caisses (`BP_LootCrate_Ammo`, `BP_LootCrate_Health`, `BP_LootCrate_Weapon` etc.) — peupler les LootTable des BPs
- Matériau `M_CrateDamaged` (ou MID instancié) — visuel état endommagé
- `Source/RevenantOps/EnemyBase.h/.cpp` — supprimer `HitFlashMaterials`, `HitFlashTimer`, MID + appel dans `OnTakeDamage`
- Matériaux zombies — retirer le param scalaire `HitFlash` s'il a été ajouté

---

## Vérification phase 20

- Compile MSVC sans erreur ni warning critique
- PIE : sons player jouent (prendre dégâts, mourir, marcher, sauter, atterrir)
- PIE : Hit Marker apparaît au tir confirmé, Damage Direction pointe vers la source des dégâts, Kill Notification visible et lisible
- PIE : aucun flash blanc sur zombies touchés (vérifier code + matériau)
- PIE : caisse intact → endommagée (visuel diff) → cassée → drops corrects selon mode (Independent vs PickOne, filtrés par armes du joueur)
- Git : 2 PRs séparés, merge dans main sans aucun conflit (preuve du découpage propre)

---

## Règles de collaboration (rappel)

- Branche `main` = stable, JAMAIS de commit direct
- Préfixe commit `[N]` ou `[J]`
- Avant toute modif d'asset binaire (`.uasset`, `.umap`, ABP, WBP, BP) : annonce à l'autre dev
- `git pull origin main` en début de session, après chaque merge
- Si conflit imprévu : stop immédiat, on en parle avant d'écraser

---

## Workflow récupération pour Jilani

1. `git fetch`
2. `git checkout main` puis `git merge origin/Nrd` (ou attendre PR Nrd → main puis `git pull origin main`)
3. Lire ce fichier `ROADMAP-v5-Phase-20.md` + nouveau `CLAUDE.md` (section "Etat Phase 20")
4. Créer sa branche : `git checkout -b jilani/phase-20-caisses` depuis main à jour
5. Annoncer à N sur Discord/WhatsApp qu'il démarre le lot caisses + hit flash
6. Commencer le travail (lot Jilani ci-dessus)

---

## Démarrage immédiat (côté Noureddine)

1. Enregistrer les 5 sons player (douleur/mort/effort/jump/land) — micro + Audacity
2. Créer branche `noureddine/phase-20-feedback`
3. Import WAV dans `Content/Mercenaires/Audio/Player/` + SoundCues
4. Ajouter 5 UPROPERTY USoundBase* dans `RevenantOpsCharacter.h`
5. Brancher dans `HandleTakeDamage` / `HandleDeath` / footstep AnimNotify
6. Live Coding (Ctrl+Alt+F11) → test PIE
7. Polish HUD widgets (Hit Marker / Damage Direction / Kill Notification)
8. PR `noureddine/phase-20-feedback` → review par J → merge dans main
