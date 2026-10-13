# Interactive Debugging Guide: GDB & LLDB Pretty-Printers

`nur.h` provides native pretty-printers for **GDB** and **LLDB**. When debugging C applications in terminal or IDEs (VS Code, CLion, Neovim), `NurResult_*` and `NurOption_*` types are displayed with clean, high-level summaries instead of raw struct unions.

---

## 1. Quick Example: Before vs. After

### Before (Standard Debugger Output)
```text
(gdb) print res
$1 = {ok = true, u = {value = 42, error = {code = 0, sys = 0, message = 0x0, context = 0x0, ...}}}
```

### After (With nur.h Pretty-Printers)
```text
(gdb) print res
$1 = Ok(42)

(gdb) print err_res
$2 = Err(NUR_E_OUT_OF_RANGE: "port out of range", at server.c:45)

(gdb) print opt
$3 = Some(100)

(gdb) print none_opt
$4 = None
```

---

## 2. GDB Setup

### Option A: Per-Project `.gdbinit`
In your project root, add to `.gdbinit`:
```gdb
python
import sys
sys.path.insert(0, '/path/to/nur.h/tools')
import nur_gdb_printers
nur_gdb_printers.register_nur_printers()
end
```

### Option B: Global `~/.gdbinit`
Add to `~/.gdbinit`:
```gdb
python
import sys
sys.path.append('/usr/local/share/nur/tools') # or repo path
import nur_gdb_printers
nur_gdb_printers.register_nur_printers()
end
```

---

## 3. LLDB Setup (macOS / Clang)

Add to `~/.lldbinit`:
```lldb
command script import /path/to/nur.h/tools/nur_lldb_printers.py
```

Now in LLDB:
```text
(lldb) frame variable res
(NurResult_i32) res = Ok(42)
```

---

## 4. Visual Studio Code (`launch.json`)

If using `cppdbg` (GDB) in VS Code, add to your `.vscode/launch.json`:

```json
{
  "name": "(gdb) Launch",
  "type": "cppdbg",
  "request": "launch",
  "program": "${workspaceFolder}/build/my_app",
  "setupCommands": [
    {
      "description": "Enable nur.h Pretty Printers",
      "text": "source ${workspaceFolder}/tools/nur_gdb_printers.py",
      "ignoreFailures": false
    }
  ]
}
```
