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
7. Patch the values at runtime.
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

## 8. Runtime Patching

Once the relevant fields are identified, patch the values in memory.

Example target values:

| Field | Example Patched Value |
| :--- | :--- |
| Health | `9999` |
| Max health | `9999` |
| Damage | `100` |

After patching, resume execution.

If the correct fields were modified, the hero should survive enemy attacks and defeat all enemies.

## 9. Verification

A successful solution should show:

- The hero no longer dies quickly.
- The hero deals significantly more damage.
- Enemy health decreases rapidly.
- The final result is a victory message.

Verification should be done through both runtime behavior and memory inspection.

## 10. Lessons Learned

This challenge demonstrates several basic reverse-engineering concepts:

- Runtime memory values can explain program behavior.
- Strings can be useful anchors during analysis.
- Object fields can often be inferred from nearby memory and access patterns.
- Static analysis and dynamic debugging are stronger when used together.
- Debug builds are easier to analyze than optimized release builds.
- Runtime patching should be performed only in software you own or are authorized to analyze.

## 11. Limitations

This is a toy challenge, not a real game security system.

It intentionally avoids:

- Anti-debugging
- Obfuscation
- Packing
- Anti-cheat mechanisms
- Kernel-level protections
- Networked state validation

The purpose is to teach basic analysis workflow, not to simulate a hardened commercial target.

## 12. Responsible Use

This write-up applies only to this educational program.

**Do not** use these techniques to cheat in online games, bypass protections, or modify software without permission.
