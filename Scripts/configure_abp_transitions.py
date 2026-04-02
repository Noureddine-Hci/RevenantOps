import unreal, json

ABP_PATH = '/Game/Mercenaires/Anims/ABP_Mercenaire'

# Strategie: le perso a TOUJOURS une arme dans RevenantOps.
# Transition Locomotion -> Armed : can_enter_transition = True (toujours actif)
# Transition Armed -> Locomotion : can_enter_transition = False (jamais retour)
# => Le state machine va en Armed immediatement et y reste.

try:
    abp = unreal.load_asset(ABP_PATH)
    graphs = abp.get_animation_graphs()
    info = {}

    # graph_16 = Locomotion -> Armed (on veut True)
    # graph_17 = Armed -> Locomotion (on veut False, deja le cas)
    g16 = graphs[16]
    g17 = graphs[17]

    info['g16_name'] = g16.get_name()
    info['g17_name'] = g17.get_name()

    # Recuperer les TransitionResult nodes
    tr16 = g16.get_graph_nodes_of_class(unreal.AnimGraphNode_TransitionResult)
    tr17 = g17.get_graph_nodes_of_class(unreal.AnimGraphNode_TransitionResult)

    info['tr16_count'] = len(tr16)
    info['tr17_count'] = len(tr17)

    # Tenter de modifier la transition Locomotion->Armed (graph_16) pour toujours etre True
    if tr16:
        n16 = tr16[0]
        try:
            # Lire le struct Node actuel
            current = n16.get_editor_property('Node')
            info['node16_before'] = str(current)

            # Creer un nouveau struct avec can_enter_transition = True
            new_node = unreal.AnimNode_TransitionResult()
            new_node.can_enter_transition = True

            with unreal.ScopedEditorTransaction('Set Locomotion->Armed always true') as _:
                n16.modify()
                n16.set_editor_property('Node', new_node)

            # Verifier
            after = n16.get_editor_property('Node')
            info['node16_after'] = str(after)
            info['set16_ok'] = True
        except Exception as e:
            info['set16_err'] = str(e)[:200]

    # Compiler et sauvegarder
    try:
        unreal.BlueprintEditorLibrary.compile_blueprint(abp)
        unreal.EditorAssetLibrary.save_asset(ABP_PATH, only_if_is_dirty=False)
        info['compile_save'] = 'ok'
    except Exception as e:
        info['compile_err'] = str(e)[:150]

    print(json.dumps(info, indent=2))

except Exception as e:
    print(json.dumps({'error': str(e)[:300]}))
