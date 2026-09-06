@echo off
pushd %~dp0

set PROTOC=..\ThirdParty\protobuf\tools\protoc.exe

%PROTOC% --proto_path=. --cpp_out=. YachtDice.proto

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] protoc failed
    popd
    exit /b 1
)

echo [OK] Generated YachtDice.pb.h / YachtDice.pb.cc
popd
