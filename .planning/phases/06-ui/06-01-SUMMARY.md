# Phase 06 — UI & Menus : Résumé d'exécution

**Complété le :** 2026-03-23
**Fichiers créés :** 6

## Ce qui a été fait

### TitleScreenWidget.h/.cpp
- Boutons : `PlayButton` (BindWidgetOptional), `QuitButton` (BindWidgetOptional)
- `OnPlayClicked()` → appelle event BP `BP_OnPlayPressed` (transition de level en Blueprint pour éviter crash MCP)
- `OnQuitClicked()` → `UKismetSystemLibrary::QuitGame()`

### GameOverWidget.h/.cpp
- Text blocks : `FinalScoreText`, `TotalKillsText`, `BestComboText` (BindWidgetOptional)
- `ShowResults(int32 Score, int32 Kills, int32 BestCombo)` — formate et affiche
- `ReplayButton` → OpenLevel vers Lvl_ThirdPerson
- `LeaderboardButton` → notifie PlayerController

### LeaderboardWidget.h/.cpp + LeaderboardSaveGame.h

**FScoreEntry** struct : Score, Kills, BestCombo, Date (FString)

**LeaderboardSaveGame**
- Hérite de USaveGame, slot `"RevenantOps_Leaderboard"`, `UserIndex 0`
- TArray<FScoreEntry> Entries (max 10)

**LeaderboardWidget**
- `AddScore(FScoreEntry)` — insère, trie descending, garde top 10, sauvegarde
- `LoadScores()` / `SaveScores()` via UGameplayStatics::LoadGameFromSlot/SaveGameToSlot
- `BackButton` → retour

## Blueprints créés

- WBP_TitleScreen (`/Game/Mercenaires/UI/`) — parent TitleScreenWidget
- WBP_GameOver (`/Game/Mercenaires/UI/`) — parent GameOverWidget
- WBP_Leaderboard (`/Game/Mercenaires/UI/`) — parent LeaderboardWidget

## Ce qui reste

### Setup éditeur UMG obligatoire

**WBP_TitleScreen**
- Ajouter : Canvas → Text "REVENANTOPS" (titre), Button "JOUER" (nom: PlayButton), Button "QUITTER" (nom: QuitButton)
- Les noms doivent matcher les BindWidgetOptional en C++

**WBP_GameOver**
- Ajouter : FinalScoreText, TotalKillsText, BestComboText (TextBlock), ReplayButton, LeaderboardButton

**WBP_Leaderboard**
- Ajouter : ScrollBox pour la liste, EntryText pattern, BackButton

## Note importante

`BP_OnPlayPressed` est un BlueprintImplementableEvent dans TitleScreenWidget. Il faut l'implémenter dans WBP_TitleScreen (Event Graph) avec un `OpenLevel` vers la level de jeu, ou vers WBP_Loadout selon le flow.
