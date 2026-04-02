import unreal, json

CHAR_BP  = '/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter'
ABP_MERC = '/Game/Mercenaires/Anims/ABP_Mercenaire'

try:
    abp = unreal.load_asset(ABP_MERC)
    bp  = unreal.load_asset(CHAR_BP)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo  = unreal.get_default_object(bp.generated_class())
    mesh = cdo.get_components_by_class(unreal.SkeletalMeshComponent)[0]

    with unreal.ScopedEditorTransaction('Switch to ABP_Mercenaire') as _:
        bp.modify(); cdo.modify(); mesh.modify()
        mesh.set_editor_property('AnimClass', abp.generated_class())

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(CHAR_BP, only_if_is_dirty=False)
    print(json.dumps({'status': 'ok', 'anim_class': 'ABP_Mercenaire'}))

except Exception as e:
    print(json.dumps({'status': 'error', 'error': str(e)[:200]}))
