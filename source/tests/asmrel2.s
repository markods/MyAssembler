
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

           mov     $zero,     %r0
                              
           mov     +2(%r0),   %r1
           add     +4(%r0),   %r1
           add      6(%r0),   %r1
           add      8(%r0),   %r1
           add     10(%r0),   %r1
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



