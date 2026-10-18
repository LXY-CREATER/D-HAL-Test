@echo off
echo 正在检查STM32CubeIDE环境...

REM 检查STM32CubeIDE是否安装
if exist "C:\ST\STM32CubeIDE_1.14.0\STM32CubeIDE\stm32cubeidec.exe" (
    set CUBEIDE_PATH=C:\ST\STM32CubeIDE_1.14.0\STM32CubeIDE\stm32cubeidec.exe
) else if exist "C:\ST\STM32CubeIDE\STM32CubeIDE\stm32cubeidec.exe" (
    set CUBEIDE_PATH=C:\ST\STM32CubeIDE\STM32CubeIDE\stm32cubeidec.exe
) else (
    echo 错误: 未找到STM32CubeIDE，请检查安装路径
    pause
    exit /b 1
)

echo 找到STM32CubeIDE: %CUBEIDE_PATH%
echo.

echo 开始编译项目...
cd /d "d:\c8t6c测试代码\c8t6_LED"

REM 使用STM32CubeIDE命令行编译
"%CUBEIDE_PATH%" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data "%CD%" -import "%CD%" -build c8t6_LED/Debug

if %ERRORLEVEL% EQU 0 (
    echo.
    echo 编译成功！
    echo 生成的文件位于: %CD%\Debug
) else (
    echo.
    echo 编译失败！错误代码: %ERRORLEVEL%
)

pause