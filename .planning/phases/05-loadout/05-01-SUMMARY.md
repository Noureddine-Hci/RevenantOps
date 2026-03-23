# Phase 05 — Loadout : Résumé d'exécution

**Complété le :** 2026-03-23
**Fichiers créés :** 2

## Ce qui a été fait

### LoadoutWidget.h/.cpp

**Struct FWeaponLoadoutInfo**
- WeaponName (FText), WeaponClass (TSubclassOf<AWeaponBase>)
- Damage, FireRate, MagazineSize, ReloadTime (float)
- WeaponIcon (UTexture2D*)

**Méthodes clés**
- `PopulateFromClasses()` — lit les CDO stats via GetBaseDamage() / GetFireRate() / etc.
- `SelectWeapon(int32 WeaponIndex, int32 SlotIndex)` — SlotIndex 0=Primary, 1=Secondary
- `ConfirmLoadout()` — broadcast FOnLoadoutConfirmed(PrimaryClass, SecondaryClass)
- `CanConfirm()` — retourne true si les deux slots sont remplis

**Delegate**
- `FOnLoadoutConfirmed` : broadcast les deux TSubclassOf<AWeaponBase> au PlayerController

## Bug corrigé

**UWidget::Slot shadowing** : UHT error — le paramètre `Slot` dans `SelectWeapon(int32 WeaponIndex, int32 Slot)` entrait en conflit avec `UWidget::Slot`. Renommé en `SlotIndex` dans .h et .cpp.

## Blueprint créé

- WBP_Loadout à `/Game/Mercenaires/UI/` — parent class LoadoutWidget
- Widget layout à faire dans l'éditeur UMG (grille armes + stats + bouton Confirmer)

## Ce qui reste

- Ouvrir WBP_Loadout dans UMG Editor
- Ajouter : grille 5 slots armes, panel stats, bouton Confirmer
- Lier les noms C++ aux BindWidget (si utilisés) ou passer par BP events
