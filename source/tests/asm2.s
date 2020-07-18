; N times multiply value of register r2

.section data,   muldata
N:       .word   10             ; multiplication factor

.section text,   multext
multiply:
         mul     N, %r2        ; N times multiply value of register r2
         ret

.global multiply
.end
