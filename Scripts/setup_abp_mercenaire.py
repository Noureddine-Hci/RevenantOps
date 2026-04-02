import unreal, json

# Approche: ajouter bIsArmed comme variable ABP via les NewVariables
# puis configurer la transition via les noeuds du graphe

ABP_PATH = '/Game/Mercenaires/Anims/ABP_Mercenaire'

try:
    abp = unreal.load_asset(ABP_PATH)

    # Verifier les variables existantes et methodes disponibles
    info = {}

    # Essayer d'acceder aux NewVariables du blueprint
    try:
        new_vars = abp.get_editor_property('NewVariables')
        info['new_vars_count'] = len(new_vars) if new_vars else 0
        info['new_vars'] = [str(v.var_name) for v in new_vars] if new_vars else []
    except Exception as e:
        info['new_vars_err'] = str(e)[:100]

    # Essayer d'acceder aux graphes
    try:
        graphs = unreal.BlueprintEditorLibrary.get_blueprint_function_names(abp)
        info['graphs'] = list(graphs)
    except Exception as e:
        info['graphs_err'] = str(e)[:100]

    # Methodes dispo sur abp
    methods = [m for m in dir(abp) if ('var' in m.lower() or 'graph' in m.lower() or 'member' in m.lower())]
    info['methods'] = methods[:20]

    print(json.dumps(info, indent=2))

except Exception as e:
    print(json.dumps({'error': str(e)[:200]}))
