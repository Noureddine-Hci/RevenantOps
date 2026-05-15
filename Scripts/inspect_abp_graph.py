import unreal

# Trouver le nom exact de la variable arme sur le character
bp = unreal.load_asset('/Game/Characters/BP_ThirdPersonCharacter')
if not bp:
    bp = unreal.load_asset('/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter')

if bp:
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    print('=== Proprietes weapon/equip du character ===')
    for attr in dir(cdo):
        low = attr.lower()
        if any(k in low for k in ['weapon', 'equip', 'arm', 'current', 'active', 'held', 'inventory']):
            try:
                val = getattr(cdo, attr)
                print(f'  {attr} = {val}')
            except:
                print(f'  {attr} [inaccessible]')
else:
    print('Character BP not found')

# Lister les noeuds dans l'Event Graph de l'ABP
abp = unreal.load_asset('/Game/Mercenaires/Anims/ABP_Mercenaire')
print('\n=== Noeuds ABP (get_nodes_of_class K2Node_Event) ===')
try:
    nodes = abp.get_nodes_of_class(unreal.K2Node_Event)
    for n in nodes:
        print(f'  {n.get_node_title(unreal.NodeTitleType.FULL_TITLE)}')
except Exception as e:
    print(f'  Erreur: {e}')

print('\n=== Noeuds ABP (get_nodes_of_class K2Node_VariableGet) ===')
try:
    nodes = abp.get_nodes_of_class(unreal.K2Node_VariableGet)
    for n in nodes:
        print(f'  {n.get_node_title(unreal.NodeTitleType.FULL_TITLE)}')
except Exception as e:
    print(f'  Erreur: {e}')
