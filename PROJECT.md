# PROJECT.md — RevenantOps

> **TPS Mode Mercenaires** inspiré de Resident Evil 5 — Unreal Engine 5.7, C++

---

## Vision

RevenantOps est un **TPS arcade orienté score** avec une boucle de gameplay courte et intense :
- Vagues d'ennemis (zombies militarisés) sur arène fermée
- Timer + score + combo (style RE5 Mercenaires)
- 5 minutes par run, multi-rejouable
- Identité visuelle : **opération militaire abandonnée** — sombre, sale, accents rouge sang + or terni

**Ce que le jeu N'EST PAS** :
- Pas un battle royale, pas un MMO, pas un RPG narratif
- Pas de campagne scénarisée — focus arcade pur
- Pas de multijoueur (v4.0 single-player local uniquement)

---

## Équipe

| Membre | Rôle | Setup | Responsabilités |
|---|---|---|---|
| **Jilani** | Dev gameplay / UI / intégration | Machine GPU + UE5 + VS | Code C++, Blueprints, level design, intégration assets, PIE testing |
| **Nourredine** | Asset producer | Pas de GPU — Blender + Audacity | Modélisation, animation Blender, audio editing externe, export FBX/WAV |
| **Claude (AI)** | Game designer + tech lead | — | Conception game design, supervision technique, planification GSD, code review |

Communication : Discord (chat + voice 1x/sem)
Git : GitHub repo `RevenantOps`, branches `jilani/sprint-N` et `noureddine/sprint-N`

---

## Stack technique

- **Moteur** : Unreal Engine 5.7
- **Langage** : C++ (logique gameplay) + Blueprint (config / intégration assets)
- **Build system** : UnrealBuildTool, Visual Studio 2026 (MSVC)
- **Modules activés** : `Engine`, `InputCore`, `EnhancedInput`, `AIModule`, `NavigationSystem`, `StateTreeModule`, `UMG`, `Slate`, `Niagara`, `EngineCameras`
- **Plugins** : Enhanced Input, MCP TCP (port 12029) pour scripting Python éditeur
- **Targets** : Win64 Editor (Development) — pas de packaging shipping pour le moment

### Outils non-UE
- **Blender 4.x** — modélisation, rigging, animation (Nourredine)
- **Audacity** — édition audio (Nourredine)
- **GIMP / Krita** — édition icônes / textures UI (au besoin)

---

## Conventions

### Code C++
- Nomenclature UE5 : préfixes `U`, `A`, `F`, `S`, `E`
- API à jour UE 5.7 — pas de deprecated
- Enhanced Input System (jamais legacy)
- Classes abstraites → BP enfants requis (Character, WeaponBase, EnemyBase, HUD)
- `UPROPERTY` systématique pour la GC sur les objets UE
- `TObjectPtr<>` au lieu de raw pointer pour les références

### Commits Git
- Format : `[N] feat(scope): desc` ou `[J] feat(scope): desc`
- Types : `feat`, `fix`, `docs`, `refactor`, `chore`
- Scope = nom du sprint ou du module
- Exemples : `[J] feat(sprint-1): UITheme dataasset`, `[N] chore(sprint-1): footstep wavs CC0`

### Ownership assets binaires (règle d'or)
- `.uasset` — **Jilani uniquement** (Nourredine n'a pas UE5)
- `.fbx` / `.wav` / `.png` — **Nourredine produit, Jilani importe**
- `.h` / `.cpp` — **Jilani uniquement**
- `.md` — **les deux**, mergeable
- **Annonce Discord obligatoire** avant de toucher un level ou un BP critique partagé

---

## État actuel (avril 2026)

Le jeu est **gameplay-complet** depuis la phase 19 :
- Boucle complète : Title → Loadout → Match → GameOver → Leaderboard
- 11 systèmes implémentés (score, combo, vagues, inventaire RE5, talents, pickups adaptatifs, caisses destructibles, dégâts localisés, leaderboard, character select, niveau Blacksite)
- 6 armes, 5 types d'ennemis, 30 BPs pickups, 199 `.uasset`

**Ce qui manque pour atteindre v4.0** (3 axes du sprint 12 semaines en cours) :
1. **Identité visuelle** : refonte menus (sortir du look "basique UE5")
2. **Sound design** : 50% des sons déclarés sont muets (DeathSound, HitSound, footsteps, musique combat)
3. **Animations détaillées** : Sprint/Crouch/Slide/Dodge/PlayerDeath manquants, animations ennemis non câblées
4. **Game design avancé** : progression unlock, difficulty curve adaptative, achievements, tutoriel

Voir `STATE.md` pour le snapshot vivant et `ROADMAP.md` pour le découpage des sprints.

---

## Liens utiles

- Repo : (à compléter — URL GitHub)
- Drive partagé : `RevenantOps_Assets/` — dossier `incoming/` pour Nourredine
- Discord : channel `#revenantops-dev`
- Documentation interne : `.planning/research/` (mood boards, specs, refs)

---

## Versions

- **v1.0** (phases 1-10, 2026-03-25) — boucle gameplay complète
- **v2.0** (phases 11-13, 2026-04-05) — DataTables, meshes armes, animations armes
- **v3.0** (phases 14-19, 2026-04-28) — pickups adaptatifs, caisses, dégâts localisés
- **v4.0** (sprint 12 sem en cours) — identité visuelle + sound design + animations + game design avancé
