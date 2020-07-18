; main program
; final result in "res" should be 457 = ((1+2+3+4+5)*1*2*3+1-1)*10/2 + 7

.section bss,    bsssec

res:    .skip   2

.section text,   textsec  

start:   mov     $0, int5_cnt  ; initial value for interrupt 5 counter
         call    sumarray      ; sum array of 5 elements
         call    mularray      ; mul array with 3 elements
         int     5             ; call handler of interrupt 5
         call    increase      ; call function to increase value of register r2
         call    decrease      ; call function to decrease value of register r2
         call    multiply      ; call function to multiply value of register r2
         call    divide        ; call function to divide value of register r2
         int     5             ; call handler of interrupt 5
         add     int5_cnt, %r2 ; add the content of interrupt 5 counter to the result
         mov     %r2, res      ; store the result in bss section on position "res"
         halt

.global start
.extern sumarray, mularray
.extern multiply, divide, increase, decrease
.extern int5_cnt
.end


