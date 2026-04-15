# Tuto Import MP5 dans UE5 — Pour Jilani

Fait par Noureddine (Blender). Toi tu fais la partie UE5.

---

## Fichiers livrés

```
Blender_exports/Weapons/MP5/
  SK_MP5.fbx               ← Skeletal Mesh + armature + 3 animations
  T_MP5_BaseColor.png      ← Albedo (couleur) — modèle CastleBravo n'a qu'une texture
  SK_MP5_Source.blend      ← Source Blender
```

> Note : ce modèle n'a qu'une BaseColor (pas de Normal ni ORM séparé). Si tu veux plus de détails PBR, on peut en créer dans Blender plus tard.

---

## Armature — bones importants

| Bone | Pièce | Utilisé dans |
|---|---|---|
| `Weapon_Root` | Corps entier (Receiver, Barrel, Stock, Grip) | Fire recoil, Equip |
| `Bolt_Bone` | Culasse mobile | Fire (recule au tir) |
| `Mag_Bone` | Chargeur | Reload (descend/remonte) |
| `Trigger_Bone` | Gâchette | (disponible pour polish) |

---

## Étape 1 — Importer la texture

Dans `Content/Mercenaires/Weapons/SMG/Textures/`, drag & drop `T_MP5_BaseColor.png`.

---

## Étape 2 — Créer le matériau M_MP5

Dans `Content/Mercenaires/Weapons/SMG/Materials/`, crée un Material `M_MP5`.

- `T_MP5_BaseColor` → Base Color
- Metallic : 0.8 (constante)
- Roughness : 0.4 (constante)

Save.

---

## Étape 3 — Importer le FBX

Dans `Content/Mercenaires/Weapons/SMG/`, drag & drop `SK_MP5.fbx`.

| Option | Valeur |
|---|---|
| Skeletal Mesh | ✅ |
| Import Animations | ✅ |
| Use T0 As Ref Pose | ✅ |
| Convert Scene Unit | ✅ |

→ **Import All**

UE5 crée :
- `SK_MP5` (Skeletal Mesh)
- `SK_MP5_Skeleton`
- `AM_SMG_Fire` — recoil rapide + bolt recule
- `AM_SMG_Reload` — chargeur descend (F12), pause, remonte (F32)
- `AM_SMG_Equip` — sortie holster

---

## Étape 4 — Matériau + AnimMontages

- Assigner `M_MP5` sur tous les slots de `SK_MP5`
- Clic droit sur chaque Sequence → **Create AnimMontage** :
  - `AM_SMG_Fire_Montage`
  - `AM_SMG_Reload_Montage`
  - `AM_SMG_Equip_Montage`
- Déplacer dans `Content/Mercenaires/Anims/Montages/`

---

## Étape 5 — Assigner dans BP_SMG

| Propriété | Valeur |
|---|---|
| Skeletal Mesh | `SK_MP5` |
| CharacterFireMontage | `AM_SMG_Fire_Montage` |
| CharacterReloadMontage | `AM_SMG_Reload_Montage` |
| CharacterEquipMontage | `AM_SMG_Equip_Montage` |

Compile + Save.

---

## Étape 6 — Socket hand_r

Dans `SK_MP5` → Skeleton → bone `Weapon_Root` → Add Socket `hand_r` :

```
Pitch = 0 | Yaw = 90 | Roll = 90
```

---

## Test PIE

1. Loadout → SMG
2. Arme visible ✅ | Fire rapide avec bolt qui recule ✅ | Reload chargeur sort/rentre ✅ | Equip holster → prêt ✅

---

*Généré par Noureddine — pipeline Blender → UE5, session 2026-04-15*
