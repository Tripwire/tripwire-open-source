echo off
..\..\mkslexyacc\mksnt\lex -a -o yylex.cpp -D yylex.h -P ..\..\mkslexyacc\etc\yylex.cpp -LC tokens.l  

if errorlevel 1 goto failed

echo Attaching header  
copy lexyacc_header.h +yylex.cpp yylex.tmp  
echo Moving file  
copy yylex.tmp ..\twparser\yylex.cpp  
copy lexyacc_header.h +yylex.h yylex.tmp  
echo Moving file  
copy yylex.tmp ..\twparser\yylex.h  
del yylex.tmp
del yylex.h
del yylex.cpp

goto end

:failed
echo Failed!

rem this is deliberate -- only way I know to set errorlevel!!!
false_this_file_does_not_exist

:end
