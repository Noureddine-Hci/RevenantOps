# Requirements: RevenantOps

**Defined:** 2026-03-22
**Core Value:** Le joueur peut incarner un personnage TPS complet avec tir, esquive, et affronter des vagues d'ennemis IA

## v2.0 Requirements

Requirements pour rendre le jeu jouable. Chaque requirement mappe a une phase du roadmap.

### Input System

- [ ] **INPUT-01**: Input Actions TPS crees (Move, Look, MouseLook, Jump, Sprint, Crouch, Dodge, Fire, Aim, Reload, SwitchWeapon)
- [ ] **INPUT-02**: Input Actions Combat crees (ComboAttack, ChargedAttack, ToggleCamera)
- [ ] **INPUT-03**: Input Mapping Context TPS configure (clavier/souris + gamepad)
- [ ] **INPUT-04**: Input Mapping Context Combat configure

### Blueprints TPS

- [ ] **BPTPS-01**: BP_RevenantOpsCharacter cree et configure (mesh, input actions, weapon slots)
- [ ] **BPTPS-02**: BP_Pistol cree (herite WeaponBase, parametres de tir configures)
- [ ] **BPTPS-03**: BP_EnemyBase cree (herite EnemyBase, HealthComponent, LifeBar)
- [ ] **BPTPS-04**: BP_RevenantOpsGameMode configure (character class, controller class)
- [ ] **BPTPS-05**: BP_RevenantOpsPlayerController configure (mapping contexts, HUD widget class)

### HUD & UI

- [ ] **UI-01**: WBP_HUD cree (herite RevenantOpsHUD, widgets bindes: HealthBar, ShieldBar, StaminaBar, AmmoText, Crosshair, HitMarker, Vignette)
- [ ] **UI-02**: WBP_EnemyLifeBar cree pour affichage vie ennemis

### Variant Combat

- [ ] **COMBAT-01**: BP_CombatCharacter cree et configure (input actions, montages, parametres melee)
- [ ] **COMBAT-02**: BP_CombatEnemy cree (montages, parametres combat, LifeBar)
- [ ] **COMBAT-03**: BP_CombatGameMode configure (character class, controller class)
- [ ] **COMBAT-04**: BP_CombatPlayerController configure (mapping contexts, character class)
- [ ] **COMBAT-05**: WBP_CombatLifeBar cree (SetLifePercentage, SetBarColor)

### IA Combat

- [ ] **AI-01**: StateTree CombatEnemy cree avec tasks C++ (ComboAttack, ChargedAttack, FaceActor, etc.)

### Animation

- [ ] **ANIM-01**: Montages TPS placeholder crees (Dodge, Fire, Reload, Equip)
- [ ] **ANIM-02**: Montages Combat placeholder crees (ComboAttack avec sections, ChargedAttack avec loop)
- [ ] **ANIM-03**: AnimBP Character basique (locomotion state machine)
- [ ] **ANIM-04**: AnimBP Enemy basique

### Integration

- [ ] **INTEG-01**: Niveau de test configure (spawners, checkpoints, ennemis)
- [ ] **INTEG-02**: Le jeu se lance en PIE sans crash
- [ ] **INTEG-03**: Le joueur peut se deplacer, tirer, et tuer des ennemis

## Future Requirements

### Variant Platforming
- **PLAT-01**: BP_PlatformingCharacter configure
- **PLAT-02**: Niveau platforming jouable

### Variant SideScrolling
- **SIDE-01**: BP_SideScrollingCharacter configure
- **SIDE-02**: Niveau side-scrolling jouable

### Polish
- **POLISH-01**: Audio/SFX integres (tir, impact, reload, UI)
- **POLISH-02**: VFX (muzzle flash, impact particles, blood)
- **POLISH-03**: Level design avance

## Out of Scope

| Feature | Reason |
|---------|--------|
| Mobile controls widget | Pas prioritaire, desktop first |
| Multiplayer | Pas prevu dans le design |
| Audio/VFX finaux | Placeholders suffisants pour v2.0 |
| Level design avance | Milestone futur apres validation gameplay |
| Import assets externes | Limitation MCP, utiliser mannequin UE5 |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| INPUT-01 | TBD | Pending |
| INPUT-02 | TBD | Pending |
| INPUT-03 | TBD | Pending |
| INPUT-04 | TBD | Pending |
| BPTPS-01 | TBD | Pending |
| BPTPS-02 | TBD | Pending |
| BPTPS-03 | TBD | Pending |
| BPTPS-04 | TBD | Pending |
| BPTPS-05 | TBD | Pending |
| UI-01 | TBD | Pending |
| UI-02 | TBD | Pending |
| COMBAT-01 | TBD | Pending |
| COMBAT-02 | TBD | Pending |
| COMBAT-03 | TBD | Pending |
| COMBAT-04 | TBD | Pending |
| COMBAT-05 | TBD | Pending |
| AI-01 | TBD | Pending |
| ANIM-01 | TBD | Pending |
| ANIM-02 | TBD | Pending |
| ANIM-03 | TBD | Pending |
| ANIM-04 | TBD | Pending |
| INTEG-01 | TBD | Pending |
| INTEG-02 | TBD | Pending |
| INTEG-03 | TBD | Pending |

**Coverage:**
- v2.0 requirements: 23 total
- Mapped to phases: 0
- Unmapped: 23

---
*Requirements defined: 2026-03-22*
*Last updated: 2026-03-22 after initial definition*
