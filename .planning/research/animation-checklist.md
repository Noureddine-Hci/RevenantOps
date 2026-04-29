# Animation Checklist — RevenantOps

> Inventaire complet des animations requises pour v4.0. État C++ × État asset × Priorité × Responsable.
>
> Légende état : ✅ Fait | 🚧 Partiel | ❌ Manquant

---

## ANIMATIONS JOUEUR

### Existant (à conserver)

| ID | Animation | État C++ | État Asset | Notes |
|---|---|---|---|---|
| P-001 | Pistol Fire | ✅ branché | ✅ AM_Pistol_Fire | OK |
| P-002 | Pistol Reload | ✅ | ✅ AM_Pistol_Reload | OK |
| P-003 | Pistol Equip | ✅ | ✅ AM_Pistol_Equip | OK |
| P-004 | Rifle Fire | ✅ | ✅ AM_Rifle_Fire | OK |
| P-005 | Rifle Reload | ✅ | ✅ AM_Rifle_Reload | OK |
| P-006 | Rifle Equip | ✅ | ✅ AM_Rifle_Equip | OK |
| P-007 | Idle (BS_IdleRun) | ✅ | ✅ | OK |
| P-008 | Walk (BS_IdleRun) | ✅ | ✅ | OK |
| P-009 | Run (BS_IdleRun) | ✅ | ✅ | OK |

### À produire (Nourredine — sprint 2)

| ID | Animation | Priorité | Difficulté | Sem | État |
|---|---|---|---|---|---|
| P-010 | Pickup (se baisser, prendre, relever) | HAUTE | ★☆☆ | sem 5 | ❌ |
| P-011 | Player Death (chute en avant) | HAUTE | ★☆☆ | sem 5 | ❌ |
| P-012 | Crouch Idle loop | HAUTE | ★★☆ | sem 6 | ❌ |
| P-013 | Crouch Walk loop | HAUTE | ★★☆ | sem 6 | ❌ |
| P-014 | Sprint loop | HAUTE | ★★☆ | sem 7 | ❌ |
| P-015 | Slide (one-shot 1s) | MOYENNE | ★★★ | sem 8 | ❌ |

### À produire (Nourredine — sprint 3 si rythme OK)

| ID | Animation | Priorité | Difficulté | État |
|---|---|---|---|---|
| P-020 | Dodge / Roll (avant/arrière/gauche/droite × 4) | MOYENNE | ★★★ | ❌ |
| P-021 | Jump (apex + landing) | MOYENNE | ★★☆ | ❌ |
| P-022 | Finisher Head (coup de coude / poignet) | BASSE | ★★★ | ❌ |
| P-023 | Finisher Leg (stomp / coup de genou) | BASSE | ★★★ | ❌ |
| P-024 | Finisher Torso (coup de poing / pousser) | BASSE | ★★★ | ❌ |

**Note** : si Nourredine bloque sur dodge/finishers (★★★), Jilani peut télécharger des packs Mixamo gratuits comme placeholder (license OK pour dev personnel).

---

## ANIMATIONS ENNEMIS

### Existant (à brancher dans le code)

| ID | Animation | État C++ | État Asset | Action requise |
|---|---|---|---|---|
| E-001 | HitReactAnim | 🚧 déclaré, **jamais joué** | ❌ | Brancher + créer asset (sprint 3) |
| E-002 | DeathAnim | 🚧 déclaré, **jamais joué** | ❌ | Brancher + créer asset (sprint 3) |
| E-003 | AttackAnim | 🚧 déclaré, **jamais joué** | ❌ | Brancher + créer asset (sprint 3) |

### Stagger system (code OK, assets manquants)

| ID | Animation | État C++ | État Asset | Sprint |
|---|---|---|---|---|
| E-010 | StaggerMontageHead | ✅ branché TakeHitZoneDamage | ❌ | 3 sem 10 |
| E-011 | StaggerMontageLeg | ✅ branché | ❌ | 3 sem 10 |
| E-012 | StaggerMontageTorso | ✅ branché | ❌ | 3 sem 10 |

### À produire (Nourredine — sprint 3)

| ID | Animation | Difficulté | Sem | État |
|---|---|---|---|---|
| E-020 | Stagger Head (zombie se tient la tête, chancelle) | ★★☆ | 10 | ❌ |
| E-021 | Stagger Leg (zombie tombe à genoux) | ★★☆ | 10 | ❌ |
| E-022 | Stagger Torso (zombie recule en se tordant) | ★★☆ | 10 | ❌ |
| E-030 | Death variation 1 (chute en avant) | ★★☆ | 11 | ❌ |
| E-031 | Death variation 2 (chute sur le côté) | ★★☆ | 11 | ❌ |
| E-032 | Death variation 3 (chute en arrière) | ★★☆ | 11 | ❌ |
| E-040 | Attack mêlée bras (Slow zombie) | ★★★ | 12 | ❌ |
| E-041 | Attack morsure (Runner) | ★★★ | 12 | ❌ |
| E-042 | Attack charge (Tank) | ★★★ | 12 | ❌ |

**Total ennemis sprint 3** : 9 animations (3 stagger + 3 death + 3 attack)
**Estimation** : 1 semaine par catégorie (10 = stagger, 11 = death, 12 = attack)

---

## ANIM BLUEPRINT — ABP_Mercenaire

### États existants
- ✅ Locomotion (BS_IdleRun)
- ✅ Armed (transition via `bIsArmed`)
- ✅ Reload state (montage)

### États à ajouter (sprint 2 sem 7-8)
- [ ] **Sprint state** : lecture `Sprint loop` quand `bIsSprinting && Velocity > 700`
- [ ] **Crouch state** : `Crouch_Idle` + `Crouch_Walk` blendspace
- [ ] **Slide state** : montage one-shot, retour Locomotion à la fin
- [ ] **Death state** : depuis n'importe quel state si `bIsDead`, lecture `Player_Death`

### Variables AnimBP à exposer
```cpp
// Ajouter dans ABP_Mercenaire (Blueprint Variables):
bool bIsSprinting     // set par RevenantOpsCharacter
bool bIsCrouching     // set par RevenantOpsCharacter
bool bIsSliding       // set par RevenantOpsCharacter
bool bIsDead          // set par RevenantOpsCharacter
float Speed2D         // calculé depuis Velocity
float SlideElapsed    // pour timing slide
```

---

## CHECKLIST C++ (Jilani)

### Sprint 2 sem 5-6 — branchements
- [ ] Importer FBX Pickup → créer `AM_Player_Pickup` montage
- [ ] Importer FBX Player_Death → créer `AM_Player_Death` montage
- [ ] `RevenantOpsCharacter::AddPickupMontage` UPROPERTY + binding sur `InteractPressed`
- [ ] `RevenantOpsCharacter::AddDeathMontage` UPROPERTY + binding sur `OnHealthDepleted`

### Sprint 2 sem 6 — crouch
- [ ] Importer FBX Crouch_Idle + Crouch_Walk → blendspace `BS_Crouch`
- [ ] ABP : Crouch state qui lit `BS_Crouch` selon `Speed2D`
- [ ] `RevenantOpsCharacter::CrouchPressed` set `bIsCrouching = true`
- [ ] Activer crouch via UE5 native (`ACharacter::Crouch()`) qui réduit hauteur capsule

### Sprint 2 sem 7 — sprint
- [ ] Importer FBX Sprint loop → `Sprint_Loop`
- [ ] ABP : Sprint state qui lit `Sprint_Loop`
- [ ] Logique : `bIsSprinting = Velocity > SprintThreshold`

### Sprint 2 sem 8 — slide
- [ ] Importer FBX Slide → `AM_Slide`
- [ ] `RevenantOpsCharacter::OnSlideStart` joue `AM_Slide` + déplacement physique slide
- [ ] ABP : Slide state qui s'active via `bIsSliding`

### Sprint 3 sem 10 — stagger
- [ ] Importer 3 FBX Stagger → 3 montages `AM_Stagger_Head/Leg/Torso`
- [ ] Assigner aux `EnemyBase::StaggerMontageHead/Leg/Torso` (déjà déclarés)
- [ ] Code C++ déjà en place (`PlayAnimMontage` dans `TakeHitZoneDamage`)

### Sprint 3 sem 11 — death
- [ ] Importer 3 FBX Death → 3 sequences `Death_01/02/03`
- [ ] `EnemyBase::HandleDeath` : sélectionner `DeathAnim` aléatoire parmi les 3
- [ ] Disable physics + ragdoll fallback après animation

### Sprint 3 sem 12 — attack
- [ ] Importer 3 FBX Attack → assigner par type :
  - `BP_ZombieSlow` : Attack mêlée bras
  - `BP_ZombieRunner` : Attack morsure
  - `BP_ZombieTank` : Attack charge
- [ ] `EnemyBase::FireAtPlayer` ou nouvelle méthode `Attack()` joue `AttackAnim`
- [ ] Damage déclenché via AnimNotify dans la fenêtre d'attaque

---

## TUTORIELS POUR NOUREDDINE

Voir détails complets dans `.planning/research/blender-tasks-noureddine.md`.

**Workflow général** (à respecter pour chaque animation) :
1. Ouvrir Blender 4.x
2. Importer le mannequin UE5 squelette (FBX fourni par Jilani sem 3)
3. Mode Pose : créer keyframes
4. Vérifier que **rootmotion = false** (le mouvement doit venir du moteur, pas de l'anim)
5. Export FBX :
   - Armature only : NON
   - Mesh : OUI
   - Bake animation : OUI
   - Force Front Axis : -Y
   - Force Up Axis : Z
6. Tester import dans Blender vide (sanity check)
7. Livrer dans Drive `incoming/Anims_PlayerName/SemX/`

**Naming convention FBX** :
- Player : `Player_<Action>.fbx` (ex: `Player_Pickup.fbx`, `Player_Crouch_Idle.fbx`)
- Ennemi : `Enemy_<Type>_<Action>.fbx` (ex: `Enemy_Zombie_Stagger_Head.fbx`)

---

## RISQUES ANIMATION

| Risque | Mitigation |
|---|---|
| Mauvais squelette (Mixamo vs UE5 Mannequin) | Fournir le FBX squelette UE5 cible à Nourredine sem 3 |
| Root motion non géré (anim glisse) | Vérifier "Bake Animation" + désactiver root motion à l'import |
| Anim trop longue / saccadée | Définir durées cibles précises dans la spec |
| Timing désynchronisé avec sons | Synchroniser via AnimNotify (ex: footstep notify aux frames pose-pied) |
| FBX corrompu / ne s'importe pas | Validation immédiate par Jilani à chaque livraison |

---

## VALIDATION GLOBALE (fin sprint 3)

- [ ] Tous les montages joueur fonctionnels (sprint, crouch, slide, pickup, death)
- [ ] ABP_Mercenaire a tous les states (locomotion, armed, sprint, crouch, slide, death)
- [ ] Stagger ennemis joue le bon montage selon `EStaggerType`
- [ ] Death ennemis joue une variation aléatoire
- [ ] Attack ennemis distinct par type
- [ ] Pas de "T-pose" visible en aucune circonstance
- [ ] Pas d'anim qui glisse (root motion correctement géré)
