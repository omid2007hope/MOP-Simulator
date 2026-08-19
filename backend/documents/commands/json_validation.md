# Command Reference: JSON Config Validation

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## Validating JSON Databases
`data/targets.json` and `data/projectiles.json` are loaded dynamically at runtime by `ConfigLoader`.

To verify JSON syntax via PowerShell before launching `sim.exe`:

```powershell
Get-Content ./data/targets.json | ConvertFrom-Json
Get-Content ./data/projectiles.json | ConvertFrom-Json
```
If no output/error is returned, the JSON formatting is valid.
