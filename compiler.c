#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string;
int poolsize;         // default size of text/data/stack
int line;             // line number

int *text;
int *old_text;
int *stack;           // local stack
char *data;

int *pc, *bp, *sp, ax; // virtual machine registers

int token_val;                // value of current token (mainly for number)
int *current_id,              // current parsed ID
    *symbols; 

// 标记符
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};

// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
    OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
    OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };



// tokens and classes (operators last and in precedence order)
enum {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
    };


// 用于词法分析，获取下一个标记，它将自动忽略空白字符。
void next() {

    return;
}

// 用于解析一个表达式。
void expression(int level) {
    // do nothing
}

//  语法分析的入口，分析整个 C 语言程序。
void program() {
    next();                  // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}

// 虚拟机的入口，用于解释目标代码
int eval() {
    
    int op, *tmp;
    while (1) {
        op = *pc++; // fetch opcode
        // 指令集
        if (op == IMM)       {ax = *pc++;}
        else if (op == LC)   {ax = *(char *)ax;}
        else if (op == LI)   {ax = *(int *)ax;}
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}
        else if (op == SI)   {*(int *)*sp++ = ax;}

        else if (op == PUSH) {*--sp = ax;}   

        else if (op == JMP)  {pc = (int *)*pc;}   

        else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}
        else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}

        else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}

        else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}

        else if (op == ADJ)  {sp = sp + *pc++;} 

        else if (op == LEV)  {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;} 

        else if (op == LEA)  {ax = (int)(bp + *pc++);}

        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;

        else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
        else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
        else if (op == CLOS) { ax = close(*sp);}
        else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) { ax = (int)malloc(*sp);}
        else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
        else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
        else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    // read source code to src 
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);


    // allocate memory for virtual machine
    if(!(text = old_text = malloc(poolsize))) {
        printf("could not malloc(%d) for test area\n", poolsize);
        return -1;
    }
    if(!(stack = malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }
    if(!(data = malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(stack, 0, poolsize);
    memset(data, 0, poolsize);

    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    program();
    return eval();
}