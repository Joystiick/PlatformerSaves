# Platformer Saver

Geode mod for Geometry Dash 2.2081 (Windows) — checkpoint save history for platformer levels.

Repository: https://github.com/Joystiick/PlatformerSaves

**Download:** [v1.5.0 release](https://github.com/Joystiick/PlatformerSaves/releases/tag/v1.5.0)

**Issues:** https://github.com/Joystiick/PlatformerSaves/issues

## Requirements

- [Geode](https://geode-sdk.org/) 5.7.1+ installed in GD 2.2081
- Dependencies (install in-game): `sabe.persistenceapi`, `geode.node-ids`

## Build

**Quick setup:** run `scripts\ready-status.ps1` to see what's missing.

1. Install [Geode CLI](https://docs.geode-sdk.org/getting-started/) (`winget install GeodeSDK.GeodeCLI`) and run `geode sdk install`
2. Add a profile: `geode profile add --name default "C:\Program Files (x86)\Steam\steamapps\common\Geometry Dash"`
3. Install dependencies: `geode project check` (from this folder)
4. Install **Visual Studio 2022 Build Tools** with C++ — **right-click `scripts\install-vs-admin.bat` → Run as administrator**
5. Build: `powershell -File scripts\setup-build.ps1`

Or manually:

```powershell
geode build
```

The `.geode` package appears in `build/` (or installs automatically if a CLI profile is configured).

CI builds on every push to `master` (Windows-only workflow). See [PLAYTEST.md](PLAYTEST.md) and [GEODE_INDEX.md](GEODE_INDEX.md).

## Controls

- **K** — quick manual save (in platformer levels)
- **L** — open save history menu
- **R** — rewind 1–3 history slots (configurable)
- **Pause → Save / Load / Rewind / Stats**

## Save data

Stored under Geode's mod save directory per level:

- `history.json` — save timeline metadata (schema v2)
- `slot{N}.psf` — full run snapshots
- `exports/{levelId}/` — export/import packages

## Platforms

Windows is supported today. **Mac and Android ports are planned** but not included in v1.5.0.

## License

See LICENSE (GPL from upstream PlatformerSaves). Pusab font: Flat-it freeware license in `resources/fonts/`.
