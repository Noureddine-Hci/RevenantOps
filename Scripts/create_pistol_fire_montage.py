import unreal, json

MM_PISTOL_FIRE = '/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire'
AM_PISTOL_FIRE = '/Game/Mercenaires/Anims/Montages/AM_Pistol_Fire'
BP_PISTOL      = '/Game/Mercenaires/Weapons/BP_Pistol'
DEST_DIR       = '/Game/Mercenaires/Anims/Montages'

results = {}

try:
    seq = unreal.load_asset(MM_PISTOL_FIRE)
    if not seq:
        results['error'] = f'{MM_PISTOL_FIRE} not found'
    else:
        results['seq_type'] = type(seq).__name__

        # Creer le montage depuis la sequence
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        factory = unreal.AnimMontageFactory()
        factory.source_animation = seq

        montage = asset_tools.create_asset(
            asset_name='AM_Pistol_Fire',
            package_path=DEST_DIR,
            asset_class=unreal.AnimMontage,
            factory=factory
        )
        if montage:
            unreal.EditorAssetLibrary.save_asset(AM_PISTOL_FIRE, only_if_is_dirty=False)
            results['montage_created'] = AM_PISTOL_FIRE

            # Assigner au BP_Pistol
            bp  = unreal.load_asset(BP_PISTOL)
            unreal.BlueprintEditorLibrary.compile_blueprint(bp)
            cdo = unreal.get_default_object(bp.generated_class())

            with unreal.ScopedEditorTransaction('Assign AM_Pistol_Fire to BP_Pistol') as _:
                bp.modify(); cdo.modify()
                cdo.set_editor_property('CharacterFireMontage', montage)

            unreal.BlueprintEditorLibrary.compile_blueprint(bp)
            unreal.EditorAssetLibrary.save_asset(BP_PISTOL, only_if_is_dirty=False)

            # Verifier
            cdo2 = unreal.get_default_object(bp.generated_class())
            fire = cdo2.get_editor_property('CharacterFireMontage')
            results['pistol_fire_montage'] = str(fire)[:80] if fire else 'None'
            results['status'] = 'ok'
        else:
            results['error'] = 'create_asset returned None'

except Exception as e:
    results['error'] = str(e)[:300]

print(json.dumps(results, indent=2))
