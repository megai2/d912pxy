# d912pxy
DirectX9 to DirectX12 API proxy for Guild Wars 2

-What this thing does? 

-Makes games that use DirectX9, use DirectX12 instead, without any change to game code. 

 Especially Guild Wars 2. You can try use it in other games, maybe it will work.
 
 
 This project is not yet finished, expect bugs, crashes, hungs, stalls and all other fun(or not so fun) stuff!
 
 Current state: v0.9 alpha
 
# Showcase

Profiling shows that api overhead for d912pxy is 40-20% less then for plain directx9.
Real performance differs on scene.

DX12:

https://cdn.discordapp.com/attachments/477036595019644928/524540609105756160/unknown.png 


DX9:

https://cdn.discordapp.com/attachments/477036595019644928/524541036626837504/unknown.png

   
# Requirements

DirectX12 capable GPU, with 12.1 feature level and 3+ Gb VRAM.

16 Gb RAM

(will be more precise, based on feedback and optimizations in code)
 
# How to use

1. Set "Resolution" in graphics options to "Fullscreen windowed"/"Windowed".
2. Download latest release
3. Unpack it into game folder
4. Run d912pxy/install.bat
5. Run the game

# How to remove

1. Run d912pxy/remove.bat
2. Delete d912pxy folder
3. Done

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
  
    1. Log file form P7logs
    2. Crash.dmp if you have it    
    
 If you asked to run debug version do this
 
   1. Run d912pxy/remove.bat
   2. Run d912pxy/install_debug.bat
   3. Run game
   4. Send the log file or/and Crash.dmp to github

## Case 3 

  Visual errors
 
**Solution**

  1. Run d912pxy/remove.bat
  2. Run d912pxy/install_ps.bat
  3. Run the game, reproduse visual error.
  4. Run d912pxy/clean_shaders.bat
  5. Run d912pxy/remove.bat
  6. Run d912pxy/install.bat
  7. Run the game again. Wait for shaders to recompile.
  8. If error fixed, if you want(and more over if you can!) - send the newest created files in d912pxy/shaders/bugs to this github. goto 8.
  9. If error is not fixed, post your issue on github with a description on how to reproduse visual error  
