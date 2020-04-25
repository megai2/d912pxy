$current_sid = [System.Security.Principal.WindowsIdentity]::GetCurrent().User | Select-Object -ExpandProperty Value

Remove-ItemProperty -Path "Registry::HKEY_USERS\$current_sid\Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" -Name *Gw2-64.exe

pause