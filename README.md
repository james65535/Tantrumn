# Project Tantrumn - Multi-Player Network Game

## Brief
Tantrumn is a racing game (on foot, not car) developed as a solo school project which is part of the [CGSpectrum](https://www.cgspectrum.com/) Game Programming Curriculum. The game was designed to accommodate single player, multi-player split view, and networked multi-player modes. The player(s) can compete against each other or AI bots.

## Role
The design and guidance was provided by the CGSpectrum curriculum along with code examples. My role was to implement the game with scope to make any changes I deemed fit. Changes I introduced spanned the gamut of Unreal Engine capabilities such as a custom [Character Movement Component](https://docs.unrealengine.com/5.2/en-US/understanding-networked-movement-in-the-character-movement-component-for-unreal-engine/) (detailed below), level layouts, Mesh / Audio / Material assets, AI behaviour, and UI elements. The implementation was done primarily in C++ with [Blueprints](https://docs.unrealengine.com/5.2/en-US/introduction-to-blueprints-visual-scripting-in-unreal-engine/) used for prototyping functionality, scripting some aspects of [Widgets](https://docs.unrealengine.com/5.2/en-US/creating-widgets-in-unreal-engine/) used as part of the UI, and level specific gameplay elements.

## Summary of Game Development Features
The game includes player levels, [Kinematics](https://en.wikipedia.org/wiki/Kinematics) test levels, custom [Game Modes](https://docs.unrealengine.com/5.2/en-US/game-mode-and-game-state-in-unreal-engine/) which can match gameplay to the corresponding levels. All gameplay supports network play whether via dedicated server and clients, or local LAN play. Detail on additional implementations included in the game as follows:
* The custom Character Movement Component extends Epic’s implementation to include additional modes of travel. These modes are efficiently replicated via [Bit Flags](https://en.wikipedia.org/wiki/Bit_field) by extending the Network Prediction system used for lag compensation (reducing the [rubberbanding](https://www.dictionary.com/browse/rubberbanding) effect prevalent in many older online games), specifically [FSavedMove](https://docs.unrealengine.com/5.2/en-US/API/Runtime/Engine/GameFramework/FSavedMove_Character/). The end result being minimal [Server Corrections](https://docs.unrealengine.com/5.2/en-US/understanding-networked-movement-in-the-character-movement-component-for-unreal-engine/#customizingnetworkedcharactermovement) for client side movement predictions.
* A stun feature can be invoked via hard landing impacts or from being hit by enemy projectiles.  The stun feature use magnitude of impact determine the duration of stun and strength of controller vibration. Additionally, the stun state controls animation behaviour by triggering corresponding [Animation Montages](https://docs.unrealengine.com/5.2/en-US/animation-montage-in-unreal-engine/).
* A Custom Network `Interp to Movement Component` was developed to support smooth network play even in high latency environments. A major improvement on Unreal's replicated movement feature, especially for moving platforms which players may stand on.
* Similar code was developed for the game's Rescue mechanic which smoothly brings Characters back to a last known good location should they fall off the map.  The feature was developed so that networked clients would experience a smooth transition as a character is retrieved, regardless of whether the rescued character is a local player, server side AI controlled, or another networked player's character.
* A UI framework for game developers was created so that the user interfaces would have a consistent look and feel.  The UI elements are defined by Data Assets and loaded dynamically at runtime if required.
* A custom AI controller was developed to utilise behaviour trees, blackboards, and perception components such as visualisation to navigate a dynamically changing terrain and provide strong competition for human players.

## How to build
1. Clone this repository and open the Tantrumn.uproject file with Unreal Engine.
2. Allow the engine to rebuild the game files when prompted
3. Within Unreal Engine, select tools then generate either Rider or Visual Studio project files.
4. You should be now able to modify and build the project code. All Tantrumn development code is contained within the Source folder.
5. After building the code, a console only server and two networked clients can be run easily via the powershell script: `Source\Tantrumn\RunServerAndTwoClients.ps1`.
6. The powershell script will need to be modified to point to the location of your local Unreal Engine executable.

## Build Notes
* Currently built and test on Unreal Engine 5.2
* While the game supports different game modes such as DodgBall, the race mode on the Vertical Platform map is feature complete, to support local or networked play.
* Each Map's blueprint selects a corresponding Game Mode
* Game Mode's determine the UI to represent to the player, the layouts are defined as Data Assets

## Third Party Attribution
* Character Model and majority of character animations come from the Unreal Engine [Hour of Code](https://www.unrealengine.com/marketplace/en-US/product/unreal-engine-hour-of-code?sessionInvalidated=true) pack
* Some small snippets of code from CGSpectrum’s example content have been re-utilised in this implementation
* The Vertical Race map uses visual assets from the PolyArt3D Content Pack [PLATFORMER - Stylized Cube World Vol.1](https://www.unrealengine.com/marketplace/en-US/product/platformer-stylized-cube-world-vol-1)
* Some Meshes and Materials from Unreal’s [Starter Content Pack](https://docs.unrealengine.com/5.2/en-US/assets-and-content-packs-in-unreal-engine/) have been utilised.
