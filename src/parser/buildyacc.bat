echo off
..\..\mkslexyacc\mksnt\yacc -LC -o ..\twparser\yyparse.cpp -P ..\..\mkslexyacc\etc\yyparse.cpp -D yyparse.h policy.y  

if errorlevel 1 goto failed


copy lexyacc_header.h + yyparse.h xxx.tmp
copy xxx.tmp ..\twparser\yyparse.h  
del xxx.tmp
del yyparse.h

goto end

:failed
echo Failed!

rem this is deliberate -- only way I know to set errorlevel!!!
false_this_file_does_not_exist

:end
