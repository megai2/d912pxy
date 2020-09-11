# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

d912pxy is a way to make games that use DirectX 9 use DirectX 12 instead, without changing any game code.

This is specifically designed for Guild Wars 2.
You can try to use it with other games; it might work.

Tool will show overlay when it installed correctly, use default hotkey Ctrl+Alt+N to toggle its mode (hide/show)

Expect some minor crashes / visual bugs.

[![Current Version](https://img.shields.io/github/release/megai2/d912pxy)](https://github.com/megai2/d912pxy/releases) 
[![Github Downloads](https://img.shields.io/github/downloads/megai2/d912pxy/total.svg)](https://github.com/megai2/d912pxy/releases) 
[![Build status](https://ci.appveyor.com/api/projects/status/gs8drlb0goyp6h28?svg=true)](https://ci.appveyor.com/project/megai2/d912pxy) 
[![Discord](https://img.shields.io/discord/384735285197537290.svg?logo=discord&logoColor=f0f0f0)](https://discord.gg/fY9KADf) 
[![Patreon](https://img.shields.io/badge/Patreon-Support_the_development-red?logo=patreon&logoColor=f0f0f0)](https://www.patreon.com/d912pxy) 

**WARNING: USE OF THIS SOFTWARE IS ENTIRELY AT YOUR OWN RISK!**

# Showcase

Profiling shows that the API overhead for d912pxy is up to 86% (r255,custom config) less than that of plain DirectX9.
Real performance differs based on the scene and the hardware.

Benchmarking results on i7-7700/GTX960:

## CPU-bound scenario
(running Mistlock Sanctuary
with [custom config](https://cdn.discordapp.com/attachments/545164738288418816/583364425847799818/config.ini))

```
@5.5k batches, all max, 1080p, v0.9.9.3a

dx9
  min FPS: ~6.2
  max FPS: ~24.1
dx12
  min FPS: ~20.9
  max FPS: ~31.5
boost:
  min FPS: ~237%
  max FPS: ~30%
  
gathered by performance data accumulation and interpolation

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

[**Base guide (EN)**](https://github.com/megai2/d912pxy/wiki/Installing)

[**Using with other addons and overlays**](https://github.com/megai2/d912pxy/wiki/Using-with-other-addons-and-overlays)

Translated guides:

[**RU**](https://github.com/megai2/d912pxy/wiki/InstallingRU)

[**FR**](https://www.youtube.com/watch?v=hfSSIBICG6E)

[**CN**](https://hackmd.io/vXa_ukUpQaOaLRBDdXdMLA)

Video guides:

[Guildwars 2 - DX12 Full Setup Guide (912pxy mod)](https://www.youtube.com/watch?v=AhtJA9B55tU)

[Guild Wars 2: How to increase your FPS in 2020](https://www.youtube.com/watch?v=vsJyQKAa8FA)

[How to chainload ArcDPS and d912pxy for Guild Wars 2](https://www.youtube.com/watch?v=0jwbDpzuv3k)

[BnS DirectX12 Mod Installation Guide - B&S Reddit IQ safe(Windows 10)](https://www.youtube.com/watch?v=QkEX4Mgxnv8)


# Known bugs

**Bug**: 

Fullscreen/Alt+Enter crashing/hanging my game.

*Troubleshooting*: 

Avoid using fullscreen mode and Alt+Enter mode switching due to DXGI limitations. 
While it works and even recovers from DXGI deadlock, this is not 100% safe.
To recover from this bug manually, do soft-reboot: use Ctrl+Alt+Delete -> Logout.

**Bug**:

Crashes with 3-rd party tool/overlay/addon 

*Troubleshooting*: 

Check is your tool/overlay/addon compatible with d912pxy [here](https://github.com/megai2/d912pxy/issues/38). 
If you see no comments for your case, [report crash](https://github.com/megai2/d912pxy/wiki/Reporting-crashes)

**Bug**:

Pop-ins/slow loading/missing objects

*Troubleshooting*

This is normal for clean/first install as tool generates shader cache from ground up.
After shader cache is generated, load times will be much faster.
If you want to eleminate this problem once and for all, use [PSO precompile](https://github.com/megai2/d912pxy/wiki/Using-PSO-precompile)
and/or ready-to-use shader [packs](https://github.com/megai2/d912pxy/wiki/Shader-packs)

**Bug**

Game/installer are not starting

*Troubleshooting*

Update [MSVC](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

**Bug**

Nothing happens when installed

*Troubleshooting*

Ensure that d3d9.dll is in game root folder or bin64 folder.
Then try using this https://github.com/megai2/d912pxy/wiki/Removing-compat-flags

# Troubleshooting

Case #1: [Game crashes/hangs/not starting](https://github.com/megai2/d912pxy/wiki/Reporting-crashes)

Case #2: [Visual errors](https://github.com/megai2/d912pxy/wiki/Reporting-visual-errors)

Case #3: [Unnatural performance drops](https://github.com/megai2/d912pxy/wiki/Reporting-performance-issues)

# Support the developer

WMZ: 442298672293

Bitcoin: 1B46eQtKUcvSLyz1F8QxKcdHACAXbuXGgM

Patreon: https://www.patreon.com/d912pxy

Paypal: megai2@ya.ru
