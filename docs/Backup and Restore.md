# Backup and Restore

INAV Configurator can automatically back up your configuration before flashing firmware and offer to restore it afterwards. When upgrading across major versions (e.g. 7.x → 8.x → 9.x), settings are automatically migrated to the new firmware format.

For manual CLI-based backup and restore, see the [CLI documentation](Cli.md#backup-via-cli).

## Automatic Backup & Restore During Firmware Flash

### What happens automatically

1. **Before flashing** (with or without Full Chip Erase enabled): Your current CLI configuration (`diff all`) is automatically captured and saved to the backup directory.
2. **After flashing**: Depending on the situation, the Configurator offers to restore your settings if Full Chip Erase was enabled:

| Scenario | Behavior |
|----------|----------|
| **Patch update** (e.g. 8.0.0 → 8.0.1) | Auto-restore offered immediately |
| **Minor update** (e.g. 8.0.0 → 8.1.0) | Auto-restore offered immediately |
| **Major upgrade** (e.g. 7.x → 8.x) with migration profile available | Migration preview shown — confirm to restore with converted settings |
| **Major upgrade** without migration profile | Warning shown — restore still possible but some settings may fail |
| **Major downgrade** (e.g. 9.x → 7.x) | Auto-restore blocked — manual restore only (settings may be incompatible) |
| **Local firmware file** (loaded from disk) | No auto-restore offered — backup is still saved |
| **Flash without Full Chip Erase** | Backup taken, no restore offered |

### Migration Preview

When updating across major versions (e.g. 7.x → 9.x), the Configurator shows a **migration preview overlay** before restoring. This lists:

- **Removed settings** — settings that no longer exist in the new firmware (will be skipped)
- **Renamed settings** — settings whose name changed (automatically converted)
- **Renamed commands** — CLI commands that were renamed (automatically converted)
- **Value replacements** — setting values that changed meaning (automatically converted)
- **Setting remappings** — numeric IDs that were renumbered (automatically converted)
- **Warnings** — settings whose semantics changed and require manual review

You can review all changes before confirming or cancelling the restore.

Multi-step migrations are handled automatically. For example, a 7.x → 9.x upgrade applies migration profiles in sequence (7→8, then 8→9).

## Manual Backup & Restore

The Firmware Flasher tab provides three buttons:

- **Backup Config** — saves your current CLI configuration to a file (opens save dialog)
- **Restore Config** — loads a backup file and restores it to your flight controller
  - If the backup is from a different major version, the migration preview is shown first
  - If no migration profile exists for the version gap, a warning is shown but you can still proceed
- **Open Backups Folder** — opens the backup directory in your file manager

For CLI-based backup and restore procedures, see [Backup via CLI](Cli.md#backup-via-cli) and [Restore via CLI](Cli.md#restore-via-cli).

## Backup File Location

Backups are stored in your OS-specific application data directory:

| OS | Path |
|----|------|
| **Windows** | `%APPDATA%/inav-configurator/backups/` |
| **macOS** | `~/Library/Application Support/inav-configurator/backups/` |
| **Linux** | `~/.config/inav-configurator/backups/` |

Use the **Open Backups Folder** button in the Firmware Flasher tab to open this directory.

## Backup File Naming

| Type | Format |
|------|--------|
| Auto-backups | `UPDATE_inav_backup_{version}_{board}_{YYYY-MM-DD_HHMMSS}.txt` |
| Manual backups | `inav_backup_{version}_{board}_{YYYY-MM-DD_HHMMSS}.txt` |

Auto-backups are pruned automatically — only the 10 most recent are kept. Files you rename are never pruned.

## Backup File Format

Backup files are plain-text CLI dumps with a metadata header:

```
# INAV Backup
# Version: 8.0.0
# Board: SPEEDYBEEF405V4
# Date: 2026-04-11T10:30:00.000Z
# Craft: MyQuad
#
# INAV/SPEEDYBEEF405V4 8.0.0 Apr  1 2026 / 12:00:00 (abc1234)
# GCC-13.2.1
# ...
set gyro_main_lpf_hz = 110
set acc_hardware = AUTO
...
```

You can open and edit these files with any text editor.

## Restore Error Handling

If errors occur during restore (e.g. unknown settings, invalid values):

- An error dialog shows the affected lines
- You can choose:
  - **Save anyway** — saves the successfully applied settings and reboots
  - **Abort** — discards all changes and exits CLI mode

## Tips

- **Always flash with Full Chip Erase** when upgrading to a new version. This ensures clean defaults and triggers automatic backup and restore.
- **Review migration previews carefully** — especially the warnings section, which highlights settings whose meaning may have changed.
- **Keep manual backups** before major upgrades. While auto-backup handles this, having an extra copy in a known location gives peace of mind.
- **Use `diff` over `dump`** for backups. The `diff` format only stores settings that differ from defaults, which makes restoring safer across versions. The auto-backup feature already uses `diff all`.
