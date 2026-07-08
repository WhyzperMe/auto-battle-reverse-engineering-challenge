## Appendix A: Practical Walkthrough (Quick Solution)

If you prefer a direct, hands-on approach, follow this step-by-step guide to locate and hard-patch the combat values using Ghidra and x64dbg.

### 1. Locating Hero Stats in Ghidra

1. Load the executable into **Ghidra** and allow it to complete its initial auto-analysis.
2. Open the **Search** menu (`Ctrl + F`) and search for the hex value `0x64` (which is `100` in decimal, the hero's starting health). 
3. Ensure you select **Instruction Operands** as the search scope.
4. Review the search results and navigate to the reference located inside the `main` function. 
5. In the Ghidra Decompiler window, you will see the C pseudo-code initializing the hero's stats, confirming the assignment: `local_1548.heroHealth = 100;`.

### 2. Hard-Patching Hero Stats in x64dbg

To apply this change permanently, switch to **x64dbg** and modify the assembly instructions directly.

1. Note the exact byte sequence for the health initialization in Ghidra's assembly view. For the hero's health, the bytes are: `c7 44 24 20 64 00 00 00`.
2. In x64dbg, open the pattern search dialog (`Ctrl + Shift + B` or right-click -> **Search for** -> **Current module** -> **Pattern**).
3. Paste the hex sequence (`c7 44 24 20 64 00 00 00`) into the **Hex** field and execute the search. Double-click the result to navigate to the instruction in the CPU view.
4. Press `Space` to open the **Assemble** dialog and modify the immediate value:
   - Change `mov dword ptr ss:[rsp+20],64` to `mov dword ptr ss:[rsp+20],0x270F` (Hex for `159999`).
   - *Note: You can use larger values like `270FF` (159999), but ensure the hex representation is correct.*
5. Locate the instruction for the hero's damage directly below it.
   - Change `mov dword ptr ss:[rsp+24],08` to `mov dword ptr ss:[rsp+24],0x270F` (Hex for `159999`).
6. **Warning:** Do not attempt to patch the `speed` variable using this method. Speed is a `float`, which requires converting the decimal value to its IEEE 754 hex representation (e.g., `1.0f` is `3F800000`) before patching.
7. Open the **Patches** window (`Ctrl + P`), select all modified instructions, and click **Patch File**. 
8. Save the new executable (e.g., `hero_patched.exe`) and run it. The hero will now start with the modified stats and win the battle.

### 3. Bonus: Patching Enemy Stats

You can apply the exact same workflow to weaken the enemies. By default, enemies have `180` HP (`0xB4` in hex) and `20` damage (`0x14` in hex).

1. In Ghidra, search for the instruction operand `0xB4` and locate the initialization inside `main`.
2. Copy the byte sequence for the enemy health initialization: `c7 44 24 2c b4 00 00 00`.
3. In x64dbg, use the pattern search (`Ctrl + Shift + B`) to find this hex sequence and navigate to it.
4. Assemble (`Space`) and change the health value:
   - Change `mov dword ptr ss:[rsp+2C],B4` to `mov dword ptr ss:[rsp+2C],1`. This sets every enemy's HP to `1`.
5. Locate the enemy damage instruction below it and change the damage value:
   - Change `mov dword ptr ss:[rsp+30],14` to `mov dword ptr ss:[rsp+30],2`. This sets their damage to `2`.
6. Open the **Patches** window (`Ctrl + P`), apply the changes, and save the file.

When you run the newly patched executable, the enemies will spawn with severely reduced stats, making the battle trivial:

```text
Spawning 5 enemies...
  + [Monster_1] spawned at (6.0, 0.0, 14.5) [HP: 1, DMG: 2]
  + [Monster_2] spawned at (-8.4, 0.0, 9.2) [HP: 1, DMG: 2]
  + [Monster_3] spawned at (-1.4, 0.0, 8.2) [HP: 1, DMG: 2]
  + [Monster_4] spawned at (-1.4, 0.0, -7.9) [HP: 1, DMG: 2]
  + [Monster_5] spawned at (-13.7, 0.0, 1.5) [HP: 1, DMG: 2]
