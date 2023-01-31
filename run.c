#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#include "basic.h"
#include "run.h"

int DE_NUM = 0;
int RN_NUM = 0;
int DI_NUM = 0;
int RR_NUM = 0;
int IQ_NUM = 0;
int EXE_NUM = 0;
int WB_idx = 0;
int ROB_NUM = 0;
int commit_idx = 0;

int bDay = 0;

result DE[WIDTH];
result RN[WIDTH];
result DI[WIDTH];
Issue_Queue* IQ;
result RR[WIDTH];
result execute_list[WIDTH*5];
result WB[WIDTH*5];

result* result_list;
int map_table[ARCHI_NUMBER];

int INSTRUCTION_NUM;
int READ_INST_NUM;
int CYCLE_NUM;
int debug;

int past_write_reg = -1;

bool ready_table[PHY_NUMBER]; //physical register가 사용가능한지
int map_table[ARCHI_NUMBER]; //각 architectural register가 어떤 physical register에 mapping 되었는지
bool free_List[PHY_NUMBER]; // free 상태인 physical register
int is_dst[PHY_NUMBER]; //각 physical register가 destination인지

/***************************************************************/
/*                                                             */
/* Procedure : commit                                          */
/*                                                             */
/* Purpose   : commit up to "WIDTH" consecutive "Ready"        */
/*             instruction from the head of ROB                */
/*                                                             */
/***************************************************************/
void commit(){
   // printf("[CM] ROB NUM %d\n",ROB_NUM);
    if(ROB_NUM > 0){
        
        for(int i = 0; i<WIDTH;i++){
            if(ROB_NUM == 0) break;

            for(int j = 0; j < ROB_NUM; j++){
                if(ROB[j].info.seq_no == commit_idx){
                    result_list[commit_idx] = ROB[j].info;
                    result_list[commit_idx].cm_duration = CYCLE_NUM - ROB[j].info.cm_begin + 1;

                    //once all older instructions have committed, free register
                    free_List[ROB[j].info.old_phy_dst] = true;
                    commit_idx++;

                    for(int k = j; k<ROB_NUM-1;k++){
                        if(ROB_NUM == 1) break;
                        ROB[k] = ROB[k+1];
                    }
                    ROB_NUM--;   
                    break;
                }
            }
        }
       // print_ROB();

    }
    
}

/////////////////////// out of Order //////////////////////////////////


/***************************************************************/
/*                                                             */
/* Procedure : writeBack                                       */
/*                                                             */
/* Purpose   : for each instruction in WB,                     */
/*           mark the instruction as "Ready" in its entry      */
/*           in the ROB                                        */
/*                                                             */
/***************************************************************/

void writeBack(){
    
    if(ROB_NUM < ROB_SIZE && WB_idx > 0){
        
        int start_WB_indx = WB_idx;
        for(int i = 0; i < start_WB_indx;i++){
     
            ROB[ROB_NUM].ready = true;
            ROB[ROB_NUM].isCommitted = false;
            ROB[ROB_NUM].toFree = WB[0].toFree;
            ROB[ROB_NUM].info = WB[0];
            ROB[ROB_NUM].info.wb_duration = CYCLE_NUM - ROB[ROB_NUM].info.wb_begin + 1;
            ROB[ROB_NUM].info.cm_begin = CYCLE_NUM + 1;

            for(int j = 0; j<WB_idx-1;j++){
                if(WB_idx == 1) break;
                WB[j] = WB[j+1];
            }

            WB_idx--;
            ROB_NUM++;            

              
        }

    }
}

/***************************************************************/
/*                                                             */
/* Procedure : execute                                         */
/*                                                             */
/* Purpose   : check for instructions that are finishing       */
/*             this cycle                                      */
/*                                                             */
/*        - remove the instruction from the execute_list       */
/*        - add the instruction to WB                          */
/*                                                             */
/***************************************************************/

void execute(){
    //printf("EXECUTE : EXE NUM is %d\n",EXE_NUM);
    int start_EXE_NUM = EXE_NUM;
    for(int i = 0; i<start_EXE_NUM;i++){
        //printf("exe 시작\n");
        //각 cycle마다 ex duration을 하나씩 더해주면서 op type 별로 WB array로 이동하는
        //시점을 조정해준다.

        execute_list[i].ex_duration += 1;

        //Type 0 has a latency of 1 cycle
        if(execute_list[i].op_type == 0){
            //ex_duration이 1일 경우 WB array로 이동
            if(execute_list[i].ex_duration == 1){

                WB[WB_idx] = execute_list[i];
                WB[WB_idx++].wb_begin = CYCLE_NUM + 1;

                execute_list[i].exe_delete = true;
            }
        }
        //Type 1 has a latency of 2 cycles
        else if(execute_list[i].op_type == 1){
            //ex_duation이 2일 경우 WB array로 이동
            if(execute_list[i].ex_duration == 2){

                WB[WB_idx] = execute_list[i];
                WB[WB_idx++].wb_begin = CYCLE_NUM + 1;

                execute_list[i].exe_delete = true;
            }
        }
        //Type 2 has a latency of 5 cycles
        else{
            //ex_duration이 5일 경우 WB array로 이동
            if(execute_list[i].ex_duration == 5){

                WB[WB_idx] = execute_list[i];
                WB[WB_idx++].wb_begin = CYCLE_NUM + 1;

                execute_list[i].exe_delete = true;
            }
        }
    }
    
    int tmp_trace = 0;
    for(int i = 0; i<start_EXE_NUM;i++){
        //만약 exe 결과 exe가 완료되어서 list에서 삭제해도 된다면
        //list에서 삭제
        if(execute_list[tmp_trace].exe_delete){
            if(execute_list[tmp_trace].dst != -1) {
                is_dst[execute_list[tmp_trace].phy_dst] -= 1;
                // printf("%d th is dst num is %d\n",tmp_trace,is_dst[execute_list[tmp_trace].phy_dst]);
                //if(is_dst[execute_list[tmp_trace].phy_dst] == 0) 
                ready_table[execute_list[tmp_trace].phy_dst] = true;
            }
            for(int k = 0; k < IQ_NUM;k++){
                if(IQ[k].isQ_info.src1 == execute_list[tmp_trace].phy_dst && execute_list[tmp_trace].phy_dst != -1){
                    IQ[k].isQ_info.src1_avail = ready_table[execute_list[tmp_trace].phy_dst];
                }

                if(IQ[k].isQ_info.src2 == execute_list[tmp_trace].phy_dst && execute_list[tmp_trace].phy_dst != -1){
                    IQ[k].isQ_info.src2_avail = ready_table[execute_list[tmp_trace].phy_dst];
                }
            }

            delete_EXE(tmp_trace); //i번째를 삭제하고
            EXE_NUM--; //EXE LIST의 원소 개수를 하나 줄여준다
        }
        else{
            tmp_trace++;
        }
    }

    if(debug == 1){
        
        for(int i = 0; i< EXE_NUM;i++){
            if(i == 0) printf("\n\n !!!!Execute result(in exe list)!!!! \n\n");
            printf("[EXE] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                execute_list[i].seq_no,execute_list[i].op_type, execute_list[i].src1, execute_list[i].src2,
                execute_list[i].dst, execute_list[i].fe_begin,execute_list[i].fe_duration,execute_list[i].de_begin,execute_list[i].de_duration,
                execute_list[i].rn_begin,execute_list[i].rn_duration, execute_list[i].di_begin,execute_list[i].di_duration,
                execute_list[i].is_begin,execute_list[i].is_duration, execute_list[i].rr_begin,execute_list[i].rr_duration,
                execute_list[i].ex_begin,execute_list[i].ex_duration,execute_list[i].wb_begin,execute_list[i].wb_duration,
                execute_list[i].cm_begin,execute_list[i].cm_duration);
        }

        for(int i = 0; i<WB_idx;i++){
            if(i == 0) printf("\n\n !!!!Execute result(wb)!!!!\n\n");
            printf("[WB] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                WB[i].seq_no,WB[i].op_type, WB[i].src1, WB[i].src2,
                WB[i].dst, WB[i].fe_begin,WB[i].fe_duration,WB[i].de_begin,WB[i].de_duration,
                WB[i].rn_begin,WB[i].rn_duration, WB[i].di_begin,WB[i].di_duration,
                WB[i].is_begin,WB[i].is_duration, WB[i].rr_begin,WB[i].rr_duration,
                WB[i].ex_begin,WB[i].ex_duration,WB[i].wb_begin,WB[i].wb_duration,
                WB[i].cm_begin,WB[i].cm_duration);

        }
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : regRead                                         */
/*                                                             */
/* Purpose   : process register read bundle and advance it     */
/*             from RR to execute_list                         */
/*                                                             */
/*        - just pass the information of each instruction      */
/*          in the register-read bundle                        */
/*                                                             */
/***************************************************************/
void regRead(){

    if( EXE_NUM < WIDTH*5 && RR_NUM > 0){
        int start_EXE_NUM = EXE_NUM;
        for(int i = start_EXE_NUM ; i<WIDTH*5; i++){

            if(RR_NUM == 0) break;

            execute_list[i] = RR[0];
            execute_list[i].rr_duration = CYCLE_NUM - execute_list[i].rr_begin + 1;
            execute_list[i].ex_begin = CYCLE_NUM + 1;

            for(int j = 0; j<RR_NUM-1;j++){
                if(RR_NUM == 1) break;
                RR[j] = RR[j+1];
            }

            RR_NUM--;
            EXE_NUM++;
          
        }


        for(int i = 0; i<WIDTH;i++){
            if(debug == 1){
                printf("[exe list] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                execute_list[i].seq_no,execute_list[i].op_type, execute_list[i].src1, execute_list[i].src2,
                execute_list[i].dst, execute_list[i].fe_begin,execute_list[i].fe_duration,execute_list[i].de_begin,execute_list[i].de_duration,
                execute_list[i].rn_begin,execute_list[i].rn_duration, execute_list[i].di_begin,execute_list[i].di_duration,
                execute_list[i].is_begin,execute_list[i].is_duration, execute_list[i].rr_begin,execute_list[i].rr_duration,
                execute_list[i].ex_begin,execute_list[i].ex_duration,execute_list[i].wb_begin,execute_list[i].wb_duration,
                execute_list[i].cm_begin,execute_list[i].cm_duration);
            }
        }
    }
}
/***************************************************************/
/*                                                             */
/* Procedure : issue                                           */
/*                                                             */
/* Purpose   : issue up to 3 oldest instructions from the IQ   */
/*        - Remove the instruction from the IQ                 */
/*        - Wakeup dependent instructions in the IQ            */
/*                                                             */
/***************************************************************/
void issue(){
    int issued = 0; // width = 3개의 instruction을 issue 한다.
    int start = 0;


    while(issued < WIDTH && IQ_NUM > 0 && RR_NUM < WIDTH && start < IQ_NUM){
        //source 2개다 available하다면 issue한다.
       
        if(IQ[start].isQ_info.src1_avail && IQ[start].isQ_info.src2_avail){
            //printf("%d is available\n",IQ[start].info.seq_no);
            
             issued++; //issue한 개수 추가
            //reg read 배열에 issue한 instruction을 넣어준다.
            RR[RR_NUM] = IQ[start].info;
            RR[RR_NUM].is_duration = CYCLE_NUM - RR[RR_NUM].is_begin + 1;
            RR[RR_NUM].rr_begin = CYCLE_NUM + 1;

            

            //remove the instruction from the IQ
            delete_IQ(IQ,start);
            IQ_NUM--;



            RR_NUM++;

        }

        //만약 그렇지 않다면 issue하지 않고 다음 instruction을 살펴본다
        else{
            start++;
        }

        
    }

    for(int i = 0; i<IQ_NUM;i++){

            if(debug == 1){
                
                printf("[Issue Queue] seq no : %d, src1 : %d src2: %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                IQ[i].info.seq_no,IQ[i].isQ_info.src1_avail,IQ[i].isQ_info.src2_avail,IQ[i].info.op_type, IQ[i].info.src1, IQ[i].info.src2,
                IQ[i].info.dst, IQ[i].info.fe_begin,IQ[i].info.fe_duration,IQ[i].info.de_begin,IQ[i].info.de_duration,
                IQ[i].info.rn_begin,IQ[i].info.rn_duration, IQ[i].info.di_begin,IQ[i].info.di_duration,
                IQ[i].info.is_begin,IQ[i].info.is_duration, IQ[i].info.rr_begin,IQ[i].info.rr_duration,
                IQ[i].info.ex_begin,IQ[i].info.ex_duration,IQ[i].info.wb_begin,IQ[i].info.wb_duration,
                IQ[i].info.cm_begin,IQ[i].info.cm_duration);
            }
    }

    for(int i = 0; i < RR_NUM;i++){
        if(debug == 1){
            printf("[RR] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                RR[i].seq_no,RR[i].op_type, RR[i].src1, RR[i].src2,
                RR[i].dst, RR[i].fe_begin,RR[i].fe_duration,RR[i].de_begin,RR[i].de_duration,
                RR[i].rn_begin,RR[i].rn_duration, RR[i].di_begin,RR[i].di_duration,
                RR[i].is_begin,RR[i].is_duration, RR[i].rr_begin,RR[i].rr_duration,
                RR[i].ex_begin,RR[i].ex_duration,RR[i].wb_begin,RR[i].wb_duration,
                RR[i].cm_begin,RR[i].cm_duration);
        }
    }
    
}
/////////////////////////////////////////////////////////////////////////

///////////////////// inOrder //////////////////////////////////////////

/***************************************************************/
/*                                                             */
/* Procedure : dispatch                                        */
/*                                                             */
/* Purpose   : (1) # of free IQ entries < size of the dispatch */
/*             bundle in DI, then do nothing                   */
/*                                                             */
/*             (2) # of free IQ entries >= size of the         */
/*             dispatch bundle in DI, dispatch all inst from   */
/*             DI to the IQ                                    */
/*                                                             */
/***************************************************************/
void dispatch(){
    
    if(IQ_SIZE - IQ_NUM >= DI_NUM && DI_NUM > 0){

        // DI array에 있는 instruction을 Issue queue array에 담는다.

        int start_IQ_NUM = IQ_NUM;

        for(int i = start_IQ_NUM; i < start_IQ_NUM + WIDTH ; i++){
            if(DI_NUM == 0 || i >= IQ_SIZE) break;

            //가장 앞의 fetched instruction을 iq에 넣는다.
            IQ[i].info = DI[0];
            IQ[i].info.di_duration = CYCLE_NUM - IQ[i].info.di_begin + 1;
            IQ[i].info.is_begin = CYCLE_NUM + 1;
            

            for(int j = 0; j<DI_NUM-1;j++){
                if(DI_NUM == 1) break;
                DI[j] = DI[j+1];

            }

            DI_NUM--;
            IQ_NUM++;

            IQ[i].isQ_info.inst = IQ[i].info.op_type;
            IQ[i].isQ_info.src1 = IQ[i].info.phy_src1;
            if(IQ[i].isQ_info.src1 == -1) IQ[i].isQ_info.src1_avail = true;
            else IQ[i].isQ_info.src1_avail = ready_table[IQ[i].isQ_info.src1];
            
            IQ[i].isQ_info.src2 = IQ[i].info.phy_src2;
            if(IQ[i].isQ_info.src2 == -1) IQ[i].isQ_info.src2_avail = true;
            else IQ[i].isQ_info.src2_avail = ready_table[IQ[i].isQ_info.src2];

            IQ[i].isQ_info.dest = IQ[i].info.phy_dst;
            ready_table[IQ[i].isQ_info.dest] = false;

            IQ[i].isQ_info.bday = bDay++;

            
        }

        
        for(int i = 0; i<IQ_NUM;i++){

            if(debug == 1){
                
                printf("[IS] seq no : %d, src1 : %d src2: %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                IQ[i].info.seq_no,IQ[i].isQ_info.src1_avail,IQ[i].isQ_info.src2_avail,IQ[i].info.op_type, IQ[i].info.src1, IQ[i].info.src2,
                IQ[i].info.dst, IQ[i].info.fe_begin,IQ[i].info.fe_duration,IQ[i].info.de_begin,IQ[i].info.de_duration,
                IQ[i].info.rn_begin,IQ[i].info.rn_duration, IQ[i].info.di_begin,IQ[i].info.di_duration,
                IQ[i].info.is_begin,IQ[i].info.is_duration, IQ[i].info.rr_begin,IQ[i].info.rr_duration,
                IQ[i].info.ex_begin,IQ[i].info.ex_duration,IQ[i].info.wb_begin,IQ[i].info.wb_duration,
                IQ[i].info.cm_begin,IQ[i].info.cm_duration);
            }
        }
    }
}


/***************************************************************/
/*                                                             */
/* Procedure : renaming                                        */
/*                                                             */
/* Purpose   : (1) DI is not empty => do nothing               */
/*                                                             */
/*             (2) DI is empty => process the rename bundle    */
/*             and advance it from RN to DI                    */
/*                                                             */
/*              - Rename its source registers                  */
/*              - Rename its destination register              */
/*                                                             */
/***************************************************************/
void renaming(){

    if(DI_NUM < WIDTH && RN_NUM > 0){
        int start_DI_NUM = DI_NUM;
        
        //1.RN array의 instruction을 차례대로 DI에 fetch
        for(int i = start_DI_NUM ; i<WIDTH; i++){

            if(RN_NUM == 0) break; //더 이상 fetch된 instruction이 없으면 break

            // ** 1. DI로 instruction 가져오기 **//
            //가장 앞의 fetched instruction을 di에 넣는다.
            DI[i] = RN[0];
            DI[i].rn_duration = CYCLE_NUM - DI[i].rn_begin + 1;
            DI[i].di_begin = CYCLE_NUM + 1;

            // //DE array의 instruction들을 한 칸 씩 앞으로 당겨준다.
            for(int j = 0; j<RN_NUM-1;j++){
                if(RN_NUM == 1) break;
                RN[j] = RN[j+1];
            }

            RN_NUM--;
            DI_NUM++;
            if (debug == 2) printf("[Register] physical dst : %d, src1: %d, src2: %d\n",DI[i].phy_dst, DI[i].phy_src1, DI[i].phy_src2);

            //** 2. 가져온 instruction renaming 하기 **//
            // RAW : read after write

            //src1 renaming
            DI[i].phy_dst = -1;
            DI[i].phy_src1 = -1;
            DI[i].phy_src2 = -1;
            DI[i].old_phy_dst = -1;

            if(DI[i].src1 != -1){
                int reg = -1;
                reg = map_table[DI[i].src1];
                DI[i].phy_src1 = reg;
                //printf("[DI RENAME] src1 : %d ",map_table[DI[i].src1]);
            }

            //src2 renaming
            if(DI[i].src2 != -1){
                int reg = -1;
                reg = map_table[DI[i].src2];
                DI[i].phy_src2 = reg;
                //printf("src2 : %d ",map_table[DI[i].src2]);
            }

            //dst renaming
            if(DI[i].dst != -1){
                DI[i].old_phy_dst = map_table[DI[i].dst];

                int reg = findPhysicalRegister();
                map_table[DI[i].dst] = reg;
                DI[i].phy_dst = reg;
                is_dst[DI[i].phy_dst] += 1;

            }
        }

       
        
        for(int i = 0; i<WIDTH;i++){

            if(debug == 1){
                printf("[Register] physical dst : %d, src1: %d, src2: %d\n",DI[i].phy_dst, DI[i].phy_src1, DI[i].phy_src2);
                printf("[DI] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                DI[i].seq_no,DI[i].op_type, DI[i].src1, DI[i].src2,
                DI[i].dst, DI[i].fe_begin,DI[i].fe_duration,DI[i].de_begin,DI[i].de_duration,
                DI[i].rn_begin,DI[i].rn_duration, DI[i].di_begin,DI[i].di_duration,
                DI[i].is_begin,DI[i].is_duration, DI[i].rr_begin,DI[i].rr_duration,
                DI[i].ex_begin,DI[i].ex_duration,DI[i].wb_begin,DI[i].wb_duration,
                DI[i].cm_begin,DI[i].cm_duration);
            }
        }
    }

}


/***************************************************************/
/*                                                             */
/* Procedure : decode                                          */
/*                                                             */
/* Purpose   : (1) RN is  not empty => do nothing              */
/*                                                             */
/*             (2) RN is empty => advance the decode bundle    */
/*             from DE to RN                                   */
/*                                                             */
/***************************************************************/
void decode(){
    //비어있는 RN 칸이 있다면

    if(RN_NUM <WIDTH && DE_NUM > 0){
        // 1. DE array에 담겨있는 instruction을 가져온 후 decode
        
        // 2. decode가 완료되면 Rename array, RN에 넣는다.
        int start_RN_NUM = RN_NUM;
       
        for(int i = start_RN_NUM ; i<WIDTH; i++){
            if(DE_NUM == 0 || RN_NUM > WIDTH) break; //더 이상 fetch된 instruction이 없으면 break

            //가장 앞의 fetched instruction을 rn에 넣는다.
            RN[i] = DE[0];
            RN[i].de_duration = CYCLE_NUM - RN[i].de_begin + 1;
            RN[i].rn_begin = CYCLE_NUM + 1;

            // //DE array의 instruction들을 한 칸 씩 앞으로 당겨준다.
            for(int j = 0; j<DE_NUM-1;j++){
                if(DE_NUM == 1) break;
                DE[j] = DE[j+1];
            }

            DE_NUM--;
            RN_NUM++;
        }
        for(int i = 0; i<WIDTH;i++){
            if(debug == 1){
                printf("[RN] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                RN[i].seq_no,RN[i].op_type, RN[i].src1, RN[i].src2,
                RN[i].dst, RN[i].fe_begin,RN[i].fe_duration,RN[i].de_begin,RN[i].de_duration,
                RN[i].rn_begin,RN[i].rn_duration, RN[i].di_begin,RN[i].di_duration,
                RN[i].is_begin,RN[i].is_duration, RN[i].rr_begin,RN[i].rr_duration,
                RN[i].ex_begin,RN[i].ex_duration,RN[i].wb_begin,RN[i].wb_duration,
                RN[i].cm_begin,RN[i].cm_duration);
            }
        }
     }

    
}


/***************************************************************/
/*                                                             */
/* Procedure : fetch                                           */
/*                                                             */
/* Purpose   : (1) Nothing IF                                  */
/*               - there are no more instructions in the trace */
/*                 file                                        */
/*               - DE is not empty                             */
/*             (2) fetch up to 2 instructions from the trace   */
/*             file                                            */
/*                                                             */
/***************************************************************/
void fetch(){
    //DE가 EMPTY 거나 TRACE FILE에 INSTRUCTION이 남았을 경우
    int start_DE_NUM = DE_NUM;
    
    if(DE_NUM < WIDTH && READ_INST_NUM < INSTRUCTION_NUM && ROB_SIZE - ROB_NUM >= WIDTH){
        for(int i = start_DE_NUM; i<WIDTH;i++){
            
            if(READ_INST_NUM > INSTRUCTION_NUM) break;
            
            DE[i] = result_list[READ_INST_NUM];
            DE[i].fe_begin = CYCLE_NUM;
            DE[i].fe_duration = 1;
            DE[i].de_begin = CYCLE_NUM + 1;
            DE[i].ready = false;
            DE[i].isCommitted = false;

            if(DE[i].dst != -1) DE[i].toFree = map_table[DE[i].dst];
            else DE[i].toFree = -1;

            READ_INST_NUM++;
            DE_NUM++;
            
        }
            
        for(int i = 0; i<WIDTH;i++){

            if(debug == 1){
                printf("[DE] seq no : %d ===> fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n",
                DE[i].seq_no,DE[i].op_type, DE[i].src1, DE[i].src2,
                DE[i].dst, DE[i].fe_begin,DE[i].fe_duration,DE[i].de_begin,DE[i].de_duration,
                DE[i].rn_begin,DE[i].rn_duration, DE[i].di_begin,DE[i].di_duration,
                DE[i].is_begin,DE[i].is_duration, DE[i].rr_begin,DE[i].rr_duration,
                DE[i].ex_begin,DE[i].ex_duration,DE[i].wb_begin,DE[i].wb_duration,
                DE[i].cm_begin,DE[i].cm_duration);
            }
        }

    }
}

// 사용 가능한 physical register를 찾는 함수
int findPhysicalRegister(){
    for(int i = 1; i<=PHY_NUMBER;i++){
        if(free_List[i] == true){
            free_List[i] = false;
            return i;
        }
    }
    return -1;
}


// execute이후 완료된 instruction은 execute list에서 삭제하는 함수
void delete_EXE(int index){
    for(int i = index; i<EXE_NUM;i++){
        execute_list[i] = execute_list[i+1];
    }
}

void delete_WB(int index){
    for(int i = index; i<WB_idx;i++){
        ROB[i] = ROB[i+1];
    }
}
