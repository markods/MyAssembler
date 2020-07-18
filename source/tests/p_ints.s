; interrupt table and interrupt handlers

.section    data,     ivt                   ; interrupt vector table
int0:       .word     0                     ; int0 - reset CPU
int1:       .word     0                     ; int1 - instruction error
int2:       .word     0                     ; int2 - timer tick
int3:       .word     0                     ; int3 - terminal
int4:       .word     0                     ; placeholder for the address of the handler of interrupt 4 
int5:       .word     handler5              ; the address of the handler that will respond to interrupt 5 (int5)
int6:       .word     0                     ; placeholder for the address of the handler of interrupt 6 
int7:       .word     0                     ; placeholder for the address of the handler of interrupt 7 
                      
.section    text,     inthandlers
handler5:   add       $1, int5_cnt          ; this handler will respond to interrupt 5 (int5) by increaseing int5_cnt counter
            iret

.section    bss,      intdata
int5_cnt:   .skip     2                     ; handler of interrupt 5 (int5) will increase this number every time

.global     int5_cnt
.end
