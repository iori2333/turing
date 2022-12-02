#Q = {start, halt, copy, copy_done, head0, head1}
#S = {0, 1}
#G = {0, 1, _}
#B = _
#q0 = start
#F = {halt}
#N = 2

; first copy input string from tape 1 to tape 2
start __ __ ** halt
start *_ *_ *r copy
copy 0_ 00 rr copy
copy 1_ 11 rr copy
copy __ __ ll copy_done

; if last symbol is 0, means we need to add 0 to the head of the string
; else we need to add 1 to the head of the string
; add diff to two tapes so that we can copy directly
copy_done *0 *_ *l head0
head0 *_ 0_ ** halt
head0 *0 00 ll head0
head0 *1 11 ll head0

copy_done *1 *_ *l head1
head1 *_ 1_ ** halt
head1 *0 00 ll head1
head1 *1 11 ll head1
