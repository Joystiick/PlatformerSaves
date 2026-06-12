# Manual test matrix

| # | Test | Expected |
|---|------|----------|
| 1 | Enter platformer level, touch checkpoint | Auto-save notification; `history.json` gains entry |
| 2 | Exit and re-enter level | Auto-continues from latest save |
| 3 | Pause → Save | New manual history entry |
| 4 | Pause → Load → pick older save | Confirms truncate; newer saves removed; state restored |
| 5 | Complete level | Saves remain on disk |
| 6 | Editor test play | No save buttons / no saves |
| 7 | Practice mode | No save UI |
| 8 | Keybind K | Manual save works |
