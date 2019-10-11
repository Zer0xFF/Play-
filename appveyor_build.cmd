@echo off
setlocal enabledelayedexpansion

cd deps
cd Dependencies
git remote add main https://github.com/Zer0xFF/Play-Dependencies.git
git fetch main
git pull main vs2019
cd ..
cd ..


mkdir build
cd build

if "%BUILD_PLAY%" == "ON" (
	cmake .. -G"%BUILD_TYPE%" -A x64 -DUSE_QT=on -DCMAKE_PREFIX_PATH="C:\Qt\5.12\%QT_FLAVOR%"
	if !errorlevel! neq 0 exit /b !errorlevel!
	
	cmake --build . --config %CONFIG_TYPE%
	if !errorlevel! neq 0 exit /b !errorlevel!

	c:\Qt\5.12\%QT_FLAVOR%\bin\windeployqt.exe ./Source/ui_qt/Release --no-system-d3d-compiler --no-quick-import --no-opengl-sw --no-compiler-runtime --no-translations	
	
	cd ..
	"C:\Program Files (x86)\NSIS\makensis.exe" ./installer_win32/%INSTALLER_SCRIPT%
	
	mkdir %REPO_COMMIT_SHORT%
	move installer_win32\*.exe %REPO_COMMIT_SHORT%
)

if "%BUILD_PSFPLAYER%" == "ON" (
	cmake .. -G"%BUILD_TYPE%" -A x64 -DBUILD_PLAY=off -DBUILD_TESTS=off -DBUILD_PSFPLAYER=on
	if !errorlevel! neq 0 exit /b !errorlevel!
	
	cmake --build . --config %CONFIG_TYPE%
	if !errorlevel! neq 0 exit /b !errorlevel!
	
	cd ..
	"C:\Program Files (x86)\NSIS\makensis.exe" ./tools/PsfPlayer/installer_win32/%INSTALLER_SCRIPT%

	mkdir %REPO_COMMIT_SHORT%
	move tools\PsfPlayer\installer_win32\*.exe %REPO_COMMIT_SHORT%
)

if "%BUILD_PSFAOT%" == "ON" (
	cmake .. -G"%BUILD_TYPE%" -A x64 -DBUILD_PLAY=off -DBUILD_TESTS=off -DBUILD_PSFPLAYER=on -DBUILD_AOT_CACHE=on
	if !errorlevel! neq 0 exit /b !errorlevel!
	
	cmake --build . --config %CONFIG_TYPE%
	if !errorlevel! neq 0 exit /b !errorlevel!
)
