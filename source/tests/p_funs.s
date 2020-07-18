; some functions
.global sumarray, mularray
.global multiply, divide, increase, decrease

.equ     one,    10-9
.equ     two,    one+one
.equ     three,  two+one
.equ     p2_ofs, p3-p0-2

.section data,   fdata

M:       .word   10             ; multiplication factor
D:       .word   2              ; divide factor

array:   .word   1,2,3,4,5      ; array of elements
cnt:     .word   5              ; number of elements in array

p0:      .word   0, one, two    ; array of multiplication factors
p3:      .word   three          ; one more multiplication factor :)

.section text,   functions

sumarray:  mov     $array, %r1   ; begin of array of elements
           mov     $0, %r2       ; total sum
           mov     cnt, %r3      ; loop counter
loop:      add     (%r1), %r2    ; add element to the sum
           int     5             ; call handler of interrupt 5
           add     $2, %r1       ; move pointer to the next element
           sub     $1, %r3       ; decrease loop counter
           cmp     %r3, $0       ; check if loop counter reached 0
           jne     loop          ; repeat the loop if 0 is not yet reached
           mov     %r2, res      ; store the result in bss section on position "res"
           ret

mularray:  mov     $p0,           %r0 ; index register
           mul     +2(%r0),       %r2
           mul     p2_ofs(%r0),   %r2
           mul     p3,            %r2
           mov     %r2, res      ; store the result in bss section on position "res"
           ret

multiply:  mul     M, %r2        ; M times multiply value of register r2
           mov     %r2, res      ; store the result in bss section on position "res"
           ret

divide:    div     D, %r2
           mov     %r2, res      ; store the result in bss section on position "res"
           ret     

increase:  add     $1, %r2
           mov     %r2, res      ; store the result in bss section on position "res"
           ret

decrease:  sub     $1, %r2
           mov     %r2, res      ; store the result in bss section on position "res"
           ret

.extern res
.end
