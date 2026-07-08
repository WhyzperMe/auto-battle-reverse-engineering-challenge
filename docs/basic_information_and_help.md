# Reverse Engineering Write-up

## Spoiler Notice

> This document describes one possible solution path for the Auto-Battle Reverse Engineering Challenge.
> 
> Try solving the challenge before reading this write-up.

## 1. Objective

The goal of this challenge is to analyze a small C++ x64 auto-battle simulation and modify the hero's runtime combat values so the hero wins the fight.

By default, the hero is weaker than the enemies and loses during normal execution.

## 2. Environment

Recommended analysis environment:

| Component | Recommendation |
| :--- | :--- |
| Operating system | Windows |
| Architecture | x64 |
| Build configuration | Debug |
| IDE | Visual Studio |
| Dynamic analysis | x64dbg |
| Static analysis | Ghidra |

Compiler optimizations should be disabled for the clearest analysis experience.

## 3. Initial Program Behavior

When the program starts, it initializes one hero and multiple enemies.

The hero has lower combat stats than the enemies. During the battle, all living enemies eventually become aggressive and attack the hero. Without modification, the hero is defeated.

The key observation is that the hero loses because of runtime state, not because of an unavoidable scripted loss condition.

## 4. Analysis Strategy

The challenge can be solved with a combination of static and dynamic analysis.

A practical workflow is:

1. Run the program normally and observe the output.
2. Identify useful strings such as `HERO`.
3. Use `x64dbg` to locate the string or nearby object memory.
4. Observe changes to health values during combat.
5. Use Ghidra to inspect combat-related functions.
6. Identify the fields responsible for health, max health, and damage.
7. Patch the values at runtime or hard-patch the executable.
8. Resume execution and verify the result.

## 5. Static Analysis with Ghidra

Load the compiled executable into Ghidra and let it perform the initial analysis.

Important areas to inspect:

- Entity-related functions
- Attack logic
- Damage calculation
- Health modification
- References to readable strings
- Reads and writes to integer fields near the hero object

Relevant conceptual functions include:

```text
Entity::Attack
Entity::TakeDamage
Hero update logic
Enemy update logic
Game loop logic
```

Depending on the exact build settings and symbol availability, names may differ or be unavailable. In that case, use string references and control-flow analysis to locate the relevant logic.

## 6. Dynamic Analysis with x64dbg

Start the executable in `x64dbg`.

Useful steps:

- Run the program until the battle starts.
- Search memory for the ASCII string `HERO`.
- Inspect memory near the located string.
- Observe nearby integer values that match visible combat stats.
- Set breakpoints on combat-related instructions.
- Step through attack and damage handling.
- Watch which fields are read or written when the hero takes damage.

> **Note:** The program includes a progressive hint system. Press `H` during execution to reveal hints one at a time. The early hints are intentionally general, while later hints provide more direct runtime information.

## 7. Locating Combat Fields

The important hero fields are:

| Field | Purpose |
| :--- | :--- |
| Health | Current hero health |
| Max health | Maximum displayed hero health |
| Damage | Base damage used when the hero attacks |

Because the program is compiled as C++, exact object layout can depend on compiler settings, build configuration, platform, and code changes.

For that reason, do not rely blindly on hardcoded offsets. Confirm all fields dynamically in the debugger.

## 8. Runtime Patching (Memory)

Once the relevant fields are identified, patch the values in memory while the program is running.

Example target values:

| Field | Example Patched Value |
| :--- | :--- |
| Health | `9999` |
| Max health | `9999` |
| Damage | `100` |

After patching, resume execution.

If the correct fields were modified, the hero should survive enemy attacks and defeat all enemies. Note that this only lasts until the program is closed.

## 9. Persistent Patching (Hard Patching ASM Values)

If you want the modifications to persist permanently without needing to re-patch the memory every time, you can modify the actual assembly instructions in the executable file using `x64dbg`.

Because this is a Debug build, the compiler initializes the `Hero` struct on the stack using immediate values in the assembly code (e.g., `100` is `0x64` in hex, and `8` is `0x08`).

Steps to create a hard-patched executable:

1. Open the executable in `x64dbg` and set a breakpoint at `main` or the `Hero` constructor.
2. Step through the initialization code until you reach the instructions that set up the `CombatStats` struct.
3. Look for assembly instructions writing immediate values to the stack or registers, such as:
   - `mov dword ptr [rbp+...], 64` (Sets Health to 100)
   - `mov dword ptr [rbp+...], 8`  (Sets Damage to 8)
4. Right-click the instruction and select **Assemble** (or press `Ctrl + A`).
5. Change the immediate value in the instruction:
   - For Health: Change `64` to `270F` (which is `9999` in hex).
   - For Damage: Change `8` to `64` (which is `100` in hex) or `270F`.
6. `x64dbg` will automatically adjust the instruction length and pad the remaining bytes with `NOP` instructions to prevent breaking the code flow.
7. Repeat this process for the `Max Health` initialization.
8. Once all desired ASM instructions are modified, open the **Patches** window (`Ctrl + P` or `File` > `Patches`).
9. Verify your changes in the list and click **Patch File**.
10. Save the new executable (e.g., as `auto_battle_patched.exe`). 

When you run this newly generated executable, the hero will start with the modified stats by default.

## 10. Verification

A successful solution should show:

- The hero no longer dies quickly.
- The hero deals significantly more damage.
- Enemy health decreases rapidly.
- The final result is a victory message.

Verification should be done through both runtime behavior and memory inspection.

## 11. Lessons Learned

This challenge demonstrates several basic reverse-engineering concepts:

- Runtime memory values can explain program behavior.
- Strings can be useful anchors during analysis.
- Object fields can often be inferred from nearby memory and access patterns.
- Static analysis and dynamic debugging are stronger when used together.
- Debug builds are easier to analyze than optimized release builds.
- Runtime patching should be performed only in software you own or are authorized to analyze.

## 12. Limitations

This is a toy challenge, not a real game security system.

It intentionally avoids:

- Anti-debugging
- Obfuscation
- Packing
- Anti-cheat mechanisms
- Kernel-level protections
- Networked state validation

The purpose is to teach basic analysis workflow, not to simulate a hardened commercial target.

## 13. Responsible Use

This write-up applies only to this educational program.

**Do not** use these techniques to cheat in online games, bypass protections, or modify software without permission.
