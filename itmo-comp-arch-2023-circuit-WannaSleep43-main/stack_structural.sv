module stack_structural_normal(
    inout wire[3:0] IO_DATA, 
    input wire RESET,
    input wire CLK,
    input wire[1:0] COMMAND,
    input wire[2:0] INDEX
    ); 
    
    wire cmd0, cmd1, cmd2, cmd3;
    decode2bit dec2bit(cmd0, cmd1, cmd2, cmd3, COMMAND);

    wire[2:0] cmd3bit3;
    assign cmd3bit3[0] = cmd3, cmd3bit3[1] = cmd3, cmd3bit3[2] = cmd3;

    wire clkPlus, clkMinus;
    and andplus(clkPlus, CLK, cmd1), andminus(clkMinus, CLK, cmd2);

    wire[2:0] pPlus, pMinus;
    
    pointerT plus(pPlus, RESET, clkPlus), minus(pMinus, RESET, clkMinus);

    wire[2:0] pointer;
    diffModulo5 plusMinus(pointer, pPlus, pMinus);

    wire rw0, notrw0;
    or orrw0(rw0, cmd0, cmd1);
    not notrw(notrw0, rw0);

    wire[2:0] modindex;
    mod5forInd mod(modindex, INDEX);

    wire[2:0] indPlOne;
    plusOne add(indPlOne, modindex);

    wire[2:0] indMask;
    and3bit maskInd(indMask, indPlOne, cmd3bit3);

    wire[2:0] pointerMinusInd;
    diffModulo5 pointerInd(pointerMinusInd, pointer, indMask);

    wire[2:0] finalIndex;
    minusOne pointerMinus(finalIndex, cmd2, pointerMinusInd);

    wire[3:0] memOut;
    StackMemory stMemory(memOut, CLK, RESET, cmd1, IO_DATA, finalIndex);

    cmos cmos0(IO_DATA[0], memOut[0], notrw0, rw0), cmos1(IO_DATA[1], memOut[1], notrw0, rw0), cmos2(IO_DATA[2], memOut[2], notrw0, rw0), cmos3(IO_DATA[3], memOut[3], notrw0, rw0);
endmodule

module RS(output wire Q, output wire notQ, input wire reset, input wire R, input wire CLK, input wire S);
    and and1(and1_out, R, CLK), and2(and2_out, S, CLK);
    nor nor1(Q, notQ, and1_out), nor2(res, Q, and2_out);
    or or1(notQ, reset, res);
endmodule

module Dtrigger (output wire Q, output wire notQ,input wire Reset, input wire CLK, input wire D);
    wire notD;
    not not1(notD, D);
    RS RS1(Q, notQ, Reset, notD, CLK, D);
endmodule

module DynamicD(output wire Q, output wire notQ, input wire Reset, input wire CLK, input wire D);

    wire notCLK, D1, notD1;
    not not1(notCLK, CLK);

    Dtrigger Dtrigger1(D1, notD1, Reset, CLK, D);
    Dtrigger Dtrigger2(Q, notQ, Reset, notCLK, D1);
endmodule

module Ttrigger(output wire Q, input wire Reset, input wire CLK);
    wire nQ;
    DynamicD Dynamic1(Q, nQ, Reset, CLK, nQ);
endmodule

module and4bit(output wire[3:0] q, input wire[3:0] a, input wire[3:0] b);
    and and0(q[0], a[0], b[0]), and1(q[1], a[1], b[1]), and2(q[2], a[2], b[2]), and3(q[3], a[3], b[3]);
endmodule

module or4bit(output wire[3:0] q, input wire[3:0] a, input wire[3:0] b);
    or or0(q[0], a[0], b[0]), or1(q[1], a[1], b[1]), or2(q[2], a[2], b[2]), or3(q[3], a[3], b[3]);
endmodule

module Memory(output wire[3:0] Q, input wire reset, input wire CLK, input wire rw, input wire[3:0] data);
    wire CLKandrw, q1, q2, q3, q4, notq1, notq2, notq3, notq4;

    and and1(CLKandrw, CLK, rw);
    Dtrigger Dtrigger1(q1, notq1, reset, CLKandrw, data[0]), Dtrigger2(q2, notq2, reset, CLKandrw, data[1]), Dtrigger3(q3, notq3, reset, CLKandrw, data[2]), Dtrigger4(q4, notq4, reset, CLKandrw, data[3]);
    
    wire[3:0] resMem, clk4;
    assign resMem[0] = q1, resMem[1] = q2, resMem[2] = q3, resMem[3] = q4;
    assign clk4[0] = CLK, clk4[1] = CLK, clk4[2] = CLK, clk4[3] = CLK;
    
    and4bit and2(Q, clk4, resMem);
endmodule

module decode2bit(
        output wire Q0, output wire Q1, output wire Q2, output wire Q3,
        input wire[1:0] A
    );
    wire not0, not1;
    not nota(not0, A[0]), notb(not1, A[1]);

    and and1(Q0, not1, not0), and2(Q1, not1, A[0]), and3(Q2, A[1], not0), and4(Q3, A[1], A[0]);
endmodule

module decode3bit(output wire Q0, output wire Q1, output wire Q2, output wire Q3, output wire Q4, input wire[2:0] A);
    wire not0, not1, not2;
    not nota(not0, A[0]), notb(not1, A[1]), notc(not2, A[2]);

    wire res1, res2, res3, res6, res7, res8;

    and and1(res1, not2, not1, not0), and2(res2, not2, not1, A[0]), and3(res3, not2, A[1], not0), and4(Q3, not2, A[1], A[0]), and5(Q4, A[2], not1, not0);
    and and6(res6, A[2], not1, A[0]), and7(res7, A[2], A[1], not0), and8(res8, A[2], A[1], A[0]);

    or or1(Q0, res1, res6), or2(Q1, res2, res7), or3(Q2, res3, res8);
endmodule

module encode3bit(
        output wire[2:0] Q,
        input wire A0, input wire A1, input wire A2, input wire A3, input wire A4
    );
    or or1(Q[0], A1, A3), or2(Q[1], A2, A3);
assign Q[2] = A4;
endmodule

module bitTo4(output wire[3:0] Q, input wire A);
    assign Q[0] = A, Q[1] = A, Q[2] = A, Q[3] = A;
endmodule

module plusOne(output wire[2:0] Q, input wire[2:0] A);
    wire res0, res1, res2, res3, res4;
    decode3bit dec(res0, res1, res2, res3, res4, A);
    encode3bit enc(Q, res4, res0, res1, res2, res3);
endmodule

module minusOne(output wire[2:0] Q, input wire CLK, input wire[2:0] A);
    wire nCLK;
    not not1(nCLK, CLK);
    
    wire[2:0] notCLK, CLK3bit;
    assign notCLK[0] = nCLK, notCLK[1] = nCLK, notCLK[2] = nCLK;
    assign CLK3bit[0] = CLK, CLK3bit[1] = CLK, CLK3bit[2] = CLK;

    wire[2:0] AandNotCLK;
    and3bit andAnotCLK(AandNotCLK, A, notCLK);

    wire res0, res1, res2, res3, res4;
    decode3bit dec(res0, res1, res2, res3, res4, A);
    wire[2:0] result;
    encode3bit enc(result, res1, res2, res3, res4, res0);

    wire[2:0] resCLK;
    and3bit resAndCLK(resCLK, result, CLK3bit);

    or3bit resOrMask(Q, resCLK, AandNotCLK);
endmodule

module and3bit(output wire[2:0] Q, input wire[2:0] A, input wire[2:0] B);
    and and1(Q[0], A[0], B[0]), and2(Q[1], A[1], B[1]), and3(Q[2], A[2], B[2]);
endmodule

module or3bit(output wire[2:0] Q, input wire[2:0] A, input wire[2:0] B);
    or or1(Q[0], A[0], B[0]), or2(Q[1], A[1], B[1]), or3(Q[2], A[2], B[2]);
endmodule

module ifThenAelseB(output wire[2:0] Q, input wire condition, input wire[2:0] A, input wire[2:0] B);
    wire notcondition;
    not not1(notcondition, condition);
    wire[2:0] cond, notcond;

    assign cond[0] = condition, cond[1] = condition, cond[2] = condition;
    assign notcond[0] = notcondition, notcond[1] = notcondition, notcond[2] = notcondition;

    wire[2:0] and1out, and2out;
    and3bit and1(and1out, A, cond), and2(and2out, B, notcond);
    or3bit or1(Q, and1out, and2out);
endmodule

module pointerT(output wire[2:0] Q, input wire reset, input wire CLK);
    wire andres, res, nq;
    not not1(nq, Q[1]);
    and and1(andres, Q[0], nq, Q[2]);
    or or1(res, reset, andres);
    Ttrigger t1(Q[0], res, CLK), t2(Q[1], res, Q[0]), t3(Q[2], res, Q[1]);
endmodule

module mod5forInd(output wire[2:0] Q, input wire[2:0] A);
    wire t0, t1, t2, t3, t4;
    decode3bit d(t0, t1, t2, t3, t4, A);
    encode3bit e(Q, t0, t1, t2, t3, t4);
endmodule

module StackMemory(output wire[3:0] Q, input wire CLK, input wire reset, input wire rw, input wire[3:0] data, input wire[2:0] index);
    wire clkrw;
    and and999(clkrw, CLK, rw);

    wire i0, i1, i2, i3, i4;
    decode3bit dec(i0, i1, i2, i3, i4, index);

    wire clki0, clki1, clki2, clki3, clki4;
    and and0(clki0, CLK, i0), and1(clki1, CLK, i1), and2(clki2, CLK, i2), and3(clki3, CLK, i3), and4(clki4, CLK, i4);

    wire[3:0] q0, q1, q2, q3, q4;
    Memory mem0(q0, reset, clki0, clkrw, data), mem1(q1, reset, clki1, clkrw, data), mem2(q2, reset, clki2, clkrw, data), mem3(q3, reset, clki3, clkrw, data), mem4(q4, reset, clki4, clkrw, data);

    wire[3:0] i4b0, i4b1, i4b2, i4b3, i4b4;
    bitTo4 to0(i4b0, i0), to1(i4b1, i1), to2(i4b2, i2), to3(i4b3, i3), to4(i4b4, i4);

    wire[3:0] res0, res1, res2, res3, res4;
    and4bit and4b0(res0, q0, i4b0), and4b1(res1, q1, i4b1), and4b2(res2, q2, i4b2), and4b3(res3, q3, i4b3), and4b4(res4, q4, i4b4);

    wire[3:0] Q1, Q2, Q3;
    or4bit or0(Q1, res0, res1), or1(Q2, Q1, res2), or2(Q3, Q2, res3), or3(Q, Q3, res4);
endmodule

module diffModulo5(output wire[2:0] Q, input wire[2:0] A, input wire[2:0] B);
    wire a0, a1, a2, a3, a4; 
    wire b0, b1, b2, b3, b4;
    decode3bit dec1(a0, a1, a2, a3, a4, A), dec2(b0, b1, b2, b3, b4, B);

    wire res0, res1, res2, res3, res4;
    and and0(res0, a0, b0), and1(res1, a1, b1), and2(res2, a2, b2), and3(res3, a3, b3), and4(res4, a4, b4);

    wire res5, res6, res7, res8, res9;
    and and5(res5, a0, b4), and6(res6, a1, b0), and7(res7, a2, b1), and8(res8, a3, b2), and9(res9, a4, b3);

    wire res10, res11, res12, res13, res14;
    and and10(res10, a0, b3), and11(res11, a1, b4), and12(res12, a2, b0), and13(res13, a3, b1), and14(res14, a4, b2);

    wire res15, res16, res17, res18, res19;
    and and15(res15, a0, b2), and16(res16, a1, b3), and17(res17, a2, b4), and18(res18, a3, b0), and19(res19, a4, b1);

    wire res20, res21, res22, res23, res24;
    and and20(res20, a0, b1), and21(res21, a1, b2), and22(res22, a2, b3), and23(res23, a3, b4), and24(res24, a4, b0);

    wire o0, o1, o2, o3, o4;
    or or0(o0, res0, res1, res2, res3, res4), or1(o1, res5, res6, res7, res8, res9), or2(o2, res10, res11, res12, res13, res14), or3(o3, res15, res16, res17, res18, res19), or4(o4, res20, res21, res22, res23, res24);

    encode3bit enc(Q, o0, o1, o2, o3, o4);
endmodule
