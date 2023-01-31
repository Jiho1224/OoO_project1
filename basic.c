#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#include "basic.h"


Issue_Queue* delete_IQ(Issue_Queue* IQ, int index){

    for(int i = index; i<IQ_SIZE-1;i++){
        IQ[i] = IQ[i+1];
    }

    return IQ;
}

// Reorder_Buffer** delete_ROB(Reorder_Buffer** ROB, int index){
//     printf("delete rob in // rob size is %d\n", ROB_NUM);
//     for(int i = index; i<ROB_NUM-1;i++){
//         ROB[i] = ROB[i+1];
//     }
//     return ROB;
// }