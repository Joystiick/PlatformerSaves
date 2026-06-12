# Platformer Saver

Geode mod for Geometry Dash 2.2074 (Windows) — checkpoint save history for platformer levels.

Repository: https://github.com/Joystiick/PlatformerSaves

## Requirements

- [Geode](https://geode-sdk.org/) installed in GD 2.2074
- Dependencies (install in-game): `sabe.persistenceapi`, `geode.node-ids`, `geode.custom-keybinds`

## Build

1. Install [Geode CLI](https://docs.geode-sdk.org/getting-started/) (`winget install GeodeSDK.GeodeCLI`) and run `geode sdk install`
2. Add a profile: `geode profile add --name default "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"`
3. Install **Visual Studio 2022 Build Tools** with the **Desktop development with C++** workload (required for `cl.exe`)
4. Install **CMake** (`winget install Kitware.CMake`) if not already on PATH
5. From this folder:

```powershell
geode build
```

The `.geode` package appears in `build/` (or installs automatically if a CLI profile is configured).

CI builds on every push to `master` (Windows-only workflow). See [PLAYTEST.md](PLAYTEST.md) and [GEODE_INDEX.md](GEODE_INDEX.md).

## Controls

- **K** — quick manual save (in platformer levels)
- **Pause → Save** — manual save (new history entry)
- **Pause → Load** — open save timeline

## Save data

Stored under Geode's mod save directory per level:

- `history.json` — save timeline metadata
- `slot{N}.psf` — full run snapshots

## License

See LICENSE (GPL from upstream PlatformerSaves). Pusab font: Flat-it freeware license in `resources/fonts/`.
