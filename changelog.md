## v1.5.0

* Save history schema v2: names, stats, branches, cap, delete/rename
* Split notifications: auto-save, manual save, continue
* Pause UI: last-saved label, stats popup, rewind button, refresh on save
* Load behavior: truncate (default) or branch fork
* Entry behavior: auto-continue or ask (PlayLevelMenuPopup)
* Speedrun mode: no manual save, separate branch filter
* Keybinds: **L** load menu, **R** rewind (1–3 slots configurable)
* Save/load sound effects setting
* Export/import save packages from load menu
* Auto-save toggle setting
* 512×512 logo for Geode index; issues link in about

## v1.0.4

* Transparent save/load sprites from user assets (cropped, centered, sized for pause menu)
* Fix pause-menu button alignment to match vanilla left-button-menu layout (49px height)

## v1.0.3

* Fix platformer checkpoints not registering on GD 2.2081 (save buttons stayed greyed out)
* New save/load button sprites; use them directly instead of double-wrapping in CircleButtonSprite

## v1.0.2

* Fix crash on level load (GD 2.2081): use `GameObject::resetMID()` instead of stale hardcoded memory offset

## v1.0.1

* Fix Geode 5 `mod.json` format: `incompatibilities` object and dependency version strings (mod failed to load on 5.7.1)

## v2.0.0

* Rebranded to **Platformer Saver** (`wuppy.platformer-saver`)
* Unlimited per-level save history with `history.json` timeline
* Auto-save on every platformer checkpoint (always on)
* Pause menu **Save** + **Load Save** always visible in normal platformer mode
* Auto-continue from latest save on level entry (no entry popup)
* Save history menu with truncate-on-load for older saves
* New settings: save/continue notifications, confirm load truncate, history sort
* Editor level saves removed; practice mode unsupported
* Custom logo, save/load button art, and PUSAB font
