#include <stdio.h>
#include <malloc.h>

#include "basic.h"
#include "model.h"
#include "run.h"

int READ_INST_NUM = 0;
int CYCLE_NUM = 0;
int debug = 0;

// run pipeline cycle
void run()
{
    IQ = (Issue_Queue *)malloc(sizeof(Issue_Queue) * IQ_SIZE);
    ROB = (Reorder_Buffer *)malloc(sizeof(Reorder_Buffer) * ROB_SIZE);
    while (advance_cycle())
    {

        commit();

        writeBack();

        execute();

        regRead();

        issue();

        dispatch();

        renaming();

        decode();

        fetch();

        CYCLE_NUM++;
    }
    if (print_type == 0)
    {
        print_result();
        print_command();
    }
    else
    {
        print_scope();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : advance_cycle                                   */
/*                                                             */
/* Purpose   : (1) advance the simulator cycle                 */
/*             (2) if pipeline is empty and the trace is       */
/*              depleted, return false to terminate the loop   */
/*                                                             */
/***************************************************************/
bool advance_cycle()
{
    // trace가 고갈된 조건 + pipeline을 모두 실행한 조건
    if (commit_idx >= INSTRUCTION_NUM)
        return false;
    else
        return true;
}

void print_result()
{

    for (int i = 0; i < INSTRUCTION_NUM; i++)
    {
        printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
               result_list[i].seq_no, result_list[i].op_type, result_list[i].src1, result_list[i].src2,
               result_list[i].dst, result_list[i].fe_begin, result_list[i].fe_duration, result_list[i].de_begin, result_list[i].de_duration,
               result_list[i].rn_begin, result_list[i].rn_duration, result_list[i].di_begin, result_list[i].di_duration,
               result_list[i].is_begin, result_list[i].is_duration, result_list[i].rr_begin, result_list[i].rr_duration,
               result_list[i].ex_begin, result_list[i].ex_duration, result_list[i].wb_begin, result_list[i].wb_duration,
               result_list[i].cm_begin, result_list[i].cm_duration);
    }
}

void print_command()
{

    printf("# === Simulator Command =========\n");
    printf("# %s\n", command);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE = %d\n", ROB_SIZE);
    printf("# IQ_SIZE  = %d\n", IQ_SIZE);
    printf("# WIDTH    = %d\n", WIDTH);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count = %d\n", INSTRUCTION_NUM);
    printf("# Cycles                    = %d\n", CYCLE_NUM - 1);
    printf("# Instructions Per Cycle    = %.2f\n", INSTRUCTION_NUM / (float)(CYCLE_NUM - 1));
}

void print_scope()
{
    int start_number = 0;
    for (int i = 0; i < INSTRUCTION_NUM; i++)
    {   
        
        if (i == 0 || i == 35 || i == 70)
        {
            if (i == 0)
                start_number = 0;
            else if (i == 35)
                start_number = 11;
            else
                start_number = 24;

            printf("                                    	");
            for (int i = start_number; i < 150; i++)
            {
                printf("0  ");
            }
            printf("\n");
            printf("                                    	");
            for (int i = start_number; i < 150; i++)
            {
                if (i < 100)
                    printf("0  ");
                else
                    printf("1  ");
            }
            printf("\n");
            printf("                                    	");
            for (int i = start_number; i < 150; i++)
            {
                if (i < 100)
                    printf("%d  ", i / 10);
                else
                    printf("%d  ", (i - 100) / 10);
            }
            printf("\n");
            printf("                                    	");
            for (int i = start_number; i < 150; i++)
            {
                printf("%d  ", i % 10);
            }
            printf("\n");
        }


        printf("%8d fu{%d} src{%3d,%3d} dst{%3d}   ", result_list[i].seq_no, result_list[i].op_type, result_list[i].src1, result_list[i].src2, result_list[i].dst);
        for (int j = start_number; j < result_list[i].cm_begin + result_list[i].cm_duration; j++)
        {
            if (j < result_list[i].fe_begin)
                printf("   ");

            if (j >= result_list[i].fe_begin && j < result_list[i].de_begin)
                printf("FE ");

            if (j >= result_list[i].de_begin && j < result_list[i].rn_begin)
                printf("DE ");

            if (j >= result_list[i].rn_begin && j < result_list[i].di_begin)
                printf("RN ");

            if (j >= result_list[i].di_begin && j < result_list[i].is_begin)
                printf("DI ");

            if (j >= result_list[i].is_begin && j < result_list[i].rr_begin)
                printf("IS ");

            if (j >= result_list[i].rr_begin && j < result_list[i].ex_begin)
                printf("RR ");

            if (j >= result_list[i].ex_begin && j < result_list[i].wb_begin)
                printf("EX ");

            if (j >= result_list[i].wb_begin && j < result_list[i].cm_begin)
                printf("WB ");

            if (j >= result_list[i].cm_begin && j < result_list[i].cm_begin + result_list[i].cm_duration)
                printf("CM ");
            
        }
        printf("\n");
    }
}