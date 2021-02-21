1. config for 8Gb VRAM/16Gb RAM and up
increases limit on per iframe draws, precompile limit,
pooling and forces residency of objects
this is the config you want to use if you don't care about how much memory
proxy will eat and the memory itself is free and ready to be used up

will almost remove any resource managment lags and so on
(still you can crank settings higher, but this ones are considered "stable")

2. config for 4gb VRAM/16Gb RAM
increases limit on per iframe draws, precompile limit and core pooling values
this is the config you want to use when you have spare RAM/VRAM
but still face OOM situations/lags with config nr.1
or simply you want to keep your chrome browser happy =]

expect some modest resource managment lag spikes if game calls for them

3. low memory config
this is the config you want to use if you don't have spare memory,
always hitting OOM/lags or using 32bit build

amount of pooling, draws per iframe, cleanup lazyness is reduced, residency enabled
don't expect smoothing out resource managment lags.
But memory overhead will be much lower than for default config and config 1 and 2