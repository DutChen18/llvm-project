def CC : CallingConv<[CCIfType<[i32], CCAssignToReg<[R4, R5]>>, CCAssignToStack<0, 0>]>;
def RetCC : CallingConv<[CCIfType<[i32], CCAssignToReg<[R4]>>]>;

def CSR : CalleeSavedRegs<(add R1, R6, R7)>;
