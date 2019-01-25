# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

d912pxy is a way to make games that use DirectX 9, use DirectX 12 instead, without changing any game code.

This is specifically designed for Guild Wars 2.
You can try use it in other games, maybe it will work.
 
This project is in early development, expect bugs, crashes, hangs, stalls and all other fun stuff.
 
# Showcase

Profiling shows that API overhead for d912pxy is up to 70% less then for plain DirectX9.
Real performance differs on scene and on hardware.

Like running mistlock sanctuary in Guild Wars 2 on i7-7700/GTX960 results in:

```
@6.3k batches, all max, 1080p

dx9
  min FPS: 5,5
  max FPS: 21,5
dx12
  min FPS: 17,6
  max FPS: 26,6
boost:
  min FPS: +220%
  max FPS: +23%
```

DX12:

https://cdn.discordapp.com/attachments/477036595019644928/524540609105756160/unknown.png 


DX9:

https://cdn.discordapp.com/attachments/477036595019644928/524541036626837504/unknown.png

# Requirements

You will obviously need a DirectX12 capable GPU, with 12.1 feature level and 3GB+ VRAM.
16 GB of RAM is recommended.

Requirements will be more precise in the future based on feedback and code optimization.

# Installing

1. Set your resolution to `Fullscreen Windowed` or `Windowed` mode.
2. Disable all overlay software.
3. Download latest release [from here](https://github.com/megai2/d912pxy/releases).
4. Unpack it into the game folder, so d912pxy folder is in game root folder. (Next to Gw2-64.exe)
5. Run the game.

# Uninstalling

You can uninstall by simply deleting the `d912pxy` folder, `bin64/d3d9.dll` and `bin64/P7x64.dll`.
Don't remove any other files other then that.

# Known bugs

-Screenshot functionality not working

# Troubleshooting

## Case 1
  Things are loading part-by-part.
    
**Solution**

  d912pxy loads shaders in async manner, cause there is no efficient way to load them instantly.
  
  This can create some visual errors, but performs much better in terms of performance.
  
  
## Case 2
  Game crashes/hangs
  
**Solution**

  Do not ask game support about this crashes!
  
  If you game crashes without d912pxy, don't ask about this here, cause d912pxy do zero modifications to game data.
  
  Be shure that game runs without d912pxy!
  
  Next update your driver and directx9.
  
  (link to dx9 https://www.microsoft.com/ru-ru/download/details.aspx?id=34429)
  
  Then, if you still here, post your issue on github with 
  
    1. Log file form P7logs
    2. Crash.dmp and d912pxy_crash.txt if you have it    
    
 If you asked to run debug version do this
 
   0. Debug version writes ton of data, do not run it for long time!
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
  8. If error is not fixed, post your issue on github with a description on how to reproduse visual error  

# Support developer

WMZ 442298672293