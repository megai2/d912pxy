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

**Bug**: Fullscreen focus lost can hung the system videofeed

*Troubleshooting*: If you encontered this, use Alt+Ctl+Delete -> Logout to soft reboot. Use windowed fullscreen as workaround.

**Bug**: Squary image with artifacts on AMD cards

*Troubleshooting*: Add valuable effort to this [issue](https://github.com/megai2/d912pxy/issues/52) and wait for fixes

**Bug**: ArcDPS not working using chain load

*Workaround*: Read [this](https://github.com/megai2/d912pxy/issues/38#issuecomment-459956222) 


# Troubleshooting

Case #1: [Things are loading part-by-part](https://github.com/megai2/d912pxy/wiki/HLSL-recompilation-and-loading#shader-loading)     

Case #2: [Game crashes/hangs](https://github.com/megai2/d912pxy/wiki/Reporting-crashes)  

Case #3: [Visual errors](https://github.com/megai2/d912pxy/wiki/Reporting-visual-errors)

Case #4: [Unnatural performance drops](https://github.com/megai2/d912pxy/wiki/Reporting-performance-issues)

# Support developer

WMZ 442298672293
