ADDI $t0, $zero, 10
ADDI $t1, $zero, 20
SLT $s0, $t0, $t1
SLT $s1, $t1, $t0
ADDI $t2, $zero, 1
SLL $t3, $t2, 4
IMPRIMIR $s0
IMPRIMIR $s1
IMPRIMIR $t3
SAIR
