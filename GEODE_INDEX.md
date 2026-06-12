# Geode index submission

Submit **Platformer Saver** (`wuppy.platformer-saver`) at [geode-sdk.org/mods](https://geode-sdk.org/mods).

## Prerequisites

- [ ] Built `.geode` package (`geode build` or GitHub Actions artifact)
- [ ] Manual testing passed ([TESTING.md](TESTING.md))
- [ ] GitHub release tagged `v1.0.0`

## Submission fields

| Field | Value |
|-------|--------|
| **Mod ID** | `wuppy.platformer-saver` |
| **Name** | Platformer Saver |
| **Version** | v1.0.0 |
| **Developer** | wuppy |
| **Repository** | https://github.com/Joystiick/PlatformerSaves |
| **Tags** | enhancement, gameplay |
| **Logo** | [logo.png](logo.png) (512×512) |
| **Description** | Use [about.md](about.md) |
| **GD version** | 2.2074 (Windows) |
| **Geode version** | 4.9.0+ |

## Dependencies (required)

- `sabe.persistenceapi` v1.1.0
- `geode.node-ids` >= v1.12.0-beta.4
- `geode.custom-keybinds` >= v1.6.0

## Incompatibilities

- `thesillydoggo.moretags` (breaking)

## Credits

Fork of [PlatformerSaves](https://github.com/0x5abe/PlatformerSaves) by Sabe (used with permission).

## After approval

1. Share the Geode index link in your README
2. Monitor issue reports for mid-level pause load edge cases
