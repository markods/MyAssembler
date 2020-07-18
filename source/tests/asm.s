; PAZNJA: ovaj fajl treba da bude u ANSI formatu
; PAZNJA: asembler je case-sensitive
; PAZNJA: po defaultu su sve operacije "w"
; PAZNJA: u .equ naredbi svaki pojedinacni literal mora biti u opsegu od -65536 do 65535
; PAZNJA: nazivi sekcija mogu biti samo bss, data, text (bez tacke na pocetku i dvotacke na kraju)


;ofstream umesto cout
;zameniti redosled ulaznih argumenata asemblera
;dodati ime sekcije
;labela start (za početak rada programa)
;tekst sekcije imaju svoj prostor, data sekcije svoj, bss svoj - treba proveriti pri loadovanju da nisu preklopljeni, to se mora reći emulatoru




  ;##############################
  ; sum array of elements, and store the result in "res"
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
           mov     %r2, res      ; store the result in bss section on position "res"
           halt
  .global start
  .end




; .equ x, datasec+4
; .section data, datasec
; emp:   .skip 4
;        .word 5,10,15
; .section text, textsec
; lbl:   mov $4, x
; .end
; 
;  ;##############################
;  ; sum array of elements, and store the result in r3
;  .section bss,  bsssec
;       .skip   2
;  x:   .skip   2
;  .section data, datasec
;       .word   1,2,3,4,5  ; array of elements
;  cnt: .word   5          ; number of elements in array
;       .skip   3
;  ofs: .word  -2
;       .word  loop        ; ovde se upisuje +15 kao ofset u odnosu na pocetak "datasec", pa se lokalni simbol "loop" ne eksportuje
;       .word  ext         ; ovde se upisuje 0
;       .word  ext7        ; ovde se upisuje 7
;  .extern ext
;  .section text, textsec  
;  start: mov   $data, %r1  ; begin of array of elements
;         mov   $0, %r2     ; total sum
;         mov   cnt, %r3    ; loop counter
;  loop:  add   (%r1), %r2  ; add element to the sum
;         add   $2, %r1     ; move pointer to the next element
;         sub   $1, %r3     ; decrease loop counter
;         cmp   %r3, $0     ; check if loop counter reached 0
;         jne   loop        ; repeat the loop if 0 is not yet reached
;  cont:  mov   %r1, x      ; store the result in bss section, on position x
;      
;         mov   $x, %r4      
;         mov   %r1, ofs(%r4) ; store the result in bss section, on position x-2
;  .global cont, start
;  .equ   ext7, ext + 7
;  .end
;     
;   ;##############################
;   ; .section bss, bsssec
;   ; .section data, datasec
;
;  .section text, textsec
;  lbl:     jmp lbl
;  .end
;  
;   
;   ;.section text, text ; nije dozvoljeno da se sekcija zove isto kao njen tip
;   ;.section bss, data   ; nije dozvoljeno da se sekcija zove isto kao tip druge sekcije
;   ;text:    jmp lbl  ; text se ne moze definisati dva puta
;   ;codesec: jmp lbl  ; naziv sekcije se ne moze definisati dva puta
;   
;   
;   
;   .equ x, 2
;   .global x
;   
;   .section data, codesec
;   .section text, codesec
;   lbl:   jmp lbl
;   .end
;   
;   .section data, datasec
;   .word    undef
;   .section text, textsec
;   undef:   call printf
;   .extern  printf
;   .end
;   
;   ;##############################
;   .extern undef
;   .section text
;      call undef
;   .end
;   
;   ;##############################
;   .section text
;          call undef
;   .extern undef
;   .end
;   
;   ;##############################
;   .global d1
;   
;   .section bss
;   bb0:     .skip 5
;   bb5:     .skip 2
;   bb7:     .skip 0
;   
;   .equ     v1,    1
;   .section data
;   d0:     .word   -1
;   d1:     .word   v1
;   d2:     .word   v2
;   c0:     .word   bb0
;   c5:     .word   bb5
;   c7:     .word   bb7
;   x:      .word   printf
;   y:      .word   putchar
;   a1:     .word   lbl1
;   a2:     .word   lbl2
;   .equ     v2,    2
;   
;   .extern printf
;   .section text
;   lbl1:    call   printf
;   lbl2:    call   putchar
;   .extern putchar
;   
;   .global d2
;   
;   .end
;   
;   ;   ;##############################
;   ; .global e1  ; ovo treba da je greska
;   ; .global e2  ; ovo treba da je greska!!!!!!!!!!!!!!!
;     .extern e1
;     .equ     e2,  e1 + 5
;     .equ     minus1, -1
;     .section data
;     niz:    .word 1, 2, 3, 4, 5
;     .equ     niz3, niz + 2
;     .global  niz3
;     .section text
;              mov   $niz, %r5
;              mov   100,  %r5
;              mov   $niz3, %r5
;              mov   1, %r1
;              mov   -1(%r1), %r5       ; da li je ovo dozvoljeno
;              mov   minus1(%r1), %r5   ; da li je ovo dozvoljeno
;              mov   niz(%r1), %r5
;              mov   $e1, %r5
;              mov   $e2, %r5
;   ; .global  e1  ; ovo treba da je greska
;   ; .global  e2  ; ovo treba da je greska!!!!!!!!!!!!!!
;     .end
;   
;   ;##############################
;   .global a, c
;   .extern b
;   .section text
;      jeq a
;      jeq e
;      jeq b
;      jeq d
;   d: mov b, %r1
;      mov c, %r2
;      mov e, %r3
;   .section data
;      .skip 8
;      .equ  e, c - text + d
;      .word  c
;   a: .word  b
;   .section bss
;   c: .skip 8
;      .global c
;   .end
;
;   ;##############################
;   .end
;
;   ;##############################
;   .section text
;      call printf
;      call printf2
;   .equ printf2, printf+2
;   .extern printf
;   .equ printf3, printf2+1
;      call printf3
;   .end
;   
;   ;##############################
;   .section data
;   d1:     .byte 5, sest, 7
;   d2:     .word 4, pet, 6
;   .equ    pet, sest-1
;   .extern sest
;   .end 
;   
;   ;##############################
;   ; sum value of r1 register r3 times, and store the result in r3
;   .section text
;         mov   $10, %r1   ; element
;         mov   $0, %r2    ; total sum
;         mov   $5, %r3    ; loop counter
;   loop: add   %r1, %r2   ; add element to the sum
;         sub   $1, %r3    ; decrease loop counter
;         cmp   %r3, $0    ; check if loop counter reached 0
;         jne   loop       ; repeat the loop if 0 is not yet reached
;   .end
;   
;   ;##############################
;   .section text
;   mov   $10, %r1
;   mov   $20, %r2
;   add   %r1, %r2
;   .end
;   
;   ;##############################
;   .section data
;   d1:     .byte 5, sest, 7
;   d2:     .word 2, tri, 4
;   .equ    tri, sest-3
;   .equ    sest, 6
;   .end 
;   
;   ;##############################
;   .section data
;   d1:     .byte -1, 0, 1, 2, 3, 255, -128
;         ; .byte 256    ; preveliko za byte
;         ; .byte -129   ; premalo za byte
;         ; .byte        ; nije navedena nijedna vrednost
;   d2:     .skip 3
;         ; .skip -3     ; only positive allowed
;         ; .skip 100000 ; preveliki broj
;         ; .skip 65535  ; text section overflow
;         ; .skip        ; nije navedena vrednost
;   d3:     .word -1, 4, 5, 6, 65535, -32768
;         ; .word 65536  ; preveliko za word
;         ; .word -32769 ; premalo za word
;         ; .word        ; nije navedena vrednost
;   
;   .section bss
;         ; .byte 1,2,3 ; not allowed in bss section
;         ; .word 1,2,3 ; not allowed in bss section
;         ; .skip 100000
;           .skip 7
;         ; .skip 65535 ; bss section overflow
;   .end
;   
;   ;##############################


;.global  byte1  ,   program_start_location   ,    dummy
;.global  byte1  ,   start   ,    dummy,  7  ; literal ne moze biti global
;.global  byte1  ,   start   ,    dummy,  -5  ; literal ne moze biti global
.global  byte1  ,   start   ,    dummy

;.extern   printf,   getchar, 7
;.extern   printf,   getchar, -5
.extern   printf,   bzvz

; .extern   printf ; already defined as extern!!!
.equ printf2, printf + 2

; byteX:    .equ jedanaest, 11   ; ovo je greska jer labela ne moze da stoji kod direktive koja ne generise sadrzaj


; badlbl:    ; label outside data/bss/text section

; asemblerske direktive za generisanje sadrzaja (.byte, .word i .skip) se moraju naci unutar .section!!!!

; add   %r1, %r2   ; greska - instrukcija izvan text sekcije

;------------------------------------------------
.section data

.word   start, dummy

; .section data  ; greska - sekcija definisana dva puta
; .section bezveze

; add   %r1, %r2   ; greska - instrukcija izvan text sekcije

.global byte0

; data:   .byte 4,5   ; greska - kljucna rec - simbol koji se koristi za oznacavanje sekcije (data, text, bss)
byte0:   .byte   5  ,  6  ,  7  ,  8

; byte0:   .byte   5  ,     ,     ,  8  ; ovo treba da bude greska!
;byte0:   .byte   5     6     7     8  ; greska

byte1:
byte11:
; byte1:  ; ponovo definisan
.byte  pet, 6, 7,  8 ,   9,deset
    .byte  cetiri   ,    pet  ,   sest
byte2:
; .byte 256 ; preveliko za byte
.byte 255
.byte 127
.byte -128

align1: .skip 5

;----------------------

word0:   .word   1  ,  2  ,  3  ,  4
word1:
;     .word  10000, 5,   -22,       ; greska
   word2:
;     .word  65535   ; preveliko za word
   word3:
      .word  -32768
;   word2:             ; twice defined symbol
      .word  0, 32767

empty1: .skip  10
;  .skip  deset, 0   ; skip moze imati samo jedan operand

        .word 5

;----------------------

.equ  x3, pet-dva
.equ  x4, pet-1
.equ  dva, pet-3
.equ  pet, 5
.equ  deset  ,  pet  +  pet

;----------------------

.equ  broj11,1+deset
.equ  minus1, -1
.equ  minus2, -1-1
.equ  tri,    -  minus1 -  minus2
;  .equ  minus1, -   ; error
;  .equ  minus1, +   ; error
      .equ  cetiri, pet+minus1
      .equ  sest, pet - 1 + minus1+1
;      .equ  100, sto  ;   na prvom mestu se mora naci simbol
      .equ  sto, 50+50
      .equ  devet, +9
;     .equ  devet, 9  ; konflikt, vec je definisano (mozda ne treba da bude konfikt ako ima istu vrednost)

;----------------------

; simboli koji zavise od eksternih, pre nego sto su eksterni deklarisani
.equ E1, e1
.equ E2, e2
.equ E3, e1 + 10
; .equ E4, 20 - e2   ; nije dozvoljeno da je eksterni simbol negativan
; .equ E5, e1 + e2   ; nije dozvoljena zavisnost od dva eksterna simbola

.extern e1
.extern e2


;  .equ ex55, t1 - t1 + e2
.equ t1,  e1
.equ t2,  e2

;.equ  hh23,   a + bb + bb + bb - bb  -c +  d-  3 +e   ; nepoznati simboli
;.equ  hh24,   hh23 + f -g                             ; nepoznati simboli


;     .equ e1, 10  ; nije dozvoljeno!

.equ ex1, e1

; .equ ex2a, -e1  ; greska - eksterni uvek mora biti pozitivan
; .equ ex2b, 22-e1  ; greska - eksterni uvek mora biti pozitivan

.equ ex3, e1 + 22

; .equ ex4a, e1 - e2 + 33  ; greska - eksterni uvek mora imati pozitivan znak
; .equ ex4b, e1 + e2 + 33  ; ne mogu dva eksterna

.equ ex5, ex3 + 8 + word3 - word2 + word1 - word0 + 12 - 10

;.equ ex5, e1 + 8  ;greska - isti simbol ponovo definisan

; .equ ex6, 15 - e1 + 8 + word3 - word2 + word1 - word0 + 12 - 10  ; greska - ispred eksternog ne sme biti znak "-"

;----------------------

.equ in9, word2 - word1 + 5 + dummy - text     

;.equ in8, word1 + dummy                        ; neparan broj iz razlicitih sekcija (nisu ponisteni)!

; .equ in7, word1+word1+word1-word0-word0+word0  ; greska jer je neparan broj iz data sekcije

.equ in7, word1+word1+word1-word0-word0-word0

;----------------------

.equ ex10, ex1 + 10
.equ ex11, ex10 + 1
.equ ex15, ex11 + 4

;----------------------

; deadlock
;.equ loop1, loop2
;.equ loop2, loop1  

;.equ  program_start_location   ,  MAin
.equ  start   ,  MAin


 ; .section".empty section"     ; da li je dozvoljeno da literal pocne bez space-a?   da li je dozvoljeno da naziv sekcije sadrzi whitespace?
 ;  .section ".empty section"   ; naziv sekcije mora biti simbol, tj. ne sme biti pod navodnicima
             ; da li je dozvoljeno da postoji prazna sekcija?


   msg:
;         .asciz      "  Hi   There!\n  "  ; da li ovo treba?


;------------------------------------------------
.section bss
a: .skip 100
b: .skip 900

;  .byte 1,2,3  ; greska - podaci izvan data sekcije
;  add   %r1, %r2   ; greska - instrukcija izvan text sekcije

 ;       .seCTIOn  text  ; case-sensitive
 ;       .section  teXt  ; case-sensitive


;------------------------------------------------
.section   text

;        .global     main         ; main je deklarisan kao global, ali se nigde ne pojavljuje
         .global     MAin
;         push
MAin  :  push       %r2            ; da li je dozvoljen space pre ":"?
dummy:   mov        %r3,   %r1            
       ; CALL       GETCHAR
         call       GETCHAR
;        cmp        %r1      ,   'A'   ; da li je dozvoljeno unositi brojeve kao karaktere?
         cmp        %r1      ,   382   ; da li je dozvoljen space pre zareza?
;        push       offset  msg       ; sta je ovo "ofset"?
         push       %r2
         call       printf
         call       printf2

; ustanoviti da li sve instrukcije mogu da imaju "b" i "w" na kraju, ili samo one koje rade sa podacima
         jmpw       23456
         jmpb       123

; proba1:  proba2:    proba3:    ; nije dozvoljeno staviti nekoliko labela jednu za drugom (ili mozda treba ovo dozvoliti?)

       ; jne- 5         ; greska
         jne        skip
         jeq        *%r3
         jeq        *%r3h
         jeq        *%r3l
       ; jeq        *%r3x  ; "x" nije dozvoljeno, vec samo "h" i "l"
         jgt        *(%r2)
         jgt        *(%r2)
         jmp        *210(%r1)
x22:
x33:
x44:
         jmp        *dummy(%r5)
;        jmp        *dummy(%r5h)  ; <h|l> je dozvoljeno samo kod registarskog direktnog adresiranja
;        jmp        *dummy(%r5l)  ; <h|l> je dozvoljeno samo kod registarskog direktnog adresiranja
         jmp        *dummy(%pc)
x2b:
         jmp        *456
;        jmp        *-456         ; ovo je greska!
;        jmp        *99999        ; preveliki broj;

;.equ     toobig,    neg1 
;         jmp        *neg1

         jmp        *dummy
         jmp        x2b

; ---------------------

         jmp        tt1
        .equ  tt1, -pp1
        .equ  pp1, -10
     ;  .equ  pp1,  10  ;  jmp ne moze na tt1, jer je vrednost pp1 pozitivna, pa je tt1 negatinno

     ;  .equ  pp2,  10   ; jmp ne moze na tt2, jer je vrednost pp2 pozitivna, pa je tt2 negativno
        .equ  pp2, -10
        .equ  tt2, -pp2
         jmp        tt2

; ---------------------

         add        %r1,  %r2
;        add        %r1h, %r2      ; neuskladjena velicina operanada
;        add        %r1l, %r2      ; neuskladjena velicina operanada
;        add        %r1,  %r2h     ; neuskladjena velicina operanada
         add        %r1h, %r2h
         add        %r1l, %r2h
;        add        %r1,  %r2l     ; neuskladjena velicina operanada
         add        %r1h, %r2l
         add        %r1l, %r2l

         addw       %r1,  %r2
;        addw       %r1h, %r2     ; ne moze sa "w"
;        addw       %r1l, %r2     ; ne moze sa "w"
;        addw       %r1,  %r2h    ; ne moze sa "w"
;        addw       %r1h, %r2h    ; ne moze sa "w"
;        addw       %r1l, %r2h    ; ne moze sa "w"
;        addw       %r1,  %r2l    ; ne moze sa "w"
;        addw       %r1h, %r2l    ; ne moze sa "w"
;        addw       %r1l, %r2l    ; ne moze sa "w"

;        addb       %r1,  %r2     ; ne moze sa "b"
;        addb       %r1h, %r2     ; ne moze sa "b"
;        addb       %r1l, %r2     ; ne moze sa "b"
;        addb       %r1,  %r2h    ; ne moze sa "b"
         addb       %r1h, %r2h
         addb       %r1l, %r2h
;        addb       %r1,  %r2l    ; ne moze sa "b"
         addb       %r1h, %r2l
         addb       %r1l, %r2l

; --------------------------

;printf: addw       %r4,  10000

;        addb       %r4,  100    ; umesto %r4 mora da stoji %r4h ili %r4l
         addb       %r4h, 100

         addb       %r4h, 10000  ; prevelika vrednost za "b"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

;        add        %r4,  + 10000   ; znak nije dozvoljen kod memorijskog adresiranja
;        add        %r4,  - 10000   ; znak nije dozvoljen kod memorijskog adresiranja
;        addw       %r4,  +  10000  ; znak nije dozvoljen kod memorijskog adresiranja
;        addw       %r4, -  10000   ; znak nije dozvoljen kod memorijskog adresiranja
;        addb       %r4,   + 100    ; znak nije dozvoljen kod memorijskog adresiranja
;        addb       %r4,  - 100     ; znak nije dozvoljen kod memorijskog adresiranja

       ; addw       %r4,11+22             ; nije dozvoljeno
       ; addw       %r4,   33  -  44      ; nije dozvoljeno
       ; addw       %r4, +                ; nije dozvoljeno
       ; addw       %r4, -                ; nije dozvoljeno
       ; addw       %r4, 55+              ; nije dozvoljeno
       ; addw       %r4, 66  -            ; nije dozvoljeno

       ; addw       11+22         ,   %r4 ; nije dozvoljeno
       ; addw          33  -  44  ,   %r4 ; nije dozvoljeno
       ; addw        +            ,   %r4 ; nije dozvoljeno
       ; addw        -            ,   %r4 ; nije dozvoljeno
       ; addw        55+          ,   %r4 ; nije dozvoljeno
       ; addw        66  -        ,   %r4 ; nije dozvoljeno
 ;       addw        +  55        ,   %r4    ; znak nije dozvoljen kod memorijskog adresiranja
 ;       addw        -  66        ,   %r4    ; znak nije dozvoljen kod memorijskog adresiranja

       ; addw       %r4,-65537 ; preveliki negativan broj
       ; addw       %r4,-65536 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addw       %r4,-32769 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addw       %r4,-32768 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addw       %r4,-10000 ; greska - znak nije dozvoljen kod memorijskog adresiranja
         addw       %r4,     0
         addw       %r4, 10000
       ; addw       %r4,+10000 ; greska - znak nije dozvoljen kod memorijskog adresiranja
         addw       %r4, 32767
         addw       %r4, 32768
         addw       %r4, 65535
       ; addw       %r4, 65536  ; preveliki pozitivan broj

       ; addb       %r4h,  -256 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addb       %r4h,  -255 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addb       %r4h,  -129 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addb       %r4h,  -128 ; greska - znak nije dozvoljen kod memorijskog adresiranja
       ; addb       %r4h,   -50 ; greska - znak nije dozvoljen kod memorijskog adresiranja
         addb       %r4h,     0
       ; addb       %r4h,   +50 ; greska - znak nije dozvoljen kod memorijskog adresiranja
         addb       %r4h,    50
         addb       %r4h,   127
         addb       %r4h,   128
         addb       %r4h,   255
         addb       %r4h,   256

       ; addw       $-65537, %r4; preveliki negativan broj
       ; addw       $-65536, %r4; preveliki negativan broj
       ; addw       $-32769, %r4; preveliki negativan broj
         addw       $-32768, %r4
         addw       $-10000, %r4
         addw            $0, %r4
         addw        $10000, %r4
         addw       $+10000, %r4
         addw        $32767, %r4
       ; addw        $32768, %r4 ; preveliki pozitivan broj
       ; addw        $65535, %r4 ; preveliki pozitivan broj
       ; addw        $65536, %r4 ; preveliki pozitivan broj

       ; addb         $-256, %r4h; preveliki negativan broj
       ; addb         $-255, %r4h; preveliki negativan broj
       ; addb         $-129, %r4h; preveliki negativan broj
         addb         $-128, %r4h
         addb          $-50, %r4h
         addb            $0, %r4h
         addb          $+50, %r4h
         addb           $50, %r4h
         addb          $127, %r4h
       ; addb          $128, %r4h; preveliki pozitivan broj
       ; addb          $255, %r4h; preveliki pozitivan broj
       ; addb          $256, %r4h; preveliki pozitivan broj

; ----------------------

;        add        %r4,$4    ; immediate addressing not allowed in dst
         add        $4,%r4
       ; add        %r4,$(neispravno)
       ; add        %r4,$dummy
       ; xchg       %r4,$40   ; immediate addressing not allowed in dst
       ; xchg       $40,%r4   ; immediate addressing not allowed in dst
         add        $dummy,%r4
         add        %r4,%r5
         add        %r4,(%r5)
         add        %r4  ,  234(%r5)   
         add        %r4  , -234(%r5)   
         addb       %r4h ,  234(%r5)   
         addb       %r4h , -234(%r5)   
         add        %r4,  dummy(%r5)
         add        %r4  ,dummy(%pc)
         add        %r4,123
         add        %r4,dummy

; ----------------------

;        add               ; missing first operand
;        add        %r1     ; missing second operand
;        jmp               ; missing operand
 
;        shr        $55,%r2  ; instrukcija SHIFT RIGHT ima obrnuti redosled destination i source (shr dst, src)
         shr        %r2,$55

skip:
         mov      %r0  ,0
         mov      %r1,%r0

         add      $5, -5(%r7)

       ; add      $5, 32768(%r7)   ; too big
         add      $5, 32767(%r7)
         add      $5, -32768(%r7)
       ; add      $5, -32769(%r7)  ; too small

.equ     bad1,     32768
.equ     ok1,      32767
.equ     ok2,     -32768
.equ     bad2,    -32769

; ----------------------

       ; add      $5, bad1(%r7)   ; too big
         add      $5, ok1(%r7)
         add      $5, ok2(%r7)
       ; add      $5, bad2(%r7)  ; too small

       ; add      $5, bad1(%pc)   ; too big
         add      $5, ok1(%pc)
         add      $5, ok2(%pc)
       ; add      $5, bad2(%pc)  ; too small

; ----------------------

.equ     fwr3,   fwr1
         jmp     fwr1
fwr1:    jmp     fwr2
fwr2:    jmp     fwr3
         jmp     fwr4
.extern  fwr4

; ----------------------

         pop      %r0
         ret


.extern GETCHAR

.equ    skip4,  skip3
.equ    skip2,  skip
.equ    skip3,  skip
.equ    skip5,  skip4 - skip2 + 1


; .end  abc ; neispravna instrukcija za zavrsetak, ne treba da stane zbog .end vec treba da prijavi gresku!!!

.end   ; kraj kompajliranja (a samim tim i prethonde sekcije)

         add       %r1, 5 ; ova instrukcija se ne kompajlira, jer se nalazi posle .end

lkflf asdjfcl csladfkj aclfskj aclskjdf fadskjf alsdjkf ; djubre takodje
