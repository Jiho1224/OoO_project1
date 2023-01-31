#ifndef __BASIC_H__
#define __BASIC_H__

#include <stdint.h>
#include <stdbool.h>


#define ARCHI_NUMBER    67 //architectural register의 개수
#define PHY_NUMBER      134 //physical register의 개수


#define WIDTH           3          //2-WAY processor 구현
extern int IQ_SIZE;
extern int ROB_SIZE;
extern int INSTRUCTION_NUM;



extern int DE_NUM;
extern int RN_NUM;
extern int DI_NUM;
extern int RR_NUM;
extern int IQ_NUM;
extern int EXE_NUM;
extern int WB_idx;
extern int ROB_NUM;
extern int commit_idx;

extern int bDay;

bool ready_table[PHY_NUMBER]; //physical register가 사용가능한지
int map_table[ARCHI_NUMBER]; //각 architectural register가 어떤 physical register에 mapping 되었는지
bool free_List[PHY_NUMBER];
int is_dst[PHY_NUMBER];

typedef struct t_t{

    int seq_no;

    char* pc;
    int opType;
    int dest;
    int src1;
    int src2;
} trace;

typedef struct inst_t {
    short opcode;

    /*R-type*/
    short func_code;

    union {
        /* R-type or I-type: */
        struct {
	    unsigned char rs;
	    unsigned char rt;

	    union {
	        short imm;

	        struct {
		    unsigned char rd;
		    unsigned char shamt;
		} r;
	    } r_i;
	} r_i;
        /* J-type: */
        uint32_t target;
    } r_t;

    uint32_t value;
} instruction;

//Issue Queue
typedef struct is_q{
    int inst;

    int src1;
    bool src1_avail;

    int src2;
    bool src2_avail;

    int dest;
    int bday;

} IQ_comp;

typedef struct r_t{

    bool ready;
    int toFree;
    bool isCommitted;

    int seq_no;
    int op_type;

    int src1;
    int src2;
    int dst;

    int phy_src1;
    int phy_src2;
    int phy_dst;
    int old_phy_dst;

    int fe_begin;
    int fe_duration;

    int de_begin;
    int de_duration;

    int rn_begin;
    int rn_duration;

    int di_begin;
    int di_duration;

    int is_begin;
    int is_duration;

    int rr_begin;
    int rr_duration;

    int ex_begin;
    int ex_duration;

    int wb_begin;
    int wb_duration;

    int cm_begin;
    int cm_duration;

    bool exe_delete;

} result;

typedef struct f_q{

    result info;
    IQ_comp isQ_info;

} Issue_Queue;

typedef struct f_r{
    bool ready;
    result info;
    int toFree;
    bool isCommitted;
} Reorder_Buffer;

extern result DE[WIDTH];
extern result RN[WIDTH];
extern result DI[WIDTH];
extern result RR[WIDTH];
extern result execute_list[WIDTH*5];
extern result WB[WIDTH*5];
extern Issue_Queue *IQ;

extern trace* input_list;
extern int READ_INST_NUM;
extern result* result_list;
extern int CYCLE_NUM;

extern int debug;
extern char command[60];

extern int print_type;

Issue_Queue*        delete_IQ(Issue_Queue* IQ, int index);
// Reorder_Buffer**                delete_ROB(Reorder_Buffer** ROB, int index);

#endif
