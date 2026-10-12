#include "stream.h"
#include "sam.h"
#include "reciter.h"
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
extern int singmode;
int debug = 0;
extern unsigned char A, X, Y;
void SamResetTiming(void);
static jmp_buf cancelled;
static SamSink output;
static SamPitch pitch_source;
static void *user;
static uint8_t pending[8];
static int emitted, end;
static void Emit(void) {
    if (!output(user, pending[emitted & 7])) longjmp(cancelled, 1);
    pending[emitted & 7] = 128;
    ++emitted;
}
void SamWrite(int position, uint8_t value) {
    /* SAM writes five values ahead, sometimes overwriting the preceding tail.
       Keep that tail mutable; flush only positions it cannot revisit. */
    while (position - emitted >= 8) Emit();
    pending[position & 7] = value;
    if (position + 1 > end) end = position + 1;
}
uint8_t SamPeriod(uint8_t period) {
    unsigned ratio = pitch_source ? pitch_source(user) : 256;
    if (!ratio) ratio = 1;
    unsigned result = ((unsigned)period * 256 + ratio / 2) / ratio;
    return result < 1 ? 1 : result > 255 ? 255 : result;
}
int SamSpeak(const char *text, uint8_t speed, uint8_t mouth, uint8_t throat,
             int sing, SamSink sink, SamPitch pitch, void *context) {
    unsigned char input[256] = {0};
    size_t length = strlen(text);
    if (!sink || !length || length > 63 || !speed || !mouth || !throat) return 0;
    for (size_t i = 0; i < length; ++i) {
        if ((unsigned char)text[i] < 32 || (unsigned char)text[i] > 126) return 0;
        input[i] = (unsigned char)toupper((unsigned char)text[i]);
    }
    input[length] = '['; // Reciter's explicit end-of-text token.
    output = sink; pitch_source = pitch; user = context;
    emitted = end = 0;
    memset(pending, 128, sizeof pending);
    A = X = Y = 0;
    SamResetTiming();
    SetSpeed(speed); SetPitch(64); SetMouth(mouth); SetThroat(throat);
    singmode = sing != 0; // Reset between phrases, including non-sing phrases.
    if (setjmp(cancelled)) return -1;
    if (!TextToPhonemes(input)) return 0;
    SetInput((char *)input);
    if (!SAMMain()) return 0;
    while (emitted < end) Emit();
    return 1;
}
