---
name: gate-check
description: "Valide si une phase RevenantOps est vraiment terminée. Produit un verdict PASS / CONCERNS / FAIL avec les blockers concrets. Utiliser quand tu dis 'phase X terminée', 'on peut passer à la suite', 'check si on est prêts'."
argument-hint: "[phase-number: ex 17 ou 18] [--quick]"
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash
model: sonnet
---

# Gate Check — RevenantOps Phase Validation

Vérifie qu'une phase est réellement terminée avant de la déclarer complète.

## 1. Identifier la phase à valider

Si `$ARGUMENTS` fourni → valider cette phase.
Sinon → demander : "Quelle phase tu veux valider ?"

## 2. Checklist universelle (toutes phases)

### Code
- [ ] Compile sans erreur (MSVC / VS 2026)
- [ ] Zero warning critique
- [ ] Conventions UE5 respectées (CamelCase, prefixes U/A/F/S/E)
- [ ] Pas de duplication inutile
- [ ] Pas de memory leaks (UPROPERTY pour GC)
- [ ] APIs UE 5.7 à jour (pas de deprecated)

### Gameplay
- [ ] Testé en PIE (pas juste en mode éditeur)
- [ ] Le flow principal fonctionne sans crash
- [ ] Pas de double-bind / double-kill / double-confirm (guards en place)
- [ ] Null checks sur les pointeurs critiques

### Assets & Blueprints
- [ ] BPs compilés sans erreur
- [ ] DataTables correctement peuplées si applicable
- [ ] Noms assets respectent les conventions (BP_, WBP_, ABP_, SM_, SK_...)

### Git
- [ ] Changements committés avec format `[J] type(scope): desc` ou `[N]`
- [ ] CLAUDE.md mis à jour avec l'état de la phase
- [ ] primer.md mis à jour

## 3. Checklist par phase

### Phase 14 (Combat Feel)
- [ ] Camera shake au tir (UCS_WeaponFire)
- [ ] Camera shake sur dégâts reçus (UCS_TakeDamage)
- [ ] Hit flash blanc 0.15s sur les ennemis touchés
- [ ] NS_MuzzleFlash, NS_Impact_Surface, NS_Impact_Blood assignés aux 6 weapon BPs
- [ ] WBP_Mercenaires_HUD avec tous les widgets requis (noms EXACTS)
- [ ] Sons Kenney : FireSound/ReloadSound/EmptySound/HitSound/DeathSound assignés

### Phase 17 (Inventaire RE5 + Viseur CS)
- [ ] Tab ouvre/ferme l'inventaire
- [ ] Navigation ZQSD/flèches dans les 9 slots
- [ ] E utilise l'item sélectionné
- [ ] Viseur CS : 4 traits visibles, s'écartent au tir, se resserrent en ADS
- [ ] WeaponIcon assigné sur BP_Pistol, BP_AssaultRifle, BP_SMG, BP_Shotgun, BP_Sniper
- [ ] Time dilation 0.3x quand inventaire ouvert
- [ ] FInputModeUIOnly bloque tir pendant inventaire

### Phase 18+ (à définir)
- Vérifier CLAUDE.md pour les critères spécifiques

## 4. Verdict

**PASS** → Tous les critères cochés. Déclarer la phase complète, mettre à jour CLAUDE.md.

**CONCERNS** → 1-2 points mineurs manquants. Lister ce qui reste, estimer l'effort.

**FAIL** → Critères bloquants non remplis. Lister les blockers avec le fichier/ligne concerné.

## 5. Actions post-verdict

Si PASS :
- Mettre à jour `CLAUDE.md` section "Etat Phase XX"
- Mettre à jour `~/.claude/primer.md` avec la prochaine étape
- Suggérer le commit : `[J/N] docs(phaseXX): gate-check PASS`

Si FAIL :
- Lister les actions correctives par ordre de priorité
- Proposer de lancer `/team-combat` si c'est du gameplay
