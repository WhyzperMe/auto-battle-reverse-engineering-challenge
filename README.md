# Auto-Battle Reverse Engineering Challenge

A small C++ x64 reverse-engineering challenge for practicing static analysis, runtime memory inspection, and controlled memory patching in a self-contained training program.

The project simulates a simple auto-battle scenario. By default, the hero is intentionally underpowered and loses the fight. The goal is to analyze the program, identify the relevant runtime combat values, and modify them during execution so the hero can win.

## Purpose

This repository is intended as an educational reverse-engineering lab.

It demonstrates:

- How simple C++ objects can appear in memory
- How runtime values such as health and damage can be located
- How dynamic analysis with x64dbg complements static analysis with Ghidra
- How string references and memory layout can help during analysis
- How controlled patching can change program behavior in a toy environment

## Tools

Recommended tools:

- Visual Studio
- x64dbg
- Ghidra
- Windows x64 Debug build

## Build Instructions

1. Open the project in Visual Studio.
2. Select the `x64` platform.
3. Select the `Debug` configuration.
4. Make sure compiler optimizations are disabled.
5. Build and run the project.

The project is designed for a Windows console environment.

## Controls

During the simulation:

| Key | Action |
|---|---|
| `SPACE` | Print the current battle status |
| `H` | Reveal the next reverse-engineering hint |
| `ESC` | Exit the simulation |

## Challenge Objective

Run the program normally and observe the battle.

The hero will lose by default. Your task is to:

1. Analyze the binary.
2. Locate the hero object or its relevant combat fields.
3. Identify the health, max health, and damage values.
4. Patch the values at runtime.
5. Resume execution and confirm that the hero wins.

The program includes a progressive hint system. The most specific hints are intentionally hidden at startup and must be revealed manually.

## Repository Structure

```text
auto-battle-reverse-engineering-challenge/
├─ docs/
│  └─ writeup.md
├─ src/
│  └─ main.cpp
├─ .gitignore
├─ LICENSE.md
├─ README.md
├─ SECURITY.md
├─ auto-battle-reverse-engineering-challenge.slnx
├─ auto-battle-reverse-engineering-challenge.vcxproj
└─ auto-battle-reverse-engineering-challenge.vcxproj.filters
