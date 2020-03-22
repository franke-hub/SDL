#!/bin/bash
set -x
rm foo bar
./Encrypt SIZE0.DAT foo SIZE8.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE8.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE0.DAT bar

rm foo bar
./Encrypt SIZE1.DAT foo SIZE7.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE7.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE1.DAT bar

rm foo bar
./Encrypt SIZE2.DAT foo SIZE6.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE6.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE2.DAT bar

rm foo bar
./Encrypt SIZE3.DAT foo SIZE5.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE5.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE3.DAT bar

rm foo bar
./Encrypt SIZE4.DAT foo SIZE4.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE4.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE4.DAT bar

rm foo bar
./Encrypt SIZE5.DAT foo SIZE3.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE3.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE5.DAT bar

rm foo bar
./Encrypt SIZE6.DAT foo SIZE2.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE2.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE6.DAT bar

rm foo bar
./Encrypt SIZE7.DAT foo SIZE1.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE1.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE7.DAT bar

rm foo bar
./Encrypt SIZE8.DAT foo SIZE0.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
./Decrypt foo       bar SIZE0.DAT $1 $2 $3 $4 $5 $6 $7 $8 $9
diff    SIZE8.DAT bar

rm foo bar
