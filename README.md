# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

d912pxy is a way to make games that use DirectX 9 use DirectX 12 instead, without changing any game code.

This is specifically designed for Guild Wars 2.
You can try to use it with other games; it might work.

This project is in beta. Expect some bugs, crashes, hangs, stalls and some other fun stuff.

[![Build status](https://ci.appveyor.com/api/projects/status/gs8drlb0goyp6h28?svg=true)](https://ci.appveyor.com/project/megai2/d912pxy)
[![](https://img.shields.io/discord/384735285197537290.svg?logo=discord&logoColor=f0f0f0)](https://discord.gg/y2MGJYr)

**WARNING: USE OF THIS SOFTWARE IS ENTIRELY AT YOUR OWN RISK!**

# Showcase

Profiling shows that the API overhead for d912pxy is up to 86% (r255,custom config) less than that of plain DirectX9.
Real performance differs based on the scene and the hardware.

Benchmarking results on i7-7700/GTX960:

## CPU-bound scenario
(running Mistlock Sanctuary)

```
@6.3k batches, all max, 1080p, v0.9.5.8a

dx9
  min FPS: 5.5
  max FPS: 21.5
dx12
  min FPS: 17.6
  max FPS: 26.6
boost:
  min FPS: +220%
  max FPS: +23%

```

## GPU-bound scenario
(1h fractal runs)

```
@all max, 1080p, v0.9.6.2a

16.4% to 2.5% less frame time

calculated based on API profiling & metrics data

This is affected by performance recording, and should be better in normal situations
```

DX12:

https://cdn.discordapp.com/attachments/477036595019644928/539417113593380865/unknown.png

DX9:

https://cdn.discordapp.com/attachments/477036595019644928/539417612501647360/unknown.png

Video by reddit u/moriz0, running WvW in 4k: https://www.youtube.com/watch?v=RQAB7Ma20Ow&

# Installing

[**Install instructions** (EN)](https://github.com/megai2/d912pxy/wiki/Installing)

[**Руководство по установке** (RU)](https://github.com/megai2/d912pxy/wiki/InstallingRU)

[**Tutoriel d'installation** (FR)](https://www.youtube.com/watch?v=hfSSIBICG6E)

# Known bugs

**Bug**: Fullscreen crash can hang system videofeed.

*Troubleshooting*: If you encounter this, use Ctrl+Alt+Delete -> Logout for a soft reboot. Use windowed fullscreen as workaround and report this as crash.

**Bug**: Crashes with 3-rd party tool/overlay/addon 

*Troubleshooting*: Check is your tool/overlay/addon compatible with d912pxy [here](https://github.com/megai2/d912pxy/issues/38). 
If you see no comments for your case, [report crash](https://github.com/megai2/d912pxy/wiki/Reporting-crashes)


# Troubleshooting

Case #1: [Things are loading piece-by-piece](https://github.com/megai2/d912pxy/wiki/HLSL-recompilation-and-loading#shader-loading)

Case #2: [Game crashes/hangs](https://github.com/megai2/d912pxy/wiki/Reporting-crashes)

Case #3: [Visual errors](https://github.com/megai2/d912pxy/wiki/Reporting-visual-errors)

Case #4: [Unnatural performance drops](https://github.com/megai2/d912pxy/wiki/Reporting-performance-issues)

# Support the developer

WMZ: 442298672293

Bitcoin: 1B46eQtKUcvSLyz1F8QxKcdHACAXbuXGgM

Patreon: https://www.patreon.com/d912pxy

Paypal: megai2@ya.ru
