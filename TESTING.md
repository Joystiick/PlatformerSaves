# Manual test matrix

| # | Test | Expected |
|---|------|----------|
| 1 | Enter platformer level, touch checkpoint | Auto-save notification (if enabled); `history.json` schema 2 entry |
| 2 | Exit and re-enter level (auto-continue) | Continues from latest save |
| 2b | Set entry-behavior to **ask** | PlayLevelMenuPopup: Continue / New game |
| 3 | Pause → Save | Manual `[M]` history entry with stats |
| 4 | Pause → Load → older save (truncate) | Confirms truncate; newer saves removed |
| 4b | load-behavior **branch** | Fork branch; newer saves kept |
| 5 | Delete / rename in load menu | Entry removed or renamed in history |
| 6 | Export / Import | Package in mod save `exports/{levelId}/` |
| 7 | Complete level | Saves remain on disk |
| 8 | Editor test play | No save buttons / no saves |
| 9 | Practice mode | No save UI |
| 10 | Keybind **K** | Manual save |
| 11 | Keybind **L** | Load history menu |
| 12 | Keybind **R** / pause rewind | Loads previous history slot |
| 13 | Pause → Stats | Deaths, attempts, time shown |
| 14 | Disabled save at same checkpoint | "Already saved at this checkpoint" |
| 15 | speedrun-mode on | No manual save; speedrun branch only in load menu |
| 16 | play-save-sounds | Sound on save/load |

## Compatibility matrix

Test with Platformer Saver enabled alongside each mod (platformer level, checkpoint + load):

| Mod | ID | Notes |
|-----|-----|-------|
| xdBot | `syzzi.xdbot` | Macro input may conflict with save/load state — test pause load after bot segment |
| Happy Textures | `spaghettdev.happy_textures` | Cosmetic; should not affect saves |
| QOLMod | `qolmod.qolmod` | General QoL; verify pause menu buttons still align |
| More Tags | `thesillydoggo.moretags` | **Breaking** incompatibility declared in mod.json |

## Settings migration

- `show-save-notifications` → `show-autosave-notifications` + `show-manual-save-notifications`
