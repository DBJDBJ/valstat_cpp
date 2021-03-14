@rem
goto start

Turn off optimization (-O0) 
and includes debugging symbols (-g).

Release version where 
optimization is turned on (-O2), debugging symbols stripped (-s) 
and assertions disabled (-DNDEBUG).

:start
@rem
@SETLOCAL ENABLEEXTENSIONS
@SET "G++=D:\PROD\TDM-GCC-64\bin\g++.exe"
@SET "OPT=-fno-exceptions -Wno-unused-parameter -Wall -Wextra -Wpedantic -std=c++17"
@SET "RELEASE=-s -O3 -march=native -mtune=native -DNDEBUG=1"
@SET "DEBUG=-O0"
@rem
%G++% %OPT% %RELEASE% -o "non trivial but simple sample" "non trivial but simple sample.cpp"