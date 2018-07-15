# FogOfWar
This is a Fog of War plugin for Unreal Engine 4. It is based on the [Fog of War Tutorial](https://forums.unrealengine.com/community/community-content-tools-and-tutorials/26436-tutorial-fog-of-war?55650-Tutorial-Fog-Of-War=) by [Isvulfe](https://forums.unrealengine.com/member/26710-isvulfe) on the Unreal Engine forums.

Right now the code is nearly identical to that of the thread, except for some minor changes to get it running on UE 4.19 and turning it into a plugin. This will change in the future as will use modify this in order to make it a better fit for my other projects.

## Compiling
1. Save/clone into the `Plugins/` directory at the project root
2. Compile. You will need to right click on the `.uproject` in your project and select `Generate Visual Studio project files` so VS is aware of the new source files.
3. Add `FogOfWar` to `PublicDependencyModuleNames` in your `.Build.cs`

The best (and AFAIK only) documentation is the [original thread](https://forums.unrealengine.com/community/community-content-tools-and-tutorials/26436-tutorial-fog-of-war?55650-Tutorial-Fog-Of-War=)
