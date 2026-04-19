#!/bin/bash
# PreToolUse hook — valide les commits git
# Vérifie le format des messages de commit RevenantOps
#
# Input: JSON stdin { "tool_name": "Bash", "tool_input": { "command": "git commit ..." } }

INPUT=$(cat)

# Parser la commande
if command -v jq >/dev/null 2>&1; then
    COMMAND=$(echo "$INPUT" | jq -r '.tool_input.command // empty')
else
    COMMAND=$(echo "$INPUT" | grep -oE '"command"[[:space:]]*:[[:space:]]*"[^"]*"' | sed 's/"command"[[:space:]]*:[[:space:]]*"//;s/"$//')
fi

# Seulement pour git commit
if ! echo "$COMMAND" | grep -qE '^(rtk )?git[[:space:]]+commit'; then
    exit 0
fi

# Extraire le message de commit
MSG=$(echo "$COMMAND" | grep -oE '(-m[[:space:]]+"|"[^"]+")' | head -1 | sed 's/-m[[:space:]]*"//;s/"$//')

WARNINGS=""

# Vérifier le préfixe [J] ou [N]
if [ -n "$MSG" ]; then
    if ! echo "$MSG" | grep -qE '^\[(J|N)\]'; then
        WARNINGS="$WARNINGS\nFORMAT: Le message doit commencer par [J] ou [N] (ex: [J] feat(gameplay): ...)"
    fi

    # Vérifier le format type(scope)
    if ! echo "$MSG" | grep -qE '^\[(J|N)\] (feat|fix|docs|refactor|chore|test)\('; then
        WARNINGS="$WARNINGS\nFORMAT: Utiliser le format [J/N] type(scope): desc — types valides: feat, fix, docs, refactor, chore, test"
    fi
fi

# Vérifier les fichiers stagés pour des problèmes courants
STAGED=$(git diff --cached --name-only 2>/dev/null)

# Warning si on committe des fichiers .uasset (binaires UE5)
UASSET_FILES=$(echo "$STAGED" | grep -E '\.uasset$|\.umap$' 2>/dev/null)
if [ -n "$UASSET_FILES" ]; then
    WARNINGS="$WARNINGS\nASSETS: Tu commites des assets binaires UE5 — as-tu annoncé à l'autre dev ?"
    echo "$UASSET_FILES" | while read -r f; do
        WARNINGS="$WARNINGS\n  → $f"
    done
fi

# Warning si on committe sur main directement
BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
if [ "$BRANCH" = "main" ] || [ "$BRANCH" = "master" ]; then
    echo "BLOQUÉ: Ne pas committer directement sur $BRANCH — utilise ta branche J ou Nrd/" >&2
    exit 2
fi

# Afficher les warnings (non-bloquants)
if [ -n "$WARNINGS" ]; then
    echo -e "=== Validation Commit RevenantOps ===$WARNINGS\n=====================================" >&2
fi

exit 0
