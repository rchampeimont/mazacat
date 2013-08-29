@echo off

del mazacat-win.zip

rd /S /Q mazacat-win

md mazacat-win
copy Release\*.dll mazacat-win
copy Release\*.exe mazacat-win
copy src\*.png mazacat-win
md mazacat-win\niveaux
xcopy src\niveaux\*.* mazacat-win\niveaux /E
7-Zip\7z a mazacat-win.zip mazacat-win

rd /S /Q mazacat-win

pause