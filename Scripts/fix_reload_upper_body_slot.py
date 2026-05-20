import unreal

MON = '/Game/Mercenaires/Anims/Montages'
montage = unreal.load_asset(f'{MON}/AM_Pistol_Reload')

# Trouver le bon nom de propriete pour les slots
print('=== Proprietes AnimMontage contenant "slot" ===')
for attr in dir(montage):
    if 'slot' in attr.lower():
        try:
            val = getattr(montage, attr)
            print(f'  {attr} = {val}')
        except:
            try:
                val = montage.get_editor_property(attr)
                print(f'  get_editor_property({attr}) = {val}')
            except Exception as e:
                print(f'  {attr} -> erreur: {str(e)[:80]}')

print('\n=== Proprietes AnimMontage contenant "anim" ===')
for attr in dir(montage):
    if 'anim' in attr.lower() and 'slot' not in attr.lower():
        try:
            val = getattr(montage, attr)
            print(f'  {attr} = {val}')
        except:
            pass
