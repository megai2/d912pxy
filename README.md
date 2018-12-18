# d912pxy
DirectX9 to DirectX12 API proxy for Guild Wars 2

-What this thing does? 

-Makes games that use DirectX9, use DirectX12 instead, without any change to game code. 

 Especially Guild Wars 2. You can try use it in other games, maybe it will work.
 
 
 This project is not yet finished, expect bugs, crashes, hungs, stalls and all other fun(or not so fun) stuff!
 
 Current state: v0.9 alpha
   
# Requirements

DirectX12 capable GPU, with 12.1 feature level and 3+ Gb VRAM.

16 Gb RAM

(will be more precise, based on feedback and optimizations in code)
 
# How to use

1. Set "Resolution" in graphics options to "Fullscreen windowed"/"Windowed".
2. Download latest release
3. Unpack it into game folder
4. Run the game

# How to remove

1. Delete d3d9.dll from bin64 folder located in game root folder. This will stop loading d912pxy at game start. 
2. If you want clean deletion, manually delete all d3d9*.dll files and P7x64*.dll files
3. Delete d912pxy folder
4. Done

# Troubleshooting

## Case 1
  Things are loading part-by-part.
  Something are missing at start.
  
**Solution**
  I'm not supplying any shaders with release. 
  This means, that shaders will be recompiled on first use, and then load much faster.
  Still, shaders loaded in realtime on demand and in async manner, what triggers missing parts/fragments.
  Most time this is not critical.
  Fixing this problem need some investigation and on TODO list.
  
## Case 2
  Game crashes/Error dialog related to d912pxy pops out.
  
**Solution**

  Do not ask game support about this crashes/errors!
  
  If you game crashes without d912pxy, don't ask about this here, cause d912pxy do zero modifications to game data.
  
  First be shure you set resolution to Windowed fullscreen or windowed.
  
  Second be shure that game runs without d912pxy.
  
  Third update your driver and directx9!
  
  (link to dx9 https://www.microsoft.com/ru-ru/download/details.aspx?id=34429)
  
  Then, if you still here, post your issue on github with 
  
    1) Log file form P7logs
    2) Crash.dmp if you have it
    3) ....
    4) PROFIT!

## Case 3

  Visual errors
 
**Solution**

  1. In bin64 subfolder, rename d3d9.dll to d3d9_.dll(or something else), rename d3d9ps.dll to d3d9.dll
  2. Run the game, reproduse visual error.
  3. Delete contents of d912pxy/shaders/cso and d912pxy/shaders/hlsl folders
  4 In bin64 subfolder, rename d3d9.dll to d3d9ps.dll, rename d3d9_.dll to d3d9.dll
  5. Run the game again. Wait for shaders to recompile.
  6. If error fixed, if you want(and more over if you can!) - send the newest created files in d912pxy/shaders/bugs to this github. goto 8.
  7. If error is not fixed, post your issue on github with a description on how to reproduse visual error
  8. ...
  9. PROFIT!
