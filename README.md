# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

d912pxy is a way to make games that use DirectX 9, use DirectX 12 instead, without changing any game code.

This is specifically designed for Guild Wars 2.
You can try to use it in other games, it might work.
 
This project is in early development, expect bugs, crashes, hangs, stalls and some other fun stuff.
 
[![Build status](https://ci.appveyor.com/api/projects/status/gs8drlb0goyp6h28?svg=true)](https://ci.appveyor.com/project/megai2/d912pxy)

**WARNING: USING OF THIS SOFTAWRE IS ENTIRELY AT YOUR OWN RISK!**
 
# Showcase

Profiling shows that API overhead for d912pxy is up to 70% less than for plain DirectX9.
Real performance differs depending on the scene and the hardware.

Benching results on i7-7700/GTX960:

For CPU bound scenario
(running Mistlock Sanctuary)

```
@6.3k batches, all max, 1080p, v0.9.5.8a

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

For GPU bound scenario
(1h fractal runs)

```
@all max, 1080p, v0.9.6.2a

16,4% to 2,5% less frame time

calculated based on API profiling & metrics data

This is affected by performance recording, should be better in normal situation
```

DX12:

https://cdn.discordapp.com/attachments/477036595019644928/539417113593380865/unknown.png 

DX9:

https://cdn.discordapp.com/attachments/477036595019644928/539417612501647360/unknown.png

Video by reddit u/moriz0, running WvW in 4k https://www.youtube.com/watch?v=RQAB7Ma20Ow&

# Requirements

**Minimal**:
1. Windows 10 ([about wine](https://github.com/megai2/d912pxy/issues/25))
2. DirectX12 capable GPU with at least feature level 11_0 support
3. 1Gb VRAM
4. 6Gb system RAM

**Proper work on minimal hardware specifications is NOT GUARANTIED!**

**Recommended**:
1. Windows 10
2. DirectX12 capable GPU with feature level 11_1 support
3. 4Gb VRAM
4. 16Gb system RAM


Performance results are depend on your hardware.
*Do not expect huge performance benefit if your system is in overload on plain DX9*


# Installing

1. Disable all overlay software.
2. Download latest release [from here](https://github.com/megai2/d912pxy/releases).
3. Unpack it into the game folder, so d912pxy folder is in game root folder. (Next to Gw2-64.exe)
4. Run d912pxy/install.exe
5. Use default install by pressing "Enter" till programm exits or choose custom install according to programm output.

# Uninstalling

1. Run d912pxy/install.exe
2. Choose remove by entering 2
3. Remove d912pxy folder

# Known bugs

Bug: Fullscreen focus lost can hung the system videofeed
Troubleshooting: If you encontered this, use Alt+Ctl+Delete -> Logout to soft reboot. Use windowed fullscreen as workaround.

Bug: Squary image with artifacts on AMD cards
Troubleshooting: Add valuable effort to this [issue](https://github.com/megai2/d912pxy/issues/52) and wait for fixes

Bug: ArcDPS not working using chain load
Workaround: Read [this](https://github.com/megai2/d912pxy/issues/38#issuecomment-459956222) 


# Troubleshooting

## Case 1
  Things are loading part-by-part.
    
**Solution**

  d912pxy loads shaders asynchrounosly, because there is no efficient way to load them instantly.
  
  This can create some visual errors, but results in **much** better performance.
  
  
## Case 2
  Game crashes/hangs
  
**Solution**

  Do not ask game support about these crashes!
  
  If your game crashes without d912pxy don't ask about this here because d912pxy does zero modification to the game data.
  
  Make sure that the game runs without d912pxy!
  
  Next, update your driver and directx9.
  
  (link to dx9 https://www.microsoft.com/en-us/download/details.aspx?id=34429)
  
  Then, if you are still here, post your issue [here](https://github.com/megai2/d912pxy/issues/13) with 
  
    1. Log file form P7logs
    2. Crash.dmp and d912pxy_crash.txt if you have it    
    3. Version number of d912pxy you used
	4. Way to reproduce crash
    
    
 If you are asked to run debug version do this
 
   **0. Debug version writes ton of data, do not run it for a long time!**
   1. Install Release_d by installer (Use standart release? N => 3)
   2. Run the game and reproduce the error
   3. Send the log file or/and Crash.dmp to github

## Case 3 

  Visual errors
 
**Solution**

  0. Update or install stable GPU drivers first! 
  1. Install Release_ps by installer (Use standart release? N => 1)
  2. Run the game and reproduce the visual error
  3. Exit the game and clear shader cache by installer (Choose an action => 3, N, N)
  4. Install the regular version
  5. Run the game again and wait for the shaders to recompile (this will take 3-15 minutes depending on situation / hardware)
  6. If the error is not fixed post your issue [here](https://github.com/megai2/d912pxy/issues/15) with a description on how to reproduce the visual error

# Support developer

WMZ 442298672293
