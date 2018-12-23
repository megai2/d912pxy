| [EN](README.md#d912pxy) | [RU](README.md#d912pxy-1) |

# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

-What this thing does? 

-Makes games that use DirectX9, use DirectX12 instead, without any change to game code. 

 Especially Guild Wars 2. You can try use it in other games, maybe it will work.
 
 
 This project is not yet finished, expect bugs, crashes, hungs, stalls and all other fun(or not so fun) stuff!
 
 Current state: v0.9.2 alpha
 
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
2. Disable all overlay software
3. Download latest release
4. Unpack it into game folder
5. Run d912pxy/install.bat
6. Run the game

# How to remove

1. Run d912pxy/remove.bat
2. Delete d912pxy folder
3. Done

# Known bugs

-Screenshot functionality not working

-SMAA shader working wrong

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
  8. If error fixed, if you want(and more over if you can!) - send the newest created files in d912pxy/shaders/bugs to this github. goto 8.
  9. If error is not fixed, post your issue on github with a description on how to reproduse visual error  
  
| [EN](README.md#d912pxy) | [RU](README.md#d912pxy-1) |

**Support developer**

WMZ 442298672293

# d912pxy - "DirectX9 to DirectX12 API proxy for Guild Wars 2"

-Что делает эта штука?

-Позволяет играм, которые используют DirectX9, использовать DirectX12 без изменений в игровом коде. 

 К таким играм относится и Guild Wars 2, для которой и написана данная библиотека. Можете попробовать использовать эту библиотеку в других играх, возможно она будет работать.
 
 
 Этот проект ещё не завершен и находится в стадии альфа тестирования, возможны зависания, ошибки и вылеты!
 
 Текущая версия: v0.9.2 альфа
 
# Результаты

Тестирование показывает, что дополнительные расходы при работе с d912pxy на 40-20% меньше, чем при работе с обычным DirectX9.
Реальная производительность зависит от сцены!

DX12:

https://cdn.discordapp.com/attachments/477036595019644928/524540609105756160/unknown.png 


DX9:

https://cdn.discordapp.com/attachments/477036595019644928/524541036626837504/unknown.png

   
# Требования

Видеокарта с поддержкой DirectX12, конкретно 12.1 feature level и 3+ Gb VRAM.

16 Gb системной памяти

(будет изменятся по мере оптимизации)
 
# Как использовать

1. Установите поле "Resolution" в настройках графики в "Fullscreen windowed"/"Windowed"
2. Выключите все оверлеи/аддоны
3. Скачайте последний доступный релиз
4. Распакуйте архив в папку с игрой
5. Запустите d912pxy/install.bat
6. Запустите игру

# Как удалить

1. Запустите d912pxy/remove.bat
2. Удалите папку d912pxy
3. Готово

# Извесные проблемы

-Функция скриншотов не работает

-SMAA сглаживание работает не так, как положено

# Решение проблем

## Случай 1
  Мир долго прогружается, прогружается по частям.
  
  Какие-либо детали отсутствуют на экране.
  
**Решение**

  Я не предоставляю шейдеры вместе с релизом.
  
  Это значит, что шейдеры будут перекомпилированы при первом использовании и далее станут загружаться намного быстрее.
  
  Однако, шейдеры загружаются в реальном времени и по требованию в асинхронной манере, что создает эффект загружающихся/отсутствующих частей.
    
  Обычно это не критично.
  
  Пока что эта проблема не решена, т.к. требует более детального исследования данного вопроса.
  
  
## Случай 2
  Игра вылетает, зависает или выдает ошибку связанную с d912pxy
  
**Решение**

  Не обращайтесь в техподдержку игры, если установили d912pxy!
  
  Если игра падает без d912pxy, не спрашивайте об этом здесь, т.к. d912pxy не делает модификаций в игровых файлах.
    
  Во-первых, удостоверьтесь, что установили "Windowed fullscreen" или "Windowed" в настройках игры.
  
  Во-вторых, удостоверьтесь, что игра работает без d912pxy.  
  
  В-третьих, обновите графические драйвера и обновите DirectX9!
    
  (ссылка на установку dx9 https://www.microsoft.com/ru-ru/download/details.aspx?id=34429)
  
  Если проблема не решена, напишите о ней на github вместе со следующей информацией:
  
    1. Лог файл из папки P7logs
    2. Crash.dmp если он у вас появился
    
 Если вас попросят запустить дебаг версию, следуйте данной инструкции:
 
   0. Дебаг версия записывает огромное количество данных, не запускайте её надолго!
   1. Запустить d912pxy/remove.bat
   2. Запустить d912pxy/install_debug.bat
   3. Запустить игру
   4. Отправить лог файл и Crash.dmp на github

## Случай 3 

  Ошибки в графике
 
**Решение**

  1. Запустите d912pxy/remove.bat
  2. Запустите d912pxy/install_ps.bat
  3. Запустите игру, повторите найденную ошибку в графике.
  4. Запустите d912pxy/clean_shaders.bat
  5. Запустите d912pxy/remove.bat
  6. Запустите d912pxy/install.bat
  7. Запустите игру заново. Подождите пока шейдеры перекомпилируются.
  8. Если проблема исправлена и вы хотите(и главное можете!) - отправьте новые файлы из папки d912pxy/shaders/bugs на github. goto 8.
  9. Если ошибка не исправлена, напишите о ней на github вместе с описанием того, как данную ошибку повторить.  
  
**Поддержка разработчика**

 WMR 232397187043
