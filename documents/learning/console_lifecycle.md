# Console Application Lifecycle & Pause Mechanism

// Copyright (c) 2026 Omid Teimory. All Rights Reserved

## The Instant Closing Bug
When launching GUI-less console binaries (`sim.exe`) directly from Windows File Explorer by double-clicking, Windows creates a host console window that automatically terminates and closes the instant `main()` returns `0`.

## Solution: `system("pause")`
To ensure output reports remain visible on Windows:
1. `system("pause")` is called prior to `return 0;` at the end of `main()`.
2. Input buffer flushing using `safeCin()` prevents leftover trailing `\n` characters from skipping the pause prompt.
