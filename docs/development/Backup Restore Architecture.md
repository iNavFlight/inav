# Backup, Restore & Settings Migration — Architecture

> **Note:** This document describes the internals of the INAV Configurator's backup/restore and settings migration system. It is intended for Configurator developers, not end users. For user-facing documentation, see the [INAV docs](https://github.com/iNavFlight/inav/blob/master/docs/Backup%20and%20Restore.md).

## Architecture Overview

```
User → Firmware Flasher Tab → STM32.connect(onCliReady) → CLI mode
                                                              ↓
                                          BackupRestore.captureCliDiffAll()
                                                              ↓
                                           Save to file, prune old backups
                                                              ↓
                                               STM32 flash (DFU or serial)
                                                              ↓
                                              onFlashComplete() callback
                                                              ↓
                                      Version check → Migration check → UI overlay
                                                              ↓
                                      User confirms → Poll for FC reconnect
                                                              ↓
                        BackupRestore.performRestore() or performRestoreWithMigration()
                                                              ↓
                                    saveAndReboot() or abortRestore()
```

## Files

| File | Purpose |
|------|---------|
| `js/backup_restore.js` | Core backup/restore module — CLI protocol, file I/O, auto-backup |
| `js/migration/migration_handler.js` | Version migration engine — profile chaining, line transformation |
| `js/migration/7_to_8.json` | Migration profile: INAV 7.x → 8.0 |
| `js/migration/8_to_9.json` | Migration profile: INAV 8.0 → 9.0 |
| `tabs/firmware_flasher.js` | Flash integration — auto-backup trigger, restore UI, version gating |
| `tabs/firmware_flasher.html` | Overlays and buttons for backup/restore/migration UI |
| `src/css/tabs/firmware_flasher.css` | Overlay styles |
| `js/protocols/stm32.js` | STM32 flash protocol — `onCliReady` callback, DFU timeout fix |
| `js/main/main.js` | Electron main process — IPC handlers for file operations |
| `js/main/preload.js` | IPC bridge — exposes backup API to renderer |
| `locale/en/messages.json` | All i18n translation keys |

## Adding a New Migration Profile

When a new major INAV version is released (e.g. 9.x → 10.x), create a migration profile:

### Step 1: Create the JSON profile

Create `js/migration/9_to_10.json`:

```json
{
    "fromVersion": "9",
    "toVersion": "10",
    "description": "INAV 9.x → 10.0 migration profile",

    "commandRenames": {
        "old_command_name": "new_command_name"
    },

    "settingRenames": {
        "old_setting_name": "new_setting_name"
    },

    "valueReplacements": {
        "setting_name": {
            "OLD_VALUE": "NEW_VALUE"
        }
    },

    "removed": [
        "deleted_setting_1",
        "deleted_setting_2"
    ],

    "settingPatternMappings": [
        {
            "pattern": "^regex_matching_setting_names$",
            "valueMap": { "old_numeric_id": "new_numeric_id" },
            "description": "Human-readable description of remapping"
        }
    ],

    "warnings": [
        "Human-readable warning about settings whose semantics changed and need manual review."
    ]
}
```

### Step 2: Register the profile

In `js/migration/migration_handler.js`, add the import and append to the array:

```javascript
import profile_9_to_10 from './9_to_10.json';

const MIGRATION_PROFILES = [
    profile_7_to_8,
    profile_8_to_9,
    profile_9_to_10,   // ← add here
];
```

The migration engine automatically chains profiles. A 7.x → 10.x upgrade will apply all three profiles in sequence (7→8, 8→9, 9→10).

### How to determine what goes into a migration profile

Compare CLI settings between the old and new firmware version:

1. **Removed settings**: Run `diff all` on old and new firmware with default settings. Settings present in old but not in new → add to `removed`
2. **Renamed settings**: Check INAV release notes and source code for renamed settings → add to `settingRenames`
3. **Renamed commands**: Check for CLI command name changes (e.g. `profile` → `control_profile`) → add to `commandRenames`
4. **Value replacements**: Check for enum value name changes → add to `valueReplacements`
5. **Pattern mappings**: Check for bulk ID renumbering (OSD elements, etc.) → add to `settingPatternMappings`
6. **Warnings**: Check for settings where the meaning/units changed but name stayed the same → add to `warnings`

Key INAV source files to check:
- `src/main/fc/settings.yaml` — all CLI settings definitions
- `src/main/fc/cli.c` — CLI command implementations
- Release notes on GitHub

## Migration Profile Schema Reference

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `fromVersion` | `string` | Yes | Source major version number (e.g. `"9"`) |
| `toVersion` | `string` | Yes | Target major version number (e.g. `"10"`) |
| `description` | `string` | Yes | Human-readable description |
| `commandRenames` | `object` | Yes | Maps old CLI command names to new names. Applied to any token in the command line. E.g. `"profile" → "control_profile"` transforms `profile 2` to `control_profile 2` |
| `settingRenames` | `object` | Yes | Maps old `set` setting names to new names. Only applies to `set <name> = <value>` lines |
| `valueReplacements` | `object` | Yes | Maps setting names to value replacement objects `{ "oldval": "newval" }`. Only modifies the value portion after `=` |
| `removed` | `string[]` | Yes | List of setting names to remove entirely. Lines with `set <name> = ...` matching these are dropped |
| `settingPatternMappings` | `array` | Yes | Array of pattern-based value remappings for settings matching a regex. Each entry has `pattern` (regex), `valueMap` (object), `description` (string) |
| `warnings` | `string[]` | Yes | Warning messages about semantic changes requiring manual review. Displayed in migration preview overlay |

## Existing Migration Profile Details

### 7_to_8.json (INAV 7.x → 8.0)

| Category | Changes |
|----------|---------|
| **Command renames** | `profile` → `control_profile` |
| **Value replacements** | `gps_provider`: `UBLOX7` → `UBLOX` |
| **Removed settings** (18) | `control_deadband`, `cpu_underclock`, `disarm_kill_switch`, `dji_workarounds`, `fw_iterm_limit_stick_position`, `gyro_anti_aliasing_lpf_type`, `gyro_hardware_lpf`, `gyro_main_lpf_type`, `gyro_use_dyn_lpf`, `inav_use_gps_no_baro`, `inav_use_gps_velned`, `ledstrip_visual_beeper`, `max_throttle`, `nav_auto_climb_rate`, `nav_manual_climb_rate`, `osd_stats_min_voltage_unit`, `pidsum_limit`, `pidsum_limit_yaw` |
| **Pattern mappings** | `osd_custom_element_N_type`: IDs remapped 4→9, 5→16, 6→7, 7→10 |
| **Warnings** | `nav_fw_wp_tracking_accuracy` semantics changed: was arbitrary tracking response, now distance in meters |

### 8_to_9.json (INAV 8.0 → 9.0)

| Category | Changes |
|----------|---------|
| **Command renames** | `controlrate_profile` → `use_control_profile` |
| **Setting renames** | `mixer_pid_profile_linking` → `mixer_control_profile_linking`, `osd_pan_servo_pwm2centideg` → `osd_pan_servo_range_decadegrees` |
| **Value replacements** | None |
| **Removed settings** | None |
| **Pattern mappings** | None |
| **Warnings** | Position estimator defaults changed (`w_z_baro_v`, `inav_w_z_gps_p`, `inav_w_z_gps_v`). `ahrs_acc_ignore_rate` default changed 20→15 |

## Migration Engine Internals

### Profile chaining

`buildMigrationChain(fromVersion, toVersion)` selects all profiles where `profileFrom >= fromMajor` and `profileTo <= toMajor`, sorted by `fromVersion`. A 7.x → 9.x migration applies both 7→8 and 8→9 profiles in sequence.

### Line processing

Each non-comment, non-empty line passes through every profile in the chain. For each profile, transformations are applied in this order:

1. Command renames (any token in the line)
2. Removed settings (line dropped if `set <name>` matches)
3. Setting renames (`set <name>` replacement)
4. Value replacements (value after `=` replaced)
5. Setting pattern mappings (regex-matched settings with value remapping)

### Missing profile detection

`hasMissingProfiles()` returns `true` when the number of profiles in the chain is fewer than the number of major version steps. The UI shows a warning but still allows restore — some settings may fail.

## Edge Cases Handled

1. **Stale FC version after flash**: Real FC version is queried via `MSP_FC_VERSION` after connect, not the cached value
2. **DFU mode (no MSP)**: `FC.CONFIG` null checks prevent crashes when connected in DFU mode
3. **DFU timeout**: UI unlock and progress label update on timeout (no permanent lock)
4. **Local firmware files**: `localFirmwareLoaded` flag prevents stale dropdown version from triggering wrong migration
5. **Backup pruning with mixed versions**: Sort by timestamp portion, not full filename
6. **Multi-step migration**: 7.x → 9.x automatically chains 7→8 + 8→9 profiles
7. **Missing migration profiles**: Warning shown but restore allowed — graceful degradation
8. **Version detection from backup**: Parsed from backup header (`# Version: X.Y.Z`), not from FC state

## i18n Keys

All backup/restore/migration translation keys in `locale/en/messages.json`:

### Backup status
| Key | Text |
|-----|------|
| `backupRestoreStatusEnteringCli` | Entering CLI |
| `backupRestoreStatusReadingConfig` | Reading configuration via CLI |
| `backupRestoreStatusSavingFile` | Saving backup file... |
| `backupRestoreStatusExitingCli` | Exiting CLI mode |
| `backupRestoreBackupSaved` | Backup saved $1 |
| `backupRestoreAutoBackupSaved` | Auto-backup saved to $1 |
| `backupRestoreBackupComplete` | Backup complete |
| `backupRestoreBackupCancelled` | Backup cancelled |
| `backupRestoreBackupFailed` | Backup failed |

### Restore status
| Key | Text |
|-----|------|
| `backupRestoreStatusConnecting` | Connecting to flight controller |
| `backupRestoreStatusRestoringConfig` | Restoring configuration |
| `backupRestoreStatusRestoringProgress` | Restoring... $1 / $2 |
| `backupRestoreStatusSaving` | Saving configuration |
| `backupRestoreRestoreComplete` | Configuration restored. Flight controller is rebooting. |
| `backupRestoreRestoreCancelled` | Restore cancelled. |
| `backupRestoreRestoreFailed` | Restore failed. |

### Auto-restore UI
| Key | Text |
|-----|------|
| `backupRestoreAutoRestoreConfirm` | Restore confirmation prompt |
| `backupRestoreAutoRestoreWaiting` | Waiting for FC to reboot after flash |
| `backupRestoreAutoRestoreYes` | Yes, restore settings |
| `backupRestoreAutoRestoreNo` | No, keep current settings |
| `backupRestoreAutoRestoreWaitingPort` | Waiting for port $1 to reconnect |
| `backupRestoreDowngradeNoAutoRestore` | Major downgrade warning |
| `backupRestoreFlashCompleteBackupSaved` | Backup saved (local firmware, no restore offer) |
| `backupRestoreMigrationApplied` | Migration applied: $1 → $2 ($3 changes) |
| `backupRestoreMigrationWarningsHeader` | Migration Warnings: |

### Migration preview
| Key | Text |
|-----|------|
| `migrationPreviewTitle` | Settings Migration Required |
| `migrationPreviewSubtitle` | Conversion explanation |
| `migrationPreviewRemovedHeader` | Removed Settings: |
| `migrationPreviewRenamedSettingsHeader` | Renamed Settings: |
| `migrationPreviewRenamedCommandsHeader` | Renamed Commands: |
| `migrationPreviewValueReplacementsHeader` | Value Replacements: |
| `migrationPreviewSettingRemappingsHeader` | Setting Remappings: |
| `migrationPreviewContinue` | Continue with migration |
| `migrationPreviewCancel` | Cancel restore |
| `migrationMissingProfileWarning` | Missing profile warning |

### Error messages
| Key | Text |
|-----|------|
| `backupRestoreErrorTitle` | Restore Errors Detected |
| `backupRestoreErrorText` | Error explanation |
| `backupRestoreErrorAbort` | Abort |
| `backupRestoreErrorSave` | Save anyway |
