#!/bin/bash
# SessionStart hook — RevenantOps
# Affiche le contexte de session au démarrage

echo "=== RevenantOps — Session Context ==="

# Branche courante
BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
if [ -n "$BRANCH" ]; then
    echo "Branche: $BRANCH"
    echo ""
    echo "Derniers commits:"
    git log --oneline -5 2>/dev/null | while read -r line; do
        echo "  $line"
    done
fi

# Fichiers modifiés non committés
MODIFIED=$(git diff --name-only 2>/dev/null)
STAGED=$(git diff --staged --name-only 2>/dev/null)
if [ -n "$MODIFIED" ] || [ -n "$STAGED" ]; then
    echo ""
    echo "⚠️  Changements non committés :"
    if [ -n "$STAGED" ]; then
        echo "  Staged:"
        echo "$STAGED" | while read -r f; do echo "    $f"; done
    fi
    if [ -n "$MODIFIED" ]; then
        echo "  Modifiés:"
        echo "$MODIFIED" | while read -r f; do echo "    $f"; done
    fi
fi

# Rappel phase active (lire CLAUDE.md)
CLAUDE_MD="CLAUDE.md"
if [ -f "$CLAUDE_MD" ]; then
    PHASE=$(grep -E "^## Etat Phase [0-9]+" "$CLAUDE_MD" | tail -1 | sed 's/## //')
    if [ -n "$PHASE" ]; then
        echo ""
        echo "Phase active : $PHASE"
    fi
fi

echo ""
echo "Skills disponibles : /gate-check /smoke-check /sprint-plan /team-combat /balance-check /code-review /bug-triage /perf-profile"
echo "Agents : unreal-specialist, ue-blueprint-specialist, ue-umg-specialist, ue-gas-specialist"
echo "======================================"
exit 0
