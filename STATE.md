# STATE.md — RevenantOps

> **Snapshot vivant** de l'état actuel du projet. Mis à jour **au début ET à la fin de chaque session** (règle CLAUDE.md).
>
> Dernière mise à jour : **2026-04-28** (création GSD planning v4.0)

---

## Sprint actuel

**Sprint 1 — Refonte identité menus** (semaines 1-4 / 28 avril → 25 mai)

**Statut global** : 🟢 Démarrage — planification complète, exécution non commencée

**Dev actif** : Jilani (branche `jilani/sprint-1`) — à créer
**Nourredine** : setup Blender + Audacity en cours (sem 1)

---

## Avancement

### Sprint 1 — semaine 1 (28 avril - 4 mai)

**Jilani** : 0/7 tâches
- [ ] J1.1 UUITheme UDataAsset
- [ ] J1.2 UIHelpers
- [ ] J1.3 Suppression MainMenuWidget
- [ ] J1.4 Refacto TitleScreenWidget
- [ ] J1.5 Import fonts
- [ ] J1.6 DA_Theme_Default
- [ ] J1.7 Update STATE.md

**Nourredine** : 0/4 tâches
- [ ] N1.1 Setup Blender + tuto Donut
- [ ] N1.2 Audacity setup
- [ ] N1.3 Normalisation SFX
- [ ] N1.4 Livraison Audio_Normalized_v1

---

## Bloqueurs

> Aucun bloqueur identifié actuellement.

---

## Feedback Nourredine (livraisons assets)

> Section vivante : Jilani note ici ses retours après import des FBX/WAV de Nourredine.

(Aucune livraison reçue pour le moment)

---

## Snapshot technique (avril 2026)

### Code C++
- **199 fichiers `.uasset`** dans Content
- **Modules actifs** : Engine, InputCore, EnhancedInput, AIModule, NavigationSystem, StateTreeModule, UMG, Slate, Niagara, EngineCameras
- **Build status** : ✅ Clean (Live Coding OK)
- **Warnings critiques** : 0

### Architecture UI (avant refonte sprint 1)
- 12 widgets dans `Source/RevenantOps/UI/`
- 8 héritent `UMenuWidgetBase` (sons hover/click OK)
- 4 n'en héritent pas : MainMenuWidget (à supprimer), RevenantOpsHUD (HUD pas un menu), LeaderboardWidget, MenuCardWidget
- **Doublon mort confirmé** : MainMenuWidget vs TitleScreenWidget — TitleScreenWidget est le seul utilisé
- **Aucune** UWidgetAnimation
- Palettes hardcodées dans chaque widget

### Architecture audio (avant sprint 2)
- **WeaponBase** : FireSound/ReloadSound/EmptySound ✅ joués
- **EnemyBase** : HitSound/DeathSound/AmbientSound 🚧 déclarés mais **jamais joués**
- **MenuMusic** : ✅ joué dans MainMenuGameMode
- **Aucune** musique combat
- **Aucune** SoundClass / SoundMix
- **Aucun** footstep system

### Animations (avant sprint 2-3)
- **Joueur** : montages armes (Pistol/Rifle Fire/Reload/Equip) ✅
- **Joueur** : Sprint/Jump/Crouch/Slide/Dodge ❌ pas d'asset, pas de logique
- **Ennemis** : DeathAnim/HitReactAnim/AttackAnim 🚧 déclarés mais **jamais joués**
- **Stagger ennemis** : code C++ branché (TakeHitZoneDamage), mais montages assets manquants

### Game design (état v3.0)
- ✅ Boucle complète Title → Loadout → Match → GameOver → Leaderboard
- ✅ 11 systèmes implémentés (score, combo, vagues, inventaire RE5, talents, pickups adaptatifs, caisses destructibles, dégâts localisés, leaderboard, character select, niveau Blacksite)
- ❌ Pas de progression / unlock (tous les personnages/talents disponibles dès le début)
- ❌ Difficulty curve plate (vagues hardcoded 7/8/9)
- ❌ Pas de tier visuel fin de partie
- ❌ Pas d'achievements
- ❌ Pas de tutoriel
- 🚧 Comportements ennemis peu différenciés

---

## Démos disponibles

> Démos vidéo enregistrées partagées sur Discord pour validation.

**v3.0** (avril 2026, pré-sprint) :
- Aucune démo officielle enregistrée

**Sprint 1** : à venir fin sem 4 (livrable "menu tour" 60s)

---

## Risques actifs

| Risque | Impact | Mitigation |
|---|---|---|
| Nourredine débutant Blender bloque sur tâches difficiles | Modéré | Tutos vidéo précis fournis ; tâches modulaires courtes ; appels Discord 1x/sem |
| Conflit `.uasset` Git si les deux touchent le même fichier | Élevé | Ownership stricte (Jilani only sur uasset/code) ; annonce Discord obligatoire |
| Sprint 1 dérape (scope creep refonte UI) | Modéré | Limiter à 4 écrans prioritaires ; HUD/GameOver/Inventory en sem 4 si temps |
| Asset Drive partagé non setup | Faible | À mettre en place sem 1 (Google Drive ou Dropbox) |
| Live Coding instable sur changements structurels | Faible | Build complet VS quand ajout de nouvelles classes UE |

---

## Sessions précédentes (historique récent)

### 2026-04-28 — Création GSD planning v4.0
- Audit complet UI / Audio / Animations / Game Design via 3 agents Explore parallèles
- Direction artistique validée : RE Grit
- Timeline validée : 12 semaines (sprint 4 sem prioritaire)
- Création PROJECT.md, REQUIREMENTS.md, ROADMAP.md, STATE.md
- Création .planning/research/ (7 fichiers de référence)
- Plan archivé : `C:\Users\jilan\.claude\plans\pure-whistling-wind.md`

### Sessions antérieures
Voir `CLAUDE.md` section "Etat Phase X" pour l'historique détaillé jusqu'à la phase 19.

---

## Liens rapides

- `PROJECT.md` — vision + équipe + stack
- `REQUIREMENTS.md` — specs détaillées par domaine
- `ROADMAP.md` — 12 semaines de tâches GSD
- `CLAUDE.md` — règles techniques projet
- `primer.md` — état session courante (court)
- `.planning/research/` — recherches game design / refs visuelles / tutos Nourredine
