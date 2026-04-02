import unreal, json

ABP_UNARMED = '/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed'
DEST        = '/Game/Mercenaires/Anims/ABP_Mercenaire'

try:
    # Dupliquer ABP_Unarmed -> ABP_Mercenaire (meme skeleton, meme structure de base)
    success = unreal.EditorAssetLibrary.duplicate_asset(ABP_UNARMED, DEST)
    if success:
        unreal.EditorAssetLibrary.save_asset(DEST, only_if_is_dirty=False)
        print(json.dumps({'status': 'ok', 'abp': DEST,
                          'note': 'Duplicate de ABP_Unarmed - skeleton et state machine copies'}))
    else:
        print(json.dumps({'status': 'error', 'msg': 'duplicate_asset returned False'}))

except Exception as e:
    print(json.dumps({'status': 'error', 'error': str(e)[:200]}))
