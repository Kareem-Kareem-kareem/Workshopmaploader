# WorkshopMapLoader

A BakkesMod plugin that lets you browse and load Steam Workshop maps inside Rocket League — works with **both Steam and Epic Games**.

![Build](https://github.com/YOUR_USERNAME/WorkshopMapLoader/actions/workflows/build.yml/badge.svg)

---

## Quick start

```bat
git clone https://github.com/YOUR_USERNAME/WorkshopMapLoader.git
cd WorkshopMapLoader
cmake -B build -A x64
cmake --build build --config Release
cmake --install build
```

That's it. CMake fetches the BakkesModSDK automatically — nothing to download manually.

> **Requires:** Visual Studio 2019+ with "Desktop development with C++" workload, and [BakkesMod](https://bakkesmod.com) installed.

---

## Epic Games users

Epic RL has no Steam Workshop tab, but BakkesMod handles it:

1. Use **F2 → Workshop** inside BakkesMod to subscribe to maps. They download to:
   ```
   %APPDATA%\bakkesmod\bakkesmod\data\maps\
   ```
   This plugin scans that folder automatically — no config needed.

2. Or download a `.udk` file manually and paste the path into the **Load File** box.

---

## Opening the window

| Method | How |
|---|---|
| BakkesMod menu | F2 → Plugins → Workshop Map Loader |
| **In-game `~` console** | `wml_open` |
| Toggle | `togglemenu workshopmaploader` |

---

## Console commands

All work from the in-game `` ` `` / `~` console:

| Command | What it does |
|---|---|
| `wml_open` | Open the map browser |
| `wml_close` | Close it |
| `wml_scan` | Re-scan workshop folders |
| `wml_list` | Print all maps with index numbers |
| `wml_load 3` | Load map #3 |
| `wml_load "C:/path/to/map.udk"` | Load by full path |
| `wml_return` | Go back to main menu |

---

## Auto-scanned folders

| Folder | Notes |
|---|---|
| `%APPDATA%\bakkesmod\bakkesmod\data\maps` | BakkesMod workshop downloads — **Epic + Steam** |
| `…/steamapps/workshop/content/252950` | Steam workshop (C/D/E drives) |
| `C:\Program Files\Epic Games\rocketleague\TAGame\CookedPCConsole` | Epic install |
| `C:\Users\Public\Documents\rocketleague_workshop` | Manual drop folder |

You can also type any path into the UI or set `wml_workshop_path` via CVar.

---

## CVars

| CVar | Default | Description |
|---|---|---|
| `wml_enabled` | `1` | Enable/disable the plugin |
| `wml_workshop_path` | *(blank = auto)* | Custom folder to scan |
| `wml_auto_scan` | `1` | Scan on plugin load |

---

## Troubleshooting

**No maps found (Epic)**  
Use BakkesMod's Workshop browser (F2 → Workshop) to download maps, or drop `.udk` files into `%APPDATA%\bakkesmod\bakkesmod\data\maps\`.

**`wml_open` does nothing**  
Make sure the plugin is enabled in F2 → Plugins first.

**Black screen after loading**  
The map may be incompatible with the current RL version. Try a different one.
