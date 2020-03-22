erase bar
decrypt foo        bar test01.dot %1 %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 goto failure
e     bar
goto exit

:failure
echo "Failed to decrypt"

:exit

