; sum array of elements, and store the multiplied result in "res"

.section bss,    bsssec
res:     .skip   2

.section data,   datasec
array:   .word   1,2,3,4,5     ; array of elements
cnt:     .word   5             ; number of elements in array

.section text,   textsec  
start:   mov     $array, %r1   ; begin of array of elements
         mov     $0, %r2       ; total sum
         mov     cnt, %r3      ; loop counter

loop:    add     (%r1), %r2    ; add element to the sum
         add     $2, %r1       ; move pointer to the next element
         sub     $1, %r3       ; decrease loop counter
         cmp     %r3, $0       ; check if loop counter reached 0
         jne     loop          ; repeat the loop if 0 is not yet reached
   
         call    multiply      ; call function to multiply value of register r2

         mov     %r2, res      ; store the result in bss section on position "res"
         halt

.global start
.extern multiply
.end


