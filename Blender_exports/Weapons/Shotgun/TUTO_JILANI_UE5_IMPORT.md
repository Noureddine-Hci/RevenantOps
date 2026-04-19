# Tuto Import Shotgun 870 dans UE5 — Pour Jilani

Fait par Noureddine (Blender). Toi tu fais la partie UE5.

---

## Fichiers livrés

```
Blender_exports/Weapons/Shotgun/
  SK_Shotgun_870.fbx           ← Skeletal Mesh + armature + 3 animations
  T_Shotgun_BaseColor.png      ← Albedo (couleur)
  T_Shotgun_Normal.png         ← Normal map
  T_Shotgun_ORM.png            ← Occlusion / Roughness / Metallic (canaux R/G/B)
```

---

## Étape 1 — Importer les textures

Dans le Content Browser UE5, va dans :
```
Content/Mercenaires/Weapons/Shotgun/Textures/
```
(Crée le dossier si besoin)

Drag & drop les 3 fichiers `.png`. Pour `T_Shotgun_Normal.png` :
- Clic droit → Asset Actions → Find in Content Browser
- Double-clic pour ouvrir → changer **Compression Settings** → `Normalmap`
- Save

---

## Étape 2 — Créer le matériau M_Shotgun_870

Dans `Content/Mercenaires/Weapons/Shotgun/Materials/`, crée un Material `M_Shotgun_870`.

Node setup :
```
T_Shotgun_BaseColor  → Base Color (Principled BSDF node)
T_Shotgun_Normal     → Normal (via Texture Sample → Normal Map node)
T_Shotgun_ORM (R)    → Ambient Occlusion
T_Shotgun_ORM (G)    → Roughness
T_Shotgun_ORM (B)    → Metallic
```

Save le matériau.

---

## Étape 3 — Importer le FBX

Dans `Content/Mercenaires/Weapons/Shotgun/`, drag & drop `SK_Shotgun_870.fbx`.

Dans la fenêtre d'import :

| Option | Valeur |
|---|---|
| Skeletal Mesh | ✅ coché |
| Import Mesh | ✅ |
| Import Animations | ✅ |
| Animation Length | Exported Time |
| Import as Morph Target | ❌ |
| Use T0 As Ref Pose | ✅ |
| Convert Scene Unit | ✅ |
| Force Front X Axis | ❌ |

→ Cliquer **Import All**

UE5 va créer :
- `SK_Shotgun_870` (Skeletal Mesh)
- `SK_Shotgun_870_Skeleton` (Skeleton)
- `AM_Shotgun_Fire` (Animation Sequence — recul fort)
- `AM_Shotgun_PumpAction` (Animation Sequence — retour pompe)
- `AM_Shotgun_Equip` (Animation Sequence — sortie holster)

---

## Étape 4 — Assigner le matériau

Double-clic sur `SK_Shotgun_870` → dans le panneau **Materials** → assigner `M_Shotgun_870` sur chaque slot.

Save.

---

## Étape 5 — Créer les AnimMontages

Pour chaque animation, clic droit sur la Sequence → **Create AnimMontage**.

Renommer :
- `AM_Shotgun_Fire_Montage`
- `AM_Shotgun_PumpAction_Montage`
- `AM_Shotgun_Equip_Montage`

Mettre dans `Content/Mercenaires/Anims/Montages/`.

---

## Étape 6 — Assigner dans BP_Shotgun

Ouvrir `BP_Shotgun` → onglet **Defaults** (Class Defaults).

| Propriété | Valeur |
|---|---|
| Skeletal Mesh | `SK_Shotgun_870` |
| CharacterFireMontage | `AM_Shotgun_Fire_Montage` |
| CharacterReloadMontage | `AM_Shotgun_PumpAction_Montage` |
| CharacterEquipMontage | `AM_Shotgun_Equip_Montage` |

> **ATTENTION** : Sur un vrai shotgun, on utilise `PumpAction` au lieu de `ReloadMontage`. Ici j'ai assigné PumpAction à CharacterReloadMontage pour compatibilité avec le système existant. Si tu veux un anim de rechargement séparé (insérer cartouches), utilise la logique custom dans `WeaponBase::Reload()`.

Compile + Save.

---

## Étape 7 — Rotation socket hand_r

Dans `SK_Shotgun_870` → onglet **Skeleton** → chercher socket `hand_r`.

Si le socket n'existe pas, clic droit sur le bone `Weapon_Root` → **Add Socket** → nommer `hand_r`.

Rotation du socket :
```
Pitch = 0
Yaw   = 90
Roll  = 90
```

---

## Étape 8 — DataTable (optionnel)

La ligne `Shotgun` dans `DT_WeaponStats` existe déjà (6 balles par chargeur, 18 munitions réserve). Les montages sont assignés sur `BP_Shotgun`, pas besoin de modifier la DT.

---

## Étape 9 — Test PIE

1. Lance le PIE
2. Loadout → sélectionne le shotgun
3. Vérifie : arme visible ✅ | Fire joue recoil + pompe revient ✅ | Equip sort du holster ✅

---

## Résumé des assets créés

| Asset | Path |
|---|---|
| `SK_Shotgun_870` | `Content/Mercenaires/Weapons/Shotgun/SK_Shotgun_870` |
| `M_Shotgun_870` | `Content/Mercenaires/Weapons/Shotgun/Materials/M_Shotgun_870` |
| `T_Shotgun_BaseColor` | `Content/Mercenaires/Weapons/Shotgun/Textures/` |
| `T_Shotgun_Normal` | `Content/Mercenaires/Weapons/Shotgun/Textures/` |
| `T_Shotgun_ORM` | `Content/Mercenaires/Weapons/Shotgun/Textures/` |
| `AM_Shotgun_Fire_Montage` | `Content/Mercenaires/Anims/Montages/` |
| `AM_Shotgun_PumpAction_Montage` | `Content/Mercenaires/Anims/Montages/` |
| `AM_Shotgun_Equip_Montage` | `Content/Mercenaires/Anims/Montages/` |

---

*Généré par Noureddine — pipeline Blender → UE5, session 2026-04-15*
