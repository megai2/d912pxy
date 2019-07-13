set count=50
:lableCycle

call P7_Test64.exe /P7.Addr=::1 /P7.Verb=3 /P7.Pool=2048 /Test=2 /Count=3 /Duration=5 /Speed=2
rem call P7_Test32.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=2 /Count=2 /Duration=5 /Speed=3

call P7_Test64.exe /P7.Addr=::1 /P7.Verb=3 /P7.Pool=2048 /Test=1 /Count=300
rem call P7_Test32.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=1 /Count=300

set /a count = count - 1

if %count%==0 goto LabelEnd
goto lableCycle

:LabelEnd

call P7_Test64.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=3 /Count=5
call P7_Test64.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=5 
call P7_Test64.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=8 
