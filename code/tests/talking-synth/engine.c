#include "stream.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct Result { uint64_t hash; size_t count, cancel; uint16_t pitch; int varied; uint8_t first; };
static int sink(void *ctx,uint8_t value) {
    struct Result *r=ctx;
    if (r->cancel && r->count==r->cancel) return 0;
    if (!r->count) r->first=value;
    if (value!=r->first) r->varied=1;
    r->hash=(r->hash^value)*1099511628211ULL; r->count++;
    return 1;
}
static uint16_t pitch(void *ctx) { return ((struct Result *)ctx)->pitch; }
static struct Result speak(const char *text,int speed,int sing,int ratio,size_t cancel,int expected) {
    struct Result r={.hash=1469598103934665603ULL,.pitch=ratio,.cancel=cancel};
    int result=SamSpeak(text,speed,128,128,sing,sink,pitch,&r);
    assert(result==expected);
    if (result==1) assert(r.count>100 && r.varied);
    return r;
}
int main(void) {
    struct Result a=speak("HELLO KASTLE",72,0,256,0,1);
    struct Result b=speak("HELLO KASTLE",72,0,256,0,1);
    assert(a.hash==b.hash && a.count==b.count);
    struct Result high=speak("HELLO KASTLE",72,0,512,0,1);
    assert(a.hash!=high.hash);
    double duration=(double)high.count/a.count;
    assert(duration>0.85 && duration<1.15); // Pitch must not double playback speed.
    struct Result slow=speak("HELLO KASTLE",120,0,256,0,1);
    assert(slow.count>a.count*1.3);
    speak("INTERRUPT THIS PHRASE",72,0,256,32,-1);
    b=speak("HELLO KASTLE",72,0,256,0,1);
    assert(a.hash==b.hash);
    speak("SING THIS",72,1,256,0,1);
    b=speak("HELLO KASTLE",72,0,256,0,1); assert(a.hash==b.hash);
    speak("",72,0,256,0,0);
    speak("HELLO",0,0,256,0,0);
    speak("THIS IS A LONGER PHRASE WITH NUMBERS 123 AND PUNCTUATION?",72,0,256,0,1);
    const char *corpus[]={"A", "I", "WHY?", "ONE TWO THREE", "1234567890", "HELLO, WORLD!", "IT'S A SYNTH", "THIS-IS-SAM", "ZZZZ", "AAAA", "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG."};
    for (size_t i=0;i<sizeof(corpus)/sizeof(corpus[0]);++i) speak(corpus[i],72,0,256,200000,1);
    const char *words[]={"HELLO", "ROBOT", "SAM", "SYNTH", "BLUE", "RED", "ONE", "THREE", "KASTLE"};
    uint32_t rng=12345;
    for(int trial=0;trial<64;trial++) {
        char text[64]={0};size_t length=0;
        while(length<45) {
            rng=rng*1664525u+1013904223u;
            const char *word=words[(rng>>16)%9];size_t n=strlen(word);
            if(length+n+1>63)break;
            memcpy(text+length,word,n);length+=n;text[length++]=' ';
        }
        struct Result r={.hash=1469598103934665603ULL,.pitch=(trial%3==0?64:trial%3==1?256:1024),.cancel=300000};
        int result=SamSpeak(text,trial%2?72:120,trial%2?128:190,trial%2?128:110,trial%2,sink,pitch,&r);
        assert(result==1 || result==-1);assert(r.count>0);
    }
    printf("Engine tests passed; normal %zu, high pitch %zu, slower %zu samples\n",a.count,high.count,slow.count);
    return 0;
}
