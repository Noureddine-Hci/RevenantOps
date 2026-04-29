# Sound Design Bible — RevenantOps

> Architecture audio complète : SoundClass tree, SoundMix, banque sons par catégorie, sources CC0.

---

## Architecture SoundClass

Hiérarchie à créer dans `Content/Mercenaires/Audio/Classes/` (sprint 2 sem 5) :

```
SC_Master (volume 1.0)
├── SC_Music (volume 0.6) — musiques menu + combat
├── SC_SFX_Weapons (volume 1.0) — tirs, reloads, empty clicks
├── SC_SFX_Enemies (volume 0.85) — hits, deaths, ambient
├── SC_SFX_UI (volume 0.7) — hover, click, transitions
├── SC_Ambience (volume 0.5) — vent, électricité, radio statique
└── SC_Footsteps (volume 0.6) — pas joueur + ennemis
```

### Pourquoi cette hiérarchie ?
- **SC_Master** : volume global, contrôlable depuis Options
- **Sliders Options** : Master, Music, SFX (regroupe Weapons+Enemies+UI+Footsteps), Ambience
- **Mix Combat** peut ducker la musique sans toucher SFX

---

## SoundMix "Combat"

À créer dans `Content/Mercenaires/Audio/Mixes/SM_Combat`.

**Comportement** : activé quand `MercenairesGameState::StartMatch`, désactivé quand `EndMatch`.

**Adjustments** :
| SoundClass | Volume Adjuster | Pitch | Fade In | Fade Out |
|---|---|---|---|---|
| SC_Music | 0.6 (-4dB approx) | 1.0 | 1.0s | 2.0s |
| SC_SFX_Weapons | 1.1 (+1dB) | 1.0 | 0.5s | 1.0s |
| SC_Ambience | 0.7 | 1.0 | 1.0s | 1.0s |

**API C++** :
```cpp
// Activer le mix au début du match
UGameplayStatics::PushSoundMixModifier(this, CombatSoundMix);

// Désactiver à la fin
UGameplayStatics::PopSoundMixModifier(this, CombatSoundMix);
```

---

## Banque sons — sources CC0

### Footsteps (Nourredine sprint 1 sem 2 — 10 sons à trouver)

**Source recommandée** : [freesound.org](https://freesound.org/) (filtrer license: CC0)

Mots-clés à utiliser :
- "footsteps concrete" → 3 variantes pour zone béton
- "footsteps metal" → 3 variantes pour zone passerelle métal
- "footsteps gravel" → 3 variantes pour zone extérieure
- "footsteps boots" → 1 variante générique fallback

**Format livré** : `.wav`, mono, 44.1kHz, normalisé -3dB peak, 0.3-0.5s
**Naming** : `Footstep_Concrete_01.wav`, `Footstep_Metal_02.wav`, etc.

### Pickup sounds (sprint 2 sem 8 — 3 sons)

| Type | Style | Source CC0 |
|---|---|---|
| Santé | Médical kit + power-up | freesound "medkit pickup", "health pickup" |
| Munitions | Cliquetis cartouches + métal | freesound "ammo pickup", "ammunition" |
| Arme | Mécanique solide + reverb | freesound "weapon pickup", "rifle cocking" |

### UI sounds (sprint 1 sem 3 — 4 sons)

| Type | Description | Source CC0 |
|---|---|---|
| Hover | Tick subtle métallique | déjà importé (à valider) |
| Click | Confirmation sèche | déjà importé (à valider) |
| Whoosh transition | Slide latéral écran | freesound "whoosh ui", "swoosh" |
| Impact panel | Panneau qui s'ouvre | freesound "ui impact", "panel slide" |

### Ambiance level (Nourredine sprint 1 sem 3)

**Composition** (Nourredine assemble dans Audacity) :
- Couche 1 : vent extérieur (loop, low frequency rumble)
- Couche 2 : bourdonnement électrique (transformateur, néons)
- Couche 3 : radio statique périodique (tous les 30-60s)
- Couche 4 (optionnel) : gouttes d'eau, ferraille au loin

**Format** : `.wav` stéréo, 44.1kHz, loop seamless 60s, normalisé -12dB (ambiance discrète)

### Musique combat (Nourredine sprint 1 sem 4 — split OST_1)

**Tâche** : prendre `Content/Mercenaires/Audio/Musiques/OST_1.wav` (existant, 42M) et splitter dans Audacity en 3 fichiers :

- `Music_Combat_Intro.wav` — 0-8s (build-up dramatique)
- `Music_Combat_Loop.wav` — 8-68s (60s, doit boucler proprement = transition silencieuse)
- `Music_Combat_Outro.wav` — fadeout 6s à partir d'un point musical

**Critique** : tester le loop pour qu'il n'y ait pas de "clic" au point de boucle (fades 50ms aux extrémités).

---

## Hooks code C++

### Sprint 2 sem 5 : Brancher sons existants

**`AI/EnemyBase.cpp` — `HandleDamage`** :
```cpp
void AEnemyBase::HandleDamage(...) {
    // ... code existant ...
    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }
}
```

**`AI/EnemyBase.cpp` — `HandleDeath`** :
```cpp
void AEnemyBase::HandleDeath(...) {
    // ... code existant ...
    if (DeathSound) {
        UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
    }
}
```

**`AI/EnemyBase.cpp` — `Tick` (déjà a `AmbientSoundTimer`)** :
```cpp
void AEnemyBase::Tick(float DeltaTime) {
    // ... code existant ...
    AmbientSoundTimer += DeltaTime;
    if (AmbientSoundTimer >= AmbientSoundInterval && AmbientSound) {
        UGameplayStatics::PlaySoundAtLocation(this, AmbientSound, GetActorLocation());
        AmbientSoundTimer = 0.f;
    }
}
```

### Sprint 2 sem 6 : Footsteps via AnimNotify

**Nouveau fichier** : `Source/RevenantOps/Audio/FootstepNotify.h`
```cpp
UCLASS()
class UFootstepNotify : public UAnimNotify {
    GENERATED_BODY()
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;
};
```

**`FootstepNotify.cpp`** :
```cpp
void UFootstepNotify::Notify(USkeletalMeshComponent* MeshComp, ...) {
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    // Line trace down pour détecter surface
    FHitResult Hit;
    FVector Start = Owner->GetActorLocation();
    FVector End = Start - FVector(0, 0, 200.f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);
    Params.bReturnPhysicalMaterial = true;

    if (Owner->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)) {
        // Choisir le son selon la surface
        EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
        USoundBase* SoundToPlay = nullptr;
        // ... lookup dans une map FootstepSounds par surface ...
        if (SoundToPlay) {
            UGameplayStatics::PlaySoundAtLocation(Owner, SoundToPlay, Hit.ImpactPoint);
        }
    }
}
```

**Intégration AnimSequence** : ouvrir `BS_IdleRun` (et anims sprint quand reçues), ajouter notify `Footstep` aux frames de pose-pied (gauche + droit).

### Sprint 2 sem 7 : Musique combat dynamique

**`Gameplay/MercenairesGameState.h`** :
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
USoundBase* CombatMusicIntro;

UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
USoundBase* CombatMusicLoop;

UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
USoundBase* CombatMusicOutro;

UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
USoundMix* CombatSoundMix;

UPROPERTY()
UAudioComponent* MusicComponent;
```

**`MercenairesGameState.cpp`** :
```cpp
void AMercenairesGameState::StartMatch() {
    // ... code existant ...

    // Play intro
    if (CombatMusicIntro) {
        MusicComponent = UGameplayStatics::SpawnSound2D(this, CombatMusicIntro);
    }

    // Schedule loop after intro duration (8s)
    GetWorld()->GetTimerManager().SetTimer(
        MusicLoopTimerHandle,
        FTimerDelegate::CreateLambda([this]() {
            if (MusicComponent) MusicComponent->Stop();
            if (CombatMusicLoop) {
                MusicComponent = UGameplayStatics::SpawnSound2D(this, CombatMusicLoop);
                if (MusicComponent) MusicComponent->bIsUISound = false;
                // SoundCue avec Looping ou USoundWave avec bLooping
            }
        }),
        8.f, false);

    // Active sound mix
    if (CombatSoundMix) {
        UGameplayStatics::PushSoundMixModifier(this, CombatSoundMix);
    }
}

void AMercenairesGameState::EndMatch(bool bVictory) {
    // ... code existant ...

    // Crossfade vers outro
    if (MusicComponent) MusicComponent->FadeOut(2.f, 0.f);
    if (CombatMusicOutro) {
        UGameplayStatics::SpawnSound2D(this, CombatMusicOutro);
    }

    // Désactiver mix
    if (CombatSoundMix) {
        UGameplayStatics::PopSoundMixModifier(this, CombatSoundMix);
    }
}
```

### Sprint 2 sem 8 : Pickup interface sound

**`Gameplay/PickupInterface.h`** — ajouter méthode :
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
USoundBase* GetPickupSound() const;
```

**Hook `RevenantOpsCharacter::InteractPressed`** :
```cpp
void ARevenantOpsCharacter::InteractPressed() {
    if (PendingInteractable && PendingInteractable->Implements<UPickupInterface>()) {
        // Jouer le son AVANT le pickup (sinon l'actor est detruit)
        if (USoundBase* Sound = IPickupInterface::Execute_GetPickupSound(PendingInteractable)) {
            UGameplayStatics::PlaySound2D(this, Sound);
        }
        IPickupInterface::Execute_TryPickupInteract(PendingInteractable, this);
    }
}
```

---

## Banques de fallback (si freesound CC0 insuffisant)

1. **BBC Sound Effects** — [bbcsfx.acropolis.org.uk](https://bbcsfx.acropolis.org.uk/) — license RemArc (usage non-commercial OK)
2. **OpenGameArt** — [opengameart.org](https://opengameart.org) — sections audio CC0/CC-BY
3. **Sonniss GDC Bundle** — [sonniss.com](https://sonniss.com/gameaudiogdc) — bundles annuels gratuits, license commercial OK
4. **Pixabay Music** — [pixabay.com/music](https://pixabay.com/music) — Pixabay License (use commercial OK)
5. **Zapsplat** — [zapsplat.com](https://www.zapsplat.com) — gratuit avec attribution

---

## Volumes recommandés (master = 1.0)

| Catégorie | Volume relatif | Justification |
|---|---|---|
| Music menu | 0.45 | Discrète sous les sons UI |
| Music combat | 0.55 | Audible mais SFX prioritaire |
| Tirs joueur | 1.0 | Punchy, c'est l'action |
| Tirs ennemis | 0.7 | Distinguable du joueur |
| Hits ennemis | 0.85 | Feedback fort |
| Death ennemis | 0.9 | Confirme le kill |
| Footsteps joueur | 0.4 | Présent mais pas envahissant |
| Footsteps ennemis | 0.6 | Importante info gameplay (alerte) |
| Pickups | 0.7 | Feedback positif |
| UI hover | 0.3 | Subtle |
| UI click | 0.5 | Confirmation claire |
| Whoosh transitions | 0.6 | Présent |
| Ambiance level | 0.3 | Background, ne masque pas |

---

## Checklist sprint 2 audio (validation)

- [ ] SoundClass tree créé et hiérarchie correcte
- [ ] SoundMix "Combat" créé et configuré
- [ ] EnemyBase joue HitSound au damage
- [ ] EnemyBase joue DeathSound au death
- [ ] EnemyBase joue AmbientSound périodique
- [ ] Footsteps audibles avec variation surface (béton/métal/gravier)
- [ ] Pickups (santé/ammo/arme) jouent un son distinct
- [ ] Musique combat démarre au StartMatch
- [ ] Musique combat fade-out à EndMatch
- [ ] Ambiance level Blacksite audible
- [ ] Sliders Options modifient les volumes correctement
- [ ] Aucun son n'overflow / clip / distorts
