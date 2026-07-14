#include "DCTargetStreamer.h"
#include "llvm/MC/MCExpr.h"

using namespace llvm;

DCTargetStreamer::DCTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void DCTargetStreamer::emitValue(const MCExpr *Value) {
  int64_t Res;

  if (Value->evaluateAsAbsolute(Res)) {
    Streamer.emitRawText(Twine(Res));
  } else {
    MCTargetStreamer::emitValue(Value);
  }
}
