Ran command: `make clean`
Ran command: `cmake --build build --target clean`
Ran command: `Get-Command mingw32-make, g++, cmake, make -ErrorAction SilentlyContinue`

In your environment (MinGW / MSYS2), `make` is installed as **`mingw32-make`**, and `cmake` is not in your system PATH.

You can use either of the following working options in PowerShell:

### Option 1: Use `mingw32-make clean`

```powershell
mingw32-make clean
```

---

### Option 2: Use Native PowerShell Command (No extra tools needed)

This deletes the compiled object (`build/`) and executable (`bin/`) folders directly:

```powershell
Remove-Item -Recurse -Force build, bin
```

Ran command: `mingw32-make clean`
Ran command: `mingw32-make clean`

To compile and rebuild your program using `mingw32-make`:

<!--!!!-->

### 1. Build / Compile the Project

```powershell
mingw32-make
```

This compiles all updated `.cpp` files into object files inside `build/` and generates `bin/sim.exe`.

---

### 2. Clean and Rebuild in One Command

If you want to clean old object files and rebuild completely from scratch in one line:

```powershell
mingw32-make clean; mingw32-make
```

---

### 3. Run the Compiled Program

Once built, run your simulator executable:

```powershell
.\bin\sim.exe
```

---

### 4. Build and Run Unit Tests

To compile and execute the test suite:

```powershell
mingw32-make test
```
