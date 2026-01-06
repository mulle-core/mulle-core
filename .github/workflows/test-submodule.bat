@echo off
setlocal enabledelayedexpansion

REM Get repo URL and reference from arguments
set "REPO_URL=%~1"
set "REPO_REF=%~2"
if "%REPO_URL%"=="" set "REPO_URL=https://github.com/mulle-core/mulle-core.git"

REM Create temp directory
for /f %%i in ('powershell -command "[System.IO.Path]::GetTempPath() + [System.Guid]::NewGuid().ToString()"') do set "TEMP_DIR=%%i"
mkdir "%TEMP_DIR%"
cd /d "%TEMP_DIR%"

echo Creating test project in %TEMP_DIR%
echo Using repository: %REPO_URL%
if not "%REPO_REF%"=="" echo Using reference: %REPO_REF%

REM Initialize git repo
git init
git config user.email "test@example.com"
git config user.name "Test User"
git config protocol.file.allow always

REM Add mulle-core as submodule
git submodule add "%REPO_URL%" mulle-core
if not "%REPO_REF%"=="" (
    cd mulle-core
    git checkout "%REPO_REF%"
    cd ..
)
git submodule update --init --recursive

REM Create CMakeLists.txt
(
echo cmake_minimum_required^(VERSION 3.15^)
echo project^(submodule-test^)
echo add_subdirectory^(mulle-core^)
echo add_executable^(test main.c^)
echo target_link_libraries^(test PRIVATE mulle-core^)
) > CMakeLists.txt

REM Create main.c
(
echo #include ^<mulle-core/mulle-core.h^>
echo.
echo int main^(^) {
echo     // Test mulle_fprintf
echo     mulle_fprintf^(stdout, "mulle-core submodule test: %%s\n", "SUCCESS"^);
echo.    
echo     // Test mulle_mmap page allocation
echo     void *page = mulle_allocator_malloc^(^&mulle_allocator_default, 4096^);
echo     if ^(!page^) {
echo         mulle_fprintf^(stderr, "mulle_mmap page allocation failed\n"^);
echo         return 1;
echo     }
echo     mulle_fprintf^(stdout, "mulle_mmap page allocation: SUCCESS\n"^);
echo     mulle_allocator_free^(^&mulle_allocator_default, page^);
echo.    
echo     // Test GetProcAddress functionality ^(Windows equivalent of dlsym^)
echo     HMODULE kernel32 = GetModuleHandle^(L"kernel32.dll"^);
echo     if ^(kernel32 ^&^& GetProcAddress^(kernel32, "GetCurrentProcess"^)^) {
echo         mulle_fprintf^(stdout, "GetProcAddress present: SUCCESS\n"^);
echo     } else {
echo         mulle_fprintf^(stderr, "GetProcAddress failed\n"^);
echo         return 1;
echo     }
echo.    
echo     return 0;
echo }
) > main.c

REM Build and test
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
build\Release\test.exe

echo Test completed successfully
cd /d %~dp0
rmdir /s /q "%TEMP_DIR%"
