#!/bin/bash
# Stop hook — RevenantOps
# Rappel commit + log session

echo "=== Fin de session RevenantOps ==="

# Vérifier changements non committés
MODIFIED=$(git diff --name-only 2>/dev/null)
STAGED=$(git diff --staged --name-only 2>/dev/null)
UNTRACKED=$(git ls-files --others --exclude-standard 2>/dev/null | head -10)

if [ -n "$MODIFIED" ] || [ -n "$STAGED" ] || [ -n "$UNTRACKED" ]; then
    echo ""
    echo "⚠️  RAPPEL : Tu as des changements non committés !"
    if [ -n "$STAGED" ]; then
        echo "  Staged (prêts à committer):"
        echo "$STAGED" | while read -r f; do echo "    $f"; done
    fi
    if [ -n "$MODIFIED" ]; then
        echo "  Modifiés:"
        echo "$MODIFIED" | while read -r f; do echo "    $f"; done
    fi
    if [ -n "$UNTRACKED" ]; then
        echo "  Nouveaux fichiers:"
        echo "$UNTRACKED" | while read -r f; do echo "    $f"; done
    fi
    echo ""
    echo "  → Committer avant de fermer : rtk git add . && rtk git commit -m '[J/N] type(scope): desc'"
else
    echo "✅ Tout est committé. Bonne session !"
fi

echo "======================================"
exit 0
