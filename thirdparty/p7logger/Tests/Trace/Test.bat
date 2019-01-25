set count=500
:lableCycle

call Trace_64d.exe /P7.Addr=::1 /P7.Verb=3 /P7.Pool=2048 /Test=2 /Count=3 /Duration=5 /Speed=2
call Trace_32d.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=2 /Count=2 /Duration=5 /Speed=3

call Trace_64d.exe /P7.Addr=::1 /P7.Verb=3 /P7.Pool=2048 /Test=1 /Count=3000
call Trace_32d.exe /P7.Addr=127.0.0.1 /P7.Verb=3 /P7.Pool=2048 /Test=1 /Count=3000

set /a count = count - 1

if %count%==0 goto LabelEnd
goto lableCycle

:LabelEnd