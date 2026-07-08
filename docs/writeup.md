\# Reverse Engineering Write-up



\## 1. Objective



The goal of this challenge is to analyze a small C++ x64 auto-battle simulation and patch the hero's runtime combat values so the hero wins the fight.



\## 2. Environment



\- OS: Windows

\- Build: x64 Debug

\- Debugger: x64dbg

\- Decompiler: Ghidra

\- IDE: Visual Studio



\## 3. Initial Behavior



When the program runs without modification, the hero loses because the enemies have stronger combat stats.



\## 4. Static Analysis with Ghidra



Relevant areas to inspect:



\- Entity combat logic

\- Health reads and writes

\- Damage calculation

\- Attack flow

\- Object field layout



\## 5. Dynamic Analysis with x64dbg



Useful steps:



1\. Run the program in x64dbg.

2\. Search for the ASCII string `HERO`.

3\. Inspect nearby memory.

4\. Set breakpoints on combat-related functions.

5\. Observe health and damage reads/writes during combat.



\## 6. Runtime Patch



The intended patch is to modify the hero's combat values during execution.



Example target values:



| Field | Patched Value |

|---|---:|

| Health | 9999 |

| Max Health | 9999 |

| Damage | 100 |



\## 7. Result



After patching the values, the hero survives and defeats all enemies.



\## 8. Lessons Learned



This challenge demonstrates how primitive object fields can be located, inspected, and modified in a running x64 process.



\## 9. Disclaimer



This write-up applies only to this educational toy program.

