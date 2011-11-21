@echo off
@setlocal
@if not exist "%~f1" goto usage
@if "%2" == "" goto usage
@del "%TEMP%\temp.nif" > nul 2<&1

call NifCmd FixTex -i "%~f1" -o "%TEMP%\temp.nif"
REM More elborate texture fixing.  Will search through base oblivion textures for name matches if necessary.
REM call NifCmd FixTex -i "%~f1" -o "%TEMP%\temp.nif" -r "e:\Nifs\Oblivion\Oblivion - Textures - Compressed" -p "e:\Nifs\Oblivion\Oblivion - Textures - Compressed\textures\armor"

REM Unparent any nodes if PrepareForNifExport was not called. (Doesnt currently work)
REM call NifCmd Unparent -i "%TEMP%\temp.nif" -o "%TEMP%\temp.nif" -e "Bip01*" "*Helper" BackWeapon SideWeapon MagicNode
call NifCmd CullChild -i "%TEMP%\temp.nif" -o "%TEMP%\temp.nif" -e "Bip01*" "*Helper" BackWeapon SideWeapon MagicNode
call NifCmd StripNodes -i "%TEMP%\temp.nif" -o "%TEMP%\temp.nif" 
call NifCmd SortNodes -i "%TEMP%\temp.nif" -o "%TEMP%\temp.nif"
copy "%TEMP%\temp.nif" "%~f2"

@goto exit
:usage
@echo Usage: FixCiv4Nif [Input] [Output]
:exit
@endlocal