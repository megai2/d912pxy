# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

d912pxy is a way to make games that use DirectX 9, use DirectX 12 instead, without changing any game code.

This is specifically designed for Guild Wars 2.
You can try to use it in other games, it might work.
 
This project is in early development, expect bugs, crashes, hangs, stalls and some other fun stuff.
 
[![Build status](https://ci.appveyor.com/api/projects/status/gs8drlb0goyp6h28?svg=true)](https://ci.appveyor.com/project/megai2/d912pxy)
[![](https://img.shields.io/discord/384735285197537290.svg?logo=discord&logoColor=f0f0f0)](https://discord.gg/y2MGJYr)

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

# Installing

[**Install instructions**(English)](https://github.com/megai2/d912pxy/wiki/Installing) 

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
  
## Case 4

  Unnatural performance drops
  
**Solution**

  1. Check that you not hit VRAM or RAM limits. If you hit it, do not report anything about performance.
  2. Test performance only on clean d912pxy, without any chainload / overlay. I can't take care about other code.
  3. Check original DX9 first! If original setup have unnatural drop of performance in your suggested scanario, it may be dependent on game engine, not DX9 or DX12.
  4. **Be shure that there is nothing on your system eating your own precious computational power.** 
  
  4.1. Antiviruses - mostly working on a realtime, they eat ton of performance. Solution: turn them off when you testing performance  
  
  4.2. Windows 10 bloatware - updates,updates of updates, installer of updates, installer of modules that install modules installing updates, collectors, inner background tasks, faulty self restarting apps that generates reports and all other things.
  Solution: Wait for a "quiet" time on your system or optimize it either by hand or by some tools.
  
  4.3. Other programs in background - commonly there is something running, like web browser. Solution: turn this programms off when you testing performance.
  
  5. **Be shure that your hardware is healthy**
  
  5.1. Cooling systems are clean and effective.
  
  5.2. HDD/SSD are working without hitches.
  
  5.3. No other hardware related issues.
  
  6. Install Release_pp by installer (Use standart release? N => 2)
  7. Run game, try to repeat performance drop 2-3 times, better with a time markers from game start.
  8. Exit game properly, with no crash.
  9. Report [here](https://github.com/megai2/d912pxy/issues/67) with a logfile from P7logs and performance_graph.png from game root folder.

# Support developer

WMZ 442298672293
