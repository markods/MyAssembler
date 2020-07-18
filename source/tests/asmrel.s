
  ;##############################
  ; sum array of elements, and store the result in r2
  .section data,   datasec
  m5:      .word   -5
  m4:      .word   -4
  m3:      .word   -3
  m2:      .word   -2
  m1:      .word   -1
  zero:    .word   0
  p1:      .word   +1
  p2:      .word   +2
  p3:      .word   +3
  p4:      .word   +4
  p5:      .word   +5

  .section text,   textsec  
  start: 
  .global  start
  .extern  ext1

;          jne     *-10(%pc)   ; literal ne moze uz PC  
           jne     *-10(%r7)
;          jne     *+10(%pc)   ; literal ne moze uz PC
           jne     *+10(%r7)
;          jne     *10(%pc)    ; literal ne moze uz PC
           jne     *10(%r7)
;          jne     -10         ; apsolutni skok ne moze na negativnu adresu
           jne     +10 
           jne     10 
  loop1:   jne     *ext1(%pc) 
           jne     *loop1(%pc) 
           jne     *ext2(%pc) 
           jne     *loop2(%pc) 
           jne     loop1
           jne     ext1
           jne     loop2
           jne     ext2
  loop2:
  .extern  ext2

           mov     $zero,     %r0
                              
           mov     -4(%r0),   %r1
           mov      0(%r0),   %r1
           mov      4(%r0),   %r1
   
           mov       m3(%r0), %r1
           mov     zero(%r0), %r1
           mov       p3(%r0), %r1

           mov       m3(%pc), %r1
           mov     zero(%pc), %r1
           mov       p3(%pc), %r1

           mov    first(%r0), %r1
           mov    last(%r0),  %r1

           mov    first(%pc), %r1
           mov    last(%pc),  %r1

   .equ    first,   -10
   .equ    last,    +10

           mov    first(%r0), %r1
           mov    last(%r0),  %r1

           mov    first(%pc), %r1
           mov    last(%pc),  %r1

   pos1:
   .extern extdat1
           mov    extdat1(%r0), %r1
           mov    extdat2(%r0), %r1
           mov    extdat1(%r7), %r1
           mov    extdat2(%r7), %r1
           mov    extdat1(%pc), %r1
           mov    extdat2(%pc), %r1

           mov    pos1(%r0),    %r1
           mov    pos2(%r0),    %r1
           mov    pos1(%r7),    %r1
           mov    pos2(%r7),    %r1
           mov    pos1(%pc),    %r1
           mov    pos2(%pc),    %r1

   .extern extdat2
   pos2:
           halt
  .end



; ;##############################
; ; sum array of elements, and store the result in r2
; .section data,   datasec
; cnt:     .word   5             ; number of elements in array
; .section text,   textsec  
; start:   mov     $0, %r2       ; total sum
;          mov     cnt, %r3      ; loop counter
; loop:    add     %r3, %r2      ; add element to the sum
;          sub     $1, %r3       ; decrease loop counter
;          cmp     %r3, $0       ; check if loop counter reached 0
;          jne     loop          ; repeat the loop if 0 is not yet reached
;          halt
; .global start
; .end



