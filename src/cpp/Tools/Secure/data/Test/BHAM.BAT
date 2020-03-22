erase foo
erase bar
encrypt test01.dit foo test01.dot %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo        bar test01.dot %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   test01.dit bar
