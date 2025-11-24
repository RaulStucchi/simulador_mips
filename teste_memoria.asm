ADDI $t0, $zero, 55
ADDI $s0, $zero, 4
SW $t0, 0($zero)
SW $t0, 4($zero)
LW $t1, 0($zero)
ADD $t2, $t1, $t0
IMPRIMIR $t2
SAIR
