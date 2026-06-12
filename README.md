# Platformer Saver

Geode mod for Geometry Dash 2.2074 (Windows) — checkpoint save history for platformer levels.

## Requirements

- [Geode](https://geode-sdk.org/) installed in GD 2.2074
- Dependencies (install in-game): `sabe.persistenceapi`, `geode.node-ids`, `geode.custom-keybinds`

## Build

1. Install [Geode CLI](https://docs.geode-sdk.org/getting-started/) and run `geode sdk install`
2. Set `GEODE_SDK` environment variable (CLI does this automatically)
3. Install Visual Studio 2022 with C++ desktop workload
4. From this folder:

```powershell
geode build
```

The `.geode` package appears in `build/` (or installs automatically if a CLI profile is configured).

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
