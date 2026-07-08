\# Auto-Battle Reverse Engineering Challenge



A small C++ x64 reverse-engineering challenge for practicing runtime memory analysis, static analysis with Ghidra, and dynamic debugging with x64dbg.



\## Overview



This project contains a simple auto-battle simulation.  

By default, the hero is underpowered and loses the fight.



The goal is to analyze the program, identify the relevant runtime combat fields, patch the hero's values in memory, and make the hero win.



\## Learning Goals



\- Understand how C++ objects appear in memory

\- Locate strings and nearby object fields in a running process

\- Use x64dbg for dynamic memory analysis

\- Use Ghidra for static analysis and cross-reference inspection

\- Understand how health and damage values are read and written at runtime



\## Tools Used



\- Visual Studio

\- x64dbg

\- Ghidra

\- Windows x64 Debug build



\## Build Instructions



1\. Open the solution in Visual Studio.

2\. Select `x64` and `Debug`.

3\. Disable compiler optimizations if they are enabled.

4\. Build and run the project.



\## Controls



\- `SPACE` - Print current battle status

\- `H` - Reveal the next reverse-engineering hint

\- `ESC` - Exit the simulation



\## Challenge Rules



Try to solve the challenge without revealing all hints immediately.  

The most specific hints are intentionally hidden behind the hint system.



\## Full Write-up



See \[`docs/writeup.md`](docs/writeup.md).



\## Disclaimer



This project is an educational reverse-engineering lab using a self-contained toy program.



It is not intended for cheating in commercial games, bypassing anti-cheat systems, or modifying third-party software without permission.

