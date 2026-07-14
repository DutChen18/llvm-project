def Feature32Bit : SubtargetFeature<"32bit", "IsDC32", "true", "Implements DC32">;
def Feature64Bit : SubtargetFeature<"64bit", "IsDC64", "true", "Implements DC64">;

def IsDC32 : Predicate<"Subtarget->is32Bit()">, AssemblerPredicate<(all_of Feature32Bit), "DC32">;
def IsDC64 : Predicate<"Subtarget->is64Bit()">, AssemblerPredicate<(all_of Feature64Bit), "DC64">;

defvar DC32 = DefaultMode;
def DC64 : HwMode<[IsDC64]>;
