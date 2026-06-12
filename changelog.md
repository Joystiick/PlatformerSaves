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

## v1.1.1

* Fixed a bug that caused incorrect song to play when loading a save
* Fixed a bug that caused a crash on mac when exiting a platformer level (thanks to hiimjasmine00)
* Added an option to automatically start a new game if there isn't a save file for a particular level

## v1.1.0

* Fixed multiple bugs
* Added a new save indicator
