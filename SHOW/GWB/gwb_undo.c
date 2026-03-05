/* gwb_undo.c — Undo/Redo (simplified stub for display-only build) */
#include "gwb.h"
#include <stdlib.h>

EulerOp *OpHead     = NULL;
int      OpCount    = 0;
int      Generatelog = 0;

void addeulerop(char opcode, Id solidno,
                Id ip1, Id ip2, Id ip3, Id ip4, Id ip5, Id ip6,
                double fp1, double fp2, double fp3, double fp4) {
    (void)opcode;(void)solidno;
    (void)ip1;(void)ip2;(void)ip3;(void)ip4;(void)ip5;(void)ip6;
    (void)fp1;(void)fp2;(void)fp3;(void)fp4;
}

void BeginTransaction(void)  {}
void EndTransaction(void)    {}
void UnDoTransaction(void)   {}
