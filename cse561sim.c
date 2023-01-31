#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "basic.h"
#include "model.h"
#include "run.h"

int IQ_SIZE = 0;
int ROB_SIZE = 0;
int INSTRUCTION_NUM = 0;
trace *input_list;
result *result_list;
char command[60];
int print_type;

result initialize()
{
    result tmp;
    tmp.seq_no = 0;
    tmp.op_type = 0;
    tmp.src1 = -1;
    tmp.src2 = -1;
    tmp.dst = -1;
    tmp.fe_begin = 0;
    tmp.fe_duration = 0;
    tmp.de_begin = 0;
    tmp.de_duration = 0;
    tmp.rn_begin = 0;
    tmp.rn_duration = 0;
    tmp.di_begin = 0;
    tmp.di_duration = 0;
    tmp.is_begin = 0;
    tmp.is_duration = 0;
    tmp.rr_begin = 0;
    tmp.rr_duration = 0;
    tmp.ex_begin = 0;
    tmp.ex_duration = 0;
    tmp.wb_begin = 0;
    tmp.wb_duration = 0;
    tmp.cm_begin = 0;
    tmp.cm_duration = 0;

    tmp.phy_src1 = 0;
    tmp.phy_src2 = 0;
    tmp.phy_dst = 0;

    tmp.exe_delete = false; //exe 후에 삭제하게 되면 true로 바꿈

    return tmp;
}
void load_program(char *program_filename)
{
    FILE *prog;
    int ii;
    char line[100];

    input_list = (trace *)malloc(sizeof(trace) * 1000);
    result_list = (result *)malloc(sizeof(result) * 1000);

    prog = fopen(program_filename, "r");

    if (prog == NULL)
    {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    //map table initialize
    for(int i = 0; i<ARCHI_NUMBER;i++) map_table[i] = i;
    //ready table initialize
    for(int i = 0; i<PHY_NUMBER;i++) ready_table[i] = true;
    //Free list initialize
    for(int i = 0; i<PHY_NUMBER;i++) {
        if(i >= 0 && i <ARCHI_NUMBER) free_List[i] = false;
        else free_List[i] = true;
    }
    //is dst list initialize
    for(int i = 0; i<ARCHI_NUMBER;i++) is_dst[i] = 0;

    while (fgets(line, sizeof(line), prog) != NULL)
    {

        char *ptr = strtok(line, " ");

        trace tmp;
        result_list[INSTRUCTION_NUM] = initialize();

        result_list[INSTRUCTION_NUM].seq_no = INSTRUCTION_NUM;
        tmp.seq_no = INSTRUCTION_NUM;
        tmp.pc = ptr;

        ptr = strtok(NULL, " ");
        tmp.opType = toInteger(ptr);
        result_list[INSTRUCTION_NUM].op_type = tmp.opType;

        ptr = strtok(NULL, " ");
        tmp.dest = toInteger(ptr);
        result_list[INSTRUCTION_NUM].dst = tmp.dest;

        ptr = strtok(NULL, " ");
        tmp.src1 = toInteger(ptr);
        result_list[INSTRUCTION_NUM].src1 = tmp.src1;

        ptr = strtok(NULL, " ");
        tmp.src2 = toInteger(ptr);
        result_list[INSTRUCTION_NUM].src2 = tmp.src2;

        ptr = strtok(NULL, " ");

        input_list[INSTRUCTION_NUM++] = tmp;
    }
}

// String to Integer
int toInteger(char *str)
{
    int result;
    int puiss;

    result = 0;
    puiss = 1;
    while (('-' == (*str)) || ((*str) == '+'))
    {
        if (*str == '-')
            puiss = puiss * -1;
        str++;
    }
    while ((*str >= '0') && (*str <= '9'))
    {
        result = (result * 10) + ((*str) - '0');
        str++;
    }
    return (result * puiss);
}

int main(int argc, char *argv[])
{

    char **tokens;

    /* Error Checking */
    if (argc < 5)
    {
        printf("Error\n");
        exit(1);
    }

    char *file_name = argv[argc - 1]; // input file name

    if(argc == 5){
        print_type = 0;
        char tmp[60] = "\0";
        for(int i = 0; i<argc;i++){
            if(i == argc-1){
                char str[30];
                strcpy(str,argv[i]);
                char* tmp2 = strtok(str,"/");
                tmp2 = strtok(NULL,"\n");
                strcat(tmp,tmp2);
            }
            else {
                strcat(tmp,argv[i]);
                strcat(tmp," ");
            }
        }
        strcat(tmp," ");
        strcpy(command,tmp);        
    }
    else{
        print_type = 1;
    }
    

    ROB_SIZE = toInteger(argv[1]);
    IQ_SIZE = toInteger(argv[2]);
    load_program(file_name);

    run();
    return 0;
}
