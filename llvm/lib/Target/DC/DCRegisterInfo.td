class DCReg<bits<6> encoding, string name> : Register<name> {
  let Namespace = "DC";
  let HWEncoding{5-0} = encoding;
}

def R0 : DCReg<0, "r0">;
def R1 : DCReg<1, "r1">;
def R2 : DCReg<2, "r2">;
def R3 : DCReg<3, "r3">;
def R4 : DCReg<4, "r4">;
def R5 : DCReg<5, "r5">;
def R6 : DCReg<6, "r6">;
def R7 : DCReg<7, "r7">;
def R8 : DCReg<8, "r8">;
def R9 : DCReg<9, "r9">;
def R10 : DCReg<10, "r10">;
def R11 : DCReg<11, "r11">;
def R12 : DCReg<12, "r12">;
def R13 : DCReg<13, "r13">;
def R14 : DCReg<14, "r14">;
def R15 : DCReg<15, "r15">;
def R16 : DCReg<16, "r16">;
def R17 : DCReg<17, "r17">;
def R18 : DCReg<18, "r18">;
def R19 : DCReg<19, "r19">;
def R20 : DCReg<20, "r20">;
def R21 : DCReg<21, "r21">;
def R22 : DCReg<22, "r22">;
def R23 : DCReg<23, "r23">;
def R24 : DCReg<24, "r24">;
def R25 : DCReg<25, "r25">;
def R26 : DCReg<26, "r26">;
def R27 : DCReg<27, "r27">;
def R28 : DCReg<28, "r28">;
def R29 : DCReg<29, "r29">;
def R30 : DCReg<30, "r30">;
def R31 : DCReg<31, "r31">;
def R32 : DCReg<32, "r32">;
def R33 : DCReg<33, "r33">;
def R34 : DCReg<34, "r34">;
def R35 : DCReg<35, "r35">;
def R36 : DCReg<36, "r36">;
def R37 : DCReg<37, "r37">;
def R38 : DCReg<38, "r38">;
def R39 : DCReg<39, "r39">;
def R40 : DCReg<40, "r40">;
def R41 : DCReg<41, "r41">;
def R42 : DCReg<42, "r42">;
def R43 : DCReg<43, "r43">;
def R44 : DCReg<44, "r44">;
def R45 : DCReg<45, "r45">;
def R46 : DCReg<46, "r46">;
def R47 : DCReg<47, "r47">;
def R48 : DCReg<48, "r48">;
def R49 : DCReg<49, "r49">;
def R50 : DCReg<50, "r50">;
def R51 : DCReg<51, "r51">;
def R52 : DCReg<52, "r52">;
def R53 : DCReg<53, "r53">;
def R54 : DCReg<54, "r54">;
def R55 : DCReg<55, "r55">;
def R56 : DCReg<56, "r56">;
def R57 : DCReg<57, "r57">;
def R58 : DCReg<58, "r58">;
def R59 : DCReg<59, "r59">;
def R60 : DCReg<60, "r60">;
def R61 : DCReg<61, "r61">;
def R62 : DCReg<62, "r62">;
def R63 : DCReg<63, "r63">;

def RLenVT : ValueTypeByHwMode<[DC32, DC64], [i32, i64]>;
def RLenRI : RegInfoByHwMode<[DC32, DC64], [RegInfo<32, 32, 32>, RegInfo<64, 64, 64>]>;

def GP : RegisterClass<"DC", [RLenVT], 32, (sequence "R%u", 0, 63)> {
  let RegInfos = RLenRI;
}
