rem generate sqlite amalgamation
cd %WORKSPACE%
cd ohos-data-insight-core/third_party/sqlite_src
mkdir bld
cd bld
nmake /f ..\Makefile.msc sqlite3.c TOP=..

rem copy sqlite3 and json
cd %WORKSPACE%/ohos-data-insight-core/third_party/sqlite
mkdir include
mkdir src
cd %WORKSPACE%/ohos-data-insight-core/third_party/sqlite_src/bld
xcopy sqlite3.h ..\\..\\sqlite\\include /Y /I /Q
xcopy sqlite3ext.h ..\\..\\sqlite\\include /Y /I /Q
xcopy sqlite3.c ..\\..\\sqlite\\src /Y /I /Q
xcopy shell.c ..\\..\\sqlite\\src /Y /I /Q
cd %WORKSPACE%/ohos-data-insight-core/third_party/
xcopy json\\single_include\\nlohmann json_modern_c++\\include /Y /I /E /Q