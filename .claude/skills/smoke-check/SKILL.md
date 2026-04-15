---
name: smoke-check
description: "Checklist rapide avant une session PIE ou avant un merge. Vérifie que le jeu démarre et que le flow principal fonctionne. Utiliser avant chaque test important ou avant de merger une branche."
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash
model: sonnet
---

# Smoke Check — RevenantOps

Checklist rapide avant de lancer une session PIE ou merger vers main.

## Pré-requis machine
- [ ] UE5.7 ouvert sur le bon niveau (Lvl_ThirdPerson ou BlackSite)
- [ ] Live Coding pas en cours (attendre la fin)
- [ ] Pas de fichiers .uasset en conflit git

## Build
- [ ] Dernière compilation OK (pas d'erreur dans Output Log)
- [ ] Tous les BPs compilés (pas d'icône d'erreur rouge dans Content Browser)
- [ ] ABP_Mercenaire compilé sans erreur

## Flow principal (5 minutes de test PIE)

### Title Screen
- [ ] Le jeu démarre sur le Title Screen
- [ ] Bouton JOUER fonctionne → passe au Loadout

### Loadout
- [ ] Sélection d'arme fonctionne
- [ ] Bouton CONFIRMER déclenche le match (pas de double-confirm)
- [ ] Arme choisie visible dans l'inventaire slot 0

### Match
- [ ] Personnage spawn avec l'arme équipée (mesh visible)
- [ ] WASD + souris fonctionnent
- [ ] Tir fonctionne (animation + son si assigné)
- [ ] Ennemis spawnnent depuis les spawn points
- [ ] Timer décompte dans le HUD
- [ ] Score s'incrémente sur kill

### Fin de match
- [ ] GameOver déclenché à timer = 0
- [ ] Leaderboard s'affiche avec le score
- [ ] Replay depuis GameOver → pas de crash, pas de double-bind

## Régressions connues à vérifier
- [ ] Sprint ne crash pas ("Array index out of bounds") → BS_IdleRun actif
- [ ] Inventaire Tab → ouvre/ferme sans bloquer les inputs définitivement
- [ ] Viseur CS visible au centre de l'écran

## Résultat
- **GO** → tout passe, prêt pour le travail / merge
- **NO-GO** → lister ce qui échoue, ne pas merger
