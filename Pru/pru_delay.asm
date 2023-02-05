    .global wait_cycles
wait_cycles:
delay:
    sub     r14,   r14, 1       ; The first argument is passed in r14
    qbne    delay, r14, 0

    jmp     r3.w2           ; r3 contains the return address