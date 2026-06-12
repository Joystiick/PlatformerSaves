# Play-test setup

Run from PowerShell in the project folder.

## 1. Install dependencies in Geometry Dash

Open GD → Geode → Download and enable:

- **sabe.persistenceapi**
- **geode.node-ids**

Or install the release `.geode` from [GitHub Releases](https://github.com/Joystiick/PlatformerSaves/releases/tag/v1.0.4).

## 2. Build and install the mod

```powershell
geode build
```

If `geode build` succeeds, the mod installs to your configured profile automatically.

Alternatively download the **Build Output** artifact from GitHub Actions (Windows workflow) after pushing to `master`.

## 3. Launch Geometry Dash

```powershell
geode profile run
```

## 4. Run the test matrix

Follow every row in [TESTING.md](TESTING.md). Priority:

1. Checkpoint auto-save
2. Re-enter level → auto-continue
3. Pause → Load → older save → truncate confirm
4. Pause → Save → new history entry
5. Level complete → saves kept
6. Editor / practice → no save UI
7. Keybind **K**

## Preflight check

```powershell
powershell -File scripts/preflight-playtest.ps1
```
