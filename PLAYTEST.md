# Play-test setup

Run from PowerShell in the project folder.

## 1. Install dependencies in Geometry Dash

Open GD → Geode → Download and enable:

- **sabe.persistenceapi**
- **geode.node-ids**

Or install the release `.geode` from [GitHub Releases](https://github.com/Joystiick/PlatformerSaves/releases/tag/v1.5.0).

## 2. Build and install the mod

```powershell
geode build
```

If `geode build` succeeds, the mod installs to your configured profile automatically.

Alternatively download the **Build Output** artifact from GitHub Actions (Windows workflow) after pushing to `master`.

### Beta / CI workflow

Pushes to `master` trigger the Windows build workflow automatically. To rebuild manually without a push, dispatch workflow **Build Geode Mod** from the [Actions tab](https://github.com/Joystiick/PlatformerSaves/actions) (workflow id `294380150`).

## 3. Launch Geometry Dash

```powershell
geode profile run
```

## 4. Run the test matrix

Follow every row in [TESTING.md](TESTING.md). Priority:

1. Checkpoint auto-save
2. Re-enter level → auto-continue (and ask mode)
3. Pause → Load → older save → truncate / branch
4. Pause → Save → new history entry
5. Rewind + Stats + export/import
6. Level complete → saves kept
7. Editor / practice → no save UI
8. Keybinds **K**, **L**, **R**

## Preflight check

```powershell
powershell -File scripts/preflight-playtest.ps1
```
