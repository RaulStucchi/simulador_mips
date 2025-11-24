#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================
// 1. DEFINIÇÕES E ESTRUTURAS
// =============================================================

#define NUM_REGS 32
#define MEM_SIZE 1024

// Estrutura do processador
typedef struct {
    int regs[NUM_REGS];        // Banco de Registradores ($0 a $31)
    int pc;                    // Program Counter
    int data_memory[MEM_SIZE]; // Memória de dados (RAM simulada em Words)
    int running;               // Flag de execução
} MipsProcessor;

// Estrutura para instruções
typedef struct {
    char code[100]; 
} Instruction;

// Variáveis Globais (Memória de Instruções)
Instruction prog_memory[MEM_SIZE]; 
int prog_size = 0;

// =============================================================
// 2. FUNÇÕES AUXILIARES
// =============================================================

// Converte nome de registrador ($t0) para índice (8)
int get_reg_index(char *reg_name) {
    if (!reg_name) return 0; 

    // Remove vírgula final, se houver
    size_t len = strlen(reg_name);
    if (len > 0 && reg_name[len-1] == ',') {
        reg_name[len-1] = '\0';
    }
    
    // Mapeamento de nomes
    if (strcmp(reg_name, "$zero") == 0) return 0;
    if (strcmp(reg_name, "$at") == 0) return 1;
    if (strcmp(reg_name, "$v0") == 0) return 2;
    if (strcmp(reg_name, "$v1") == 0) return 3;
    if (strcmp(reg_name, "$a0") == 0) return 4;
    if (strcmp(reg_name, "$t0") == 0) return 8;
    if (strcmp(reg_name, "$t1") == 0) return 9;
    if (strcmp(reg_name, "$t2") == 0) return 10;
    if (strcmp(reg_name, "$t3") == 0) return 11;
    if (strcmp(reg_name, "$t4") == 0) return 12;
    if (strcmp(reg_name, "$t5") == 0) return 13;
    if (strcmp(reg_name, "$t6") == 0) return 14;
    if (strcmp(reg_name, "$t7") == 0) return 15;
    if (strcmp(reg_name, "$s0") == 0) return 16;
    if (strcmp(reg_name, "$s1") == 0) return 17;
    if (strcmp(reg_name, "$s2") == 0) return 18;
    if (strcmp(reg_name, "$s3") == 0) return 19;
    if (strcmp(reg_name, "$t8") == 0) return 24;
    if (strcmp(reg_name, "$t9") == 0) return 25;
    if (strcmp(reg_name, "$sp") == 0) return 29;
    if (strcmp(reg_name, "$ra") == 0) return 31;

    // Se for numérico direto ($8)
    if (reg_name[0] == '$') {
        return atoi(reg_name + 1); 
    }
    return 0; 
}

// Imprime número em binário
void print_binary(int num, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        printf("%d", bit);
    }
}

// Limpa a tela (compatível com Windows e Linux/Mac)
void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Carrega arquivo .asm ou .txt para a memória
// Carrega arquivo .asm ou .txt para a memória
void load_program_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro: Nao foi possivel abrir o arquivo '%s'.\n", filename);
        exit(1);
    }

    prog_size = 0;
    char buffer[100];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // CORREÇÃO: Remove \r e \n para funcionar bem no Windows
        buffer[strcspn(buffer, "\r\n")] = 0; 
        
        // Ignora linhas vazias
        if (strlen(buffer) > 1) {
            strcpy(prog_memory[prog_size].code, buffer);
            prog_size++;
        }
        if (prog_size >= MEM_SIZE) break;
    }
    fclose(file);
    printf("Programa '%s' carregado com %d instrucoes.\n", filename, prog_size);
}

// =============================================================
// 3. TRADUTOR (Assembly -> Binário para visualização)
// =============================================================

void translate_and_print(char *line) {
    char temp[100];
    strcpy(temp, line); 
    
    char *cmd = strtok(temp, " \t,"); 
    if (!cmd) return;

    printf("Binario: ");

    // --- TIPO R (ADD, SUB, AND, OR, MULT, SLT) ---
    if (strcmp(cmd, "ADD") == 0 || strcmp(cmd, "SUB") == 0 || strcmp(cmd, "AND") == 0 || 
        strcmp(cmd, "OR") == 0 || strcmp(cmd, "MULT") == 0 || strcmp(cmd, "SLT") == 0) {
        
        char *rd_str = strtok(NULL, " \t,");
        char *rs_str = strtok(NULL, " \t,");
        char *rt_str = strtok(NULL, " \t,");
        
        // Formato R: Opcode(0) rs rt rd shamt(0) funct
        printf("000000 "); 
        print_binary(get_reg_index(rs_str), 5); printf(" "); // rs
        print_binary(get_reg_index(rt_str), 5); printf(" "); // rt
        print_binary(get_reg_index(rd_str), 5); printf(" "); // rd
        printf("00000 "); // shamt
        
        // Funct codes simplificados
        if (strcmp(cmd, "ADD") == 0) printf("100000");
        else if (strcmp(cmd, "SUB") == 0) printf("100010");
        else if (strcmp(cmd, "AND") == 0) printf("100100");
        else if (strcmp(cmd, "OR")  == 0) printf("100101");
        else if (strcmp(cmd, "MULT") == 0) printf("011000");
        else if (strcmp(cmd, "SLT") == 0) printf("101010");
    }
    // --- TIPO R Especial (SLL) ---
    else if (strcmp(cmd, "SLL") == 0) {
        char *rd_str = strtok(NULL, " \t,");
        char *rt_str = strtok(NULL, " \t,");
        char *shamt_str = strtok(NULL, " \t,");
        
        printf("000000 ");
        printf("00000 "); // rs (não usa)
        print_binary(get_reg_index(rt_str), 5); printf(" ");
        print_binary(get_reg_index(rd_str), 5); printf(" ");
        print_binary(atoi(shamt_str), 5); // shamt
        printf(" 000000"); // funct SLL
    }
    // --- TIPO I (ADDI) ---
    else if (strcmp(cmd, "ADDI") == 0) {
        char *rt_str = strtok(NULL, " \t,");
        char *rs_str = strtok(NULL, " \t,");
        char *imm_str = strtok(NULL, " \t,");
        
        printf("001000 "); // Opcode ADDI
        print_binary(get_reg_index(rs_str), 5); printf(" ");
        print_binary(get_reg_index(rt_str), 5); printf(" ");
        print_binary(atoi(imm_str), 16);
    }
    // --- TIPO I (Memória, SLTI, LUI) ---
    else if (strcmp(cmd, "LW") == 0 || strcmp(cmd, "SW") == 0 || strcmp(cmd, "SLTI") == 0 || strcmp(cmd, "LUI") == 0) {
         char *rt_str = strtok(NULL, " \t,");
         char *rs_str = NULL; 
         char *imm_str = NULL;
         int opcode_val = 0;
         
         if (strcmp(cmd, "LW") == 0) {
             opcode_val = 35; // 100011
             imm_str = strtok(NULL, "(");
             rs_str = strtok(NULL, ")");
         } else if (strcmp(cmd, "SW") == 0) {
             opcode_val = 43; // 101011
             imm_str = strtok(NULL, "(");
             rs_str = strtok(NULL, ")");
         } else if (strcmp(cmd, "SLTI") == 0) {
             opcode_val = 10; // 001010
             rs_str = strtok(NULL, " \t,");
             imm_str = strtok(NULL, " \t,");
         } else { // LUI
             opcode_val = 15; // 001111
             rs_str = "$zero"; // LUI ignora rs
             imm_str = strtok(NULL, " \t,");
         }

         print_binary(opcode_val, 6); printf(" ");
         print_binary(get_reg_index(rs_str ? rs_str : "$zero"), 5); printf(" ");
         print_binary(get_reg_index(rt_str), 5); printf(" ");
         print_binary(atoi(imm_str ? imm_str : "0"), 16);
    }
    else if (strcmp(cmd, "SAIR") == 0) {
        printf("111111 00000 00000 0000000000000000 (EXIT)");
    }
    else if (strcmp(cmd, "IMPRIMIR") == 0) {
        printf("000010 00000 00000 0000000000000000 (SYSCALL)");
    }
    else {
        printf("[Instrucao Desconhecida na View]");
    }
    printf("\n");
}

// =============================================================
// 4. EXECUTOR (Lógica de Hardware)
// =============================================================

void execute_step(MipsProcessor *cpu) {
    if (cpu->pc >= prog_size) {
        cpu->running = 0;
        return;
    }

    // Copia a linha para não alterar a memória original durante o parse
    char line[100];
    strcpy(line, prog_memory[cpu->pc].code);

    printf("\n--------------------------------\n");
    printf("Instrucao: %s\n", line);
    translate_and_print(line); // Exibe binário

    // Parser para execução
    char *cmd = strtok(line, " \t,");
    if(!cmd) return;

    // --- ARITMÉTICA ---
    if (strcmp(cmd, "ADD") == 0) {
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        cpu->regs[rd] = cpu->regs[rs] + cpu->regs[rt];
    }
    else if (strcmp(cmd, "SUB") == 0) {
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        cpu->regs[rd] = cpu->regs[rs] - cpu->regs[rt];
    }
    else if (strcmp(cmd, "MULT") == 0) {
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        cpu->regs[rd] = cpu->regs[rs] * cpu->regs[rt];
    }
    else if (strcmp(cmd, "ADDI") == 0) {
        int rt = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int imm = atoi(strtok(NULL, " \t,"));
        cpu->regs[rt] = cpu->regs[rs] + imm;
    }

    // --- LÓGICA ---
    else if (strcmp(cmd, "AND") == 0) {
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        cpu->regs[rd] = cpu->regs[rs] & cpu->regs[rt];
    }
    else if (strcmp(cmd, "OR") == 0) {
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        cpu->regs[rd] = cpu->regs[rs] | cpu->regs[rt];
    }
    else if (strcmp(cmd, "SLL") == 0) { 
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        int shamt = atoi(strtok(NULL, " \t,"));
        cpu->regs[rd] = cpu->regs[rt] << shamt;
    }

    // --- MEMÓRIA (LW / SW) ---
    else if (strcmp(cmd, "LW") == 0) {
        int rt = get_reg_index(strtok(NULL, " \t,"));
        int offset = atoi(strtok(NULL, "(")); 
        int rs = get_reg_index(strtok(NULL, ")"));     
        
        int addr = (cpu->regs[rs] + offset) / 4; // Endereço byte -> word
        if(addr >= 0 && addr < MEM_SIZE)
            cpu->regs[rt] = cpu->data_memory[addr];
    }
    else if (strcmp(cmd, "SW") == 0) {
        int rt = get_reg_index(strtok(NULL, " \t,"));
        int offset = atoi(strtok(NULL, "("));
        int rs = get_reg_index(strtok(NULL, ")"));
        
        int addr = (cpu->regs[rs] + offset) / 4;
        if(addr >= 0 && addr < MEM_SIZE)
            cpu->data_memory[addr] = cpu->regs[rt];
    }
    else if (strcmp(cmd, "LUI") == 0) {
        // LUI $rt, imm (Carrega imediato na parte superior)
        int rt = get_reg_index(strtok(NULL, " \t,"));
        int imm = atoi(strtok(NULL, " \t,"));
        cpu->regs[rt] = (imm << 16);
    }

    // --- CONDICIONAL ---
    else if (strcmp(cmd, "SLT") == 0) {
        int rd = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int rt = get_reg_index(strtok(NULL, " \t,"));
        cpu->regs[rd] = (cpu->regs[rs] < cpu->regs[rt]) ? 1 : 0;
    }
    else if (strcmp(cmd, "SLTI") == 0) {
        // SLTI $rt, $rs, imm
        int rt = get_reg_index(strtok(NULL, " \t,"));
        int rs = get_reg_index(strtok(NULL, " \t,"));
        int imm = atoi(strtok(NULL, " \t,"));
        cpu->regs[rt] = (cpu->regs[rs] < imm) ? 1 : 0;
    }
    
    // --- SISTEMA ---
    else if (strcmp(cmd, "IMPRIMIR") == 0) {
        char *arg = strtok(NULL, " \t,");
        if (arg) {
            if (arg[0] == '$') printf(">>> SAIDA (Syscall): %d\n", cpu->regs[get_reg_index(arg)]);
            else printf(">>> SAIDA (Syscall): %d\n", atoi(arg));
        }
    }
    else if (strcmp(cmd, "SAIR") == 0) {
        cpu->running = 0;
        printf("Fim do programa detectado (SAIR).\n");
        return;
    }

    // Garante que $zero é sempre 0 (Hardware constraint)
    cpu->regs[0] = 0;
    cpu->pc++;
}

// =============================================================
// 5. INTERFACE E RELATÓRIO
// =============================================================

void generate_report(MipsProcessor *cpu) {
    FILE *f = fopen("relatorio.txt", "w");
    if (!f) return;
    fprintf(f, "--- RELATORIO FINAL MIPS ---\n");
    for(int i=0; i<NUM_REGS; i++) {
        fprintf(f, "$%d: %d\n", i, cpu->regs[i]);
    }
    fclose(f);
    printf("Relatorio salvo em 'relatorio.txt'.\n");
}

void print_interface(MipsProcessor *cpu) {
    printf("\n================ MIPS SIMULATOR =================\n");
    
    if (cpu->pc < prog_size) {
        printf("PC: %04d | Instrucao: %-20s\n", cpu->pc, prog_memory[cpu->pc].code);
    } else {
        printf("PC: %04d | [Fim do Programa]\n", cpu->pc);
    }

    printf("-------------------------------------------------\n");
    printf("REGISTRADORES (Principais):\n");
    printf("$zero: %-4d | $v0: %-4d | $a0: %-4d\n", cpu->regs[0], cpu->regs[2], cpu->regs[4]);
    printf("$t0:   %-4d | $t1: %-4d | $t2: %-4d | $t3: %-4d\n", cpu->regs[8], cpu->regs[9], cpu->regs[10], cpu->regs[11]);
    printf("$s0:   %-4d | $s1: %-4d | $s2: %-4d | $s3: %-4d\n", cpu->regs[16], cpu->regs[17], cpu->regs[18], cpu->regs[19]);
    printf("=================================================\n");
}

// =============================================================
// 6. MAIN
// =============================================================

int main() {
    MipsProcessor cpu;
    
    // 1. Inicialização
    memset(cpu.regs, 0, sizeof(cpu.regs));
    memset(cpu.data_memory, 0, sizeof(cpu.data_memory));
    cpu.pc = 0;
    cpu.running = 1;

    // 2. Carregamento
    char filename[100];
    // Limpa a tela para início limpo
    clear_screen();
    printf("MIPS Simulator Iniciado.\n");
    printf("Digite o nome do arquivo (ex: teste.asm): ");
    
    if (scanf("%s", filename) != 1) return 1;
    
    // Limpa buffer do teclado
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    load_program_from_file(filename);

    if (prog_size == 0) {
        printf("Erro: Nenhum programa carregado ou arquivo vazio.\n");
        return 1;
    }

    // 3. Loop Principal
    char input;
    int automatic_mode = 0;

    while (cpu.running) {
        print_interface(&cpu);

        if (!automatic_mode) {
            printf("\n[ENTER] Passo a Passo | [A] Automatico | [S] Sair > ");
            input = getchar();

            if (input == 's' || input == 'S') break;
            else if (input == 'a' || input == 'A') {
                automatic_mode = 1;
                printf(">>> Modo Automatico Ativado! <<<\n");
            }
            else if (input != '\n') {
                while ((c = getchar()) != '\n' && c != EOF);
            }
        }
        
        execute_step(&cpu);
    }

    // 4. Finalização
    generate_report(&cpu);
    printf("\nSimulacao finalizada com sucesso.\n");
    
    printf("Pressione ENTER para fechar...");
    getchar();
    
    return 0;
}
