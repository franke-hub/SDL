@echo on
erase foo
erase bar
encrypt size0.dat foo size8.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size8.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size0.dat bar

erase foo
erase bar
encrypt size1.dat foo size7.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size7.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size1.dat bar

erase foo
erase bar
encrypt size2.dat foo size6.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size6.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size2.dat bar

erase foo
erase bar
encrypt size3.dat foo size5.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size5.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size3.dat bar

erase foo
erase bar
encrypt size4.dat foo size4.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size4.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size4.dat bar

erase foo
erase bar
encrypt size5.dat foo size3.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size3.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size5.dat bar

erase foo
erase bar
encrypt size6.dat foo size2.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size2.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size6.dat bar

erase foo
erase bar
encrypt size7.dat foo size1.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size1.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size7.dat bar

erase foo
erase bar
encrypt size8.dat foo size0.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
decrypt foo       bar size0.dat %1 %2 %3 %4 %5 %6 %7 %8 %9
fdiff   size8.dat bar

erase foo
erase bar
