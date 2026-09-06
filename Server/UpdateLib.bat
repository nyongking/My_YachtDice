@echo off
pushd %~dp0

:: CoreLib headers
xcopy /Y /D /S ..\CoreLib\*.h Inc\ >nul 2>&1

:: ServerCoreLib headers
xcopy /Y /D /S ..\ServerCoreLib\*.h Inc\ >nul 2>&1

:: CoreLib lib
xcopy /Y /D ..\CoreLib\Bin\Debug\CoreLib.lib Lib\Debug\ >nul 2>&1
xcopy /Y /D ..\CoreLib\Bin\Release\CoreLib.lib Lib\Release\ >nul 2>&1

:: ServerCoreLib lib
xcopy /Y /D ..\ServerCoreLib\Bin\Debug\ServerCoreLib.lib Lib\Debug\ >nul 2>&1
xcopy /Y /D ..\ServerCoreLib\Bin\Release\ServerCoreLib.lib Lib\Release\ >nul 2>&1

:: PhysicsLib headers
xcopy /Y /D /S ..\PhysicsLib\*.h Inc\ >nul 2>&1

:: PhysicsLib lib
xcopy /Y /D ..\PhysicsLib\Bin\Debug\PhysicsLib.lib Lib\Debug\ >nul 2>&1
xcopy /Y /D ..\PhysicsLib\Bin\Release\PhysicsLib.lib Lib\Release\ >nul 2>&1

:: Protobuf DLLs to output
xcopy /Y /D ..\ThirdParty\protobuf\bin\Debug\*.dll Bin\Debug\ >nul 2>&1
xcopy /Y /D ..\ThirdParty\protobuf\bin\*.dll Bin\Release\ >nul 2>&1

popd
