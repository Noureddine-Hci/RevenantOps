# Tuto Import M92F dans UE5 — Pour Jilani

Fait par Noureddine (Blender). Toi tu fais la partie UE5.

---

## Fichiers livrés

```
Blender_exports/Weapons/M92F/
  SK_M92F.fbx            ← Skeletal Mesh + armature + 3 animations
  T_M92F_BaseColor.png   ← Albedo (couleur)
  T_M92F_Normal.png      ← Normal map
  T_M92F_ORM.png         ← Occlusion / Roughness / Metallic (canaux R/G/B)
```

---

## Étape 1 — Importer les textures

Dans le Content Browser UE5, va dans :
```
Content/Mercenaires/Weapons/Pistol/Textures/
```
(Crée le dossier si besoin)

Drag & drop les 3 fichiers `.png`. Pour `T_M92F_Normal.png` :
- Clic droit → Asset Actions → Find in Content Browser
- Double-clic pour ouvrir → changer **Compression Settings** → `Normalmap`
- Save

---

## Étape 2 — Créer le matériau M_M92F

Dans `Content/Mercenaires/Weapons/Pistol/Materials/`, crée un Material `M_M92F`.

Node setup :
```
T_M92F_BaseColor  → Base Color (Principled BSDF node)
T_M92F_Normal     → Normal (via Texture Sample → Normal Map node)
T_M92F_ORM (R)    → Ambient Occlusion
T_M92F_ORM (G)    → Roughness
T_M92F_ORM (B)    → Metallic
```

> Dans UE5 : Texture Sample → clic droit sur le pin de sortie → Split Struct Pin pour avoir R/G/B séparés.

Save le matériau.

---

## Étape 3 — Importer le FBX

Dans `Content/Mercenaires/Weapons/Pistol/`, drag & drop `SK_M92F.fbx`.

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
- `SK_M92F` (Skeletal Mesh)
- `SK_M92F_Skeleton` (Skeleton)
- `AM_Pistol_Fire` (Animation Sequence)
- `AM_Pistol_Reload` (Animation Sequence)
- `AM_Pistol_Equip` (Animation Sequence)

---

## Étape 4 — Assigner le matériau

Double-clic sur `SK_M92F` → dans le panneau **Materials** → assigner `M_M92F` sur chaque slot.

Save.

---

## Étape 5 — Créer les AnimMontages

Pour chaque animation, clic droit sur la Sequence → **Create AnimMontage**.

Renommer :
- `AM_Pistol_Fire_Montage`
- `AM_Pistol_Reload_Montage`
- `AM_Pistol_Equip_Montage`

Mettre dans `Content/Mercenaires/Anims/Montages/`.

---

## Étape 6 — Assigner dans BP_Pistol

Ouvrir `BP_Pistol` → onglet **Defaults** (Class Defaults).

| Propriété | Valeur |
|---|---|
| Skeletal Mesh | `SK_M92F` |
| CharacterFireMontage | `AM_Pistol_Fire_Montage` |
| CharacterReloadMontage | `AM_Pistol_Reload_Montage` |
| CharacterEquipMontage | `AM_Pistol_Equip_Montage` |

Compile + Save.

---

## Étape 7 — Rotation socket hand_r

Dans `SK_M92F` → onglet **Skeleton** → chercher socket `hand_r`.

Si le socket n'existe pas, clic droit sur le bone `hand_r` → **Add Socket** → nommer `hand_r`.

Rotation du socket :
```
Pitch = 0
Yaw   = 90
Roll  = 90
```

> Ces valeurs corrigent l'orientation native Blender (Y-forward) vers UE5 (X-forward).

---

## Étape 8 — Test PIE

1. Lance le PIE
2. Prends le pistol dans le loadout
3. Vérifie : arme visible dans la main ✅ | Fire joue l'anim ✅ | Reload joue ✅ | Equip à la sortie de holster ✅

Si l'arme est mal orientée dans la main → ajuste la rotation du socket (Yaw ±90).

---

## DataTable (si besoin)

La ligne `Pistol` dans `DT_WeaponStats` existe déjà. Si tu veux ajouter les refs des montages :
- Ouvrir `DT_WeaponStats` → ligne `Pistol`
- Les montages sont assignés directement sur `BP_Pistol`, pas dans le DT — rien à faire ici.

---

## Résumé des assets créés

| Asset | Path |
|---|---|
| `SK_M92F` | `Content/Mercenaires/Weapons/Pistol/SK_M92F` |
| `M_M92F` | `Content/Mercenaires/Weapons/Pistol/Materials/M_M92F` |
| `T_M92F_BaseColor` | `Content/Mercenaires/Weapons/Pistol/Textures/` |
| `T_M92F_Normal` | `Content/Mercenaires/Weapons/Pistol/Textures/` |
| `T_M92F_ORM` | `Content/Mercenaires/Weapons/Pistol/Textures/` |
| `AM_Pistol_Fire_Montage` | `Content/Mercenaires/Anims/Montages/` |
| `AM_Pistol_Reload_Montage` | `Content/Mercenaires/Anims/Montages/` |
| `AM_Pistol_Equip_Montage` | `Content/Mercenaires/Anims/Montages/` |

---

*Généré par Noureddine — pipeline Blender → UE5, session 2026-04-15*
