.global start
.equ in,    out+2             ; ff02
.equ out,   65280             ; ff00
.equ delta, 65296             ; ff10
.equ invalid_address, 52428   ; cccc
.equ limit, 6

.section    data,     ivt                   ; interrupt vector table
int0:       .word     0                     ; int0 - reset CPU
int1:       .word     error                 ; int1 - instruction error
int2:       .word     tick                  ; int2 - timer tick
int3:       .word     tin                   ; int3 - terminal
int4:       .word     0                     ; placeholder for the address of the handler of interrupt 4 
int5:       .word     0                     ; the address of the handler that will respond to interrupt 5
int6:       .word     0                     ; placeholder for the address of the handler of interrupt 6 
int7:       .word     0                     ; placeholder for the address of the handler of interrupt 7 

cnt:        .byte     6
msg:        .byte     101 ; e
            .byte     114 ; r
            .byte     114 ; r
            .byte     111 ; o
            .byte     114 ; r
            .byte     10  ; newline (line feed)
            .byte     0

            .word     0   ; padding
            .word     0
            .word     0


            
        
.section    text,     textsec

start:      mov    $8721,    %r0    ; 2211
            mov    %r0,      4000
loop:       jmp    loop             ; jmp    *loop(%pc)
            halt


error:      xor    %r0,      %r0
            movb   cnt,      %r1l
loop1:      movb   msg(%r0), %r2l
            mov    %r2,      out
            add    $1,       %r0
            cmp    %r0,      %r1
            jne    loop1
            halt


tick:       and    $0,       %r0
            mov    delta,    %r0
            add    $1,       %r0
            mov    %r0,      delta
            cmp    %r0,      $limit
            jeq    invalid_address
            add    $96,      %r0
            mov    %r0,      out
            mov    $10,      %r0
            mov    %r0,      out
            iret


tin:        mov    $0,       %r0
            or     $0,       %r0
            movb   in,       %r0l
            movb   %r0l,     out
            iret


.end


