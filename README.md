# LiesOfAP
This is a mod for Lies of P that works with the [multi game randomizer Achipelago](https://archipelago.gg) it is currently being activly developed. All keys, weapons, puppet parts, amulets, ergo items, and materials are randomized with consumables and throwables as filler.

You can get the latest release from [the releases page](https://github.com/Xtruh/LiesOfAP/releases)

# Changes From Base Game
- The Azure Dragon Glaive and other Wu-long collab items are not given to you at the start but are in the item pool
- Phase 7 in the P Organ upgrade is available on new game with a yaml option to add more quartz to the item pool
- Chapter 1 and the original Hotel Krat are available after they normally become unacceptable

# In-Game Client Commands
- `/connect {address:port} {slotname} {password}`
  - Used to connect to the multiworld. Password is optional
- `/disconnect`
  - Disconnect from the multiworld
- `/deathlink`
  - Toggles deathlink
- `/ringlink`
  - Toggles ringlink
- `/hardringlink`
  - Toggles hard ringlink
- `/ratio {new_ratio}`
  - Check the current ring link ratio by using the command with no `new_ratio` otherwise change the current ring link 
  ratio to `new_ratio` Min: 1 Max: 100

# Mod Installation
1. Go to the [Releases page](https://github.com/Xtruh/LiesOfAP/releases) and download the latest version of LiesOfP.zip
2. Locate your lies of P install if on steam should look something like (/Steam/Steamapps/Common/Lies of P)
3. Extract the contents of LiesOfAP.zip directly into Steam/Steamapps/Common/Lies of P

# Updating Guide
1. Go to the latest release and install update.zip
2. Extract the content of update.zip
3. put any new apworld inside your archipelago/custom_worlds folder
4. put any new main.dll inside Lies of P\LiesofP\Binaries\Win64\ue4ss\Mods\LiesOfAP\dlls
5. put any new ~mods or logicmods folder inside Lies of P\LiesofP\Content\paks

# Generating a Game
5. Go to the [Releases page](https://github.com/Xtruh/LiesOfAP/releases) and dowload the latest liesofp.apworld
2. Place liesofp.apworld into Archipelago/custom_worlds.
3. Place all yamls for the seed into Archipelago/Players.
4. Run ArchipelagoGenerate.exe.
5. A zip file will be added to Archipelago/output, which can [hosted on the website](https://archipelago.gg/uploads) or hosted locally with ArchipelagoServer.exe.

# Joining a MultiWorld Game
1. Launch Lies of P and create a new save file.
2. Once in game press **Enter** or **/** to open the text client and connect by typing (/connect Server:Port Slotname password).
3. If entered correctly, a message should appear in the text client saying you are connected.

# Playing Lies of P without the randomizer
1. Move all .pak files located in LiesofP\Content\Paks\ ~mods and LogicMods folders
2. Set "LiesOfAP: 1" to 0 located in LiesofP\Content\Binaries\Win64\ue4ss\Mod\mods.txt

- **Alternatively** you can create copy of the game before installing the mod (not recommended due to the size of the game)

  1. In Steam\steamapps\common, make a copy of your LiesofP folder and rename it "LiesofP_AP" (or something similar).
  2. You can add the new LiesofP.exe as a "Non-Steam Game" in Steam to access it easily from Steam and to maintain Steam Input compatibility (which may be required for some controllers)
  3. Extract the contents of LiesOfAP.zip directly into Steam\Steamapps\Common\LiesofP_AP (or whatever you named it)
