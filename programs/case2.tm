#Q = {start, lt, rt, li, ri, acc, acc1, acc2, acc3, acc4, rej, rej1, rej2, rej3, rej4, rej5, halt}
#S = {1}
#G = {1, _, t, r, u, e, f, a, l, s}
#B = _
#q0 = start
#F = {halt}
#N = 2

; notice that 1 + 3 + 5 + ... + (2n-1) = n^2
start __ __ l* acc
start 1_ 11 ** rt

; t = 1, len(tape1) = 1
; tape0 can only go right
; tape0 stops, len(tape1) += 2
; traverse tape1 reversely
rt 11 11 rr rt
rt __ __ l* acc
rt _* __ l* rej ; tape0 ended, reject
rt 1_ 11 *r ri  ; len1 += 1
ri 1_ 11 ** lt  ; len1 += 1, change direction

lt 11 11 rl lt
lt __ __ l* acc
lt _* __ l* rej ; tape0 ended, reject
lt 1_ 11 *l li  ; len1 += 1
li 1_ 11 ** rt  ; len1 += 1, change direction

; clean up tape 0
acc *_ __ l* acc
acc __ __ r* acc1
rej *_ __ l* rej
rej __ __ r* rej1

; write true
acc1 *_ t_ r* acc2
acc1 __ t_ r* acc2
acc2 *_ r_ r* acc3
acc2 __ r_ r* acc3
acc3 *_ u_ r* acc4
acc3 __ u_ r* acc4
acc4 *_ e_ ** halt
acc4 __ e_ ** halt

; write false
rej1 *_ f_ r* rej2
rej1 __ f_ r* rej2
rej2 *_ a_ r* rej3
rej2 __ a_ r* rej3
rej3 *_ l_ r* rej4
rej3 __ l_ r* rej4
rej4 *_ s_ r* rej5
rej4 __ s_ r* rej5
rej5 *_ e_ ** halt
rej5 __ e_ ** halt
