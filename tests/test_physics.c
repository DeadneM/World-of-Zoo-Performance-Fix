#ifdef NDEBUG
#undef NDEBUG
#endif
#define WOZ_TEST
#include "../src/woz_fix.c"
#include <assert.h>
#include <math.h>
#define THISCALL __attribute__((thiscall))
static const char *analysis_exe;
static void load_code(unsigned rva,BYTE *out,size_t n){
  FILE *f=fopen(analysis_exe,"rb");assert(f);assert(!fseek(f,(long)rva,SEEK_SET));assert(fread(out,1,n,f)==n);fclose(f);
}
static void relative_call(BYTE *site,uintptr_t target){assert(site[0]==0xe8);put32(site+1,target-(uintptr_t)(site+5));}
static unsigned wall_ms,events,world_calls;
static double world_seconds;
static float world_last;
static unsigned THISCALL wall_clock(void *object){(void)object;return wall_ms;}
static void *THISCALL event_create(void *event,unsigned type){*(unsigned*)((BYTE*)event+8)=type;return event;}
static void THISCALL event_destroy(void *event){(void)event;}
static void THISCALL event_send(void *bus,void *event){(void)bus;assert(*(unsigned*)((BYTE*)event+8)==123);events++;}
static void THISCALL world_step(void *world,float dt){assert(world==(void*)0x12345678);world_calls++;world_last=dt;world_seconds+=dt;}
typedef void (THISCALL *Update)(void*);
static BYTE *code,*clock_code,*frame_code,*step_code;
static BYTE timer[32],accumulator[32],physics[64];
static void *clock_table[3],*accum_table[9],*physics_table[6],*time_table[2],*bus_table[3];
static void **clock_source,**bus;
static void reset(void){
  memset(timer,0,sizeof(timer));memset(accumulator,0,sizeof(accumulator));memset(physics,0,sizeof(physics));
  *(void***)timer=clock_table;*(void**)(timer+4)=&clock_source;*(float*)(timer+24)=1.0f;
  *(void***)accumulator=accum_table;*(float*)(accumulator+8)=1.0f/30.0f;
  *(void***)physics=physics_table;*(void**)(physics+12)=accumulator;
  *(void**)(physics+24)=timer;*(void**)(physics+28)=(void*)0x12345678;
  *(float*)(physics+32)=1.0f/30.0f;*(void**)(physics+40)=&bus;
  wall_ms=events=world_calls=0;world_seconds=0;world_last=0;
}
static void setup(void){
  code=VirtualAlloc(NULL,8192,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);assert(code);
  frame_code=code;step_code=code+256;clock_code=code+512;
  load_code(0x3df7f0,frame_code,sizeof(physics_update_signature));
  load_code(0x3df840,step_code,sizeof(physics_step_signature));
  assert(!memcmp(frame_code,physics_update_signature,sizeof(physics_update_signature)));
  assert(!memcmp(step_code,physics_step_signature,sizeof(physics_step_signature)));
  *(float*)(code+2048)=0;put32(frame_code+48,(uintptr_t)(code+2048));
  *(unsigned*)(code+2052)=123;put32(step_code+8,(uintptr_t)(code+2052));
  relative_call(step_code+0x17,(uintptr_t)event_create);
  relative_call(step_code+0x29,(uintptr_t)event_destroy);
  relative_call(step_code+0x38,(uintptr_t)world_step);
  load_code(0xefe90,clock_code,0x82);
  uint32_t old[]={0xa18a20,0xa1c588,0xa13900,0xa17600};
  *(float*)(code+2064)=4294967296.0f;*(double*)(code+2080)=0.0010000000474974513;
  *(float*)(code+2096)=0.10000000149011612f;*(double*)(code+2112)=0.10000000149011612;
  for(unsigned j=0;j<4;j++){
    unsigned matches=0;
    for(unsigned i=0;i<0x82-3;i++)if(!memcmp(clock_code+i,&old[j],4)){put32(clock_code+i,(uintptr_t)(code+2064+16*j));matches++;}
    assert(matches==1);
  }
  load_code(0x5dc910,code+768,0x17);load_code(0x5dc990,code+800,4);
  load_code(0x031120,code+832,4);load_code(0x3df950,code+848,4);
  clock_table[2]=code+832;accum_table[1]=code+768;accum_table[8]=code+800;
  physics_table[5]=step_code;time_table[1]=(void*)wall_clock;bus_table[2]=(void*)event_send;
  clock_source=time_table;bus=bus_table;reset();
}
static void tick(unsigned next_ms){wall_ms=next_ms;((Update)clock_code)(timer);((Update)frame_code)(physics);}
static double run(unsigned fps,unsigned seconds){
  reset();for(unsigned i=1;i<=fps*seconds;i++)tick(i*1000/fps);
  assert(world_calls==fps*seconds&&events==world_calls);return world_seconds;
}
int main(int argc,char **argv){
  if(argc!=2){fprintf(stderr,"Usage: %s <analysis-exe>\n",argv[0]);return 2;}
  analysis_exe=argv[1];setup();
  double old30=run(30,10),old60=run(60,10);
  printf("REPRODUCED: 10 wall-clock seconds -> native world %.6f s at 30 FPS; %.6f s at 60 FPS.\n",old30,old60);
  assert(fabs(old30-10.0)<0.0001&&fabs(old60-20.0)<0.0001);
  BYTE *hook=prepare_physics_stub(frame_code+0x17);assert(hook);
  assert(patch_branch(frame_code+0x17,7,hook));
  for(unsigned fps=20;fps<=60;fps+=10){
    double measured=run(fps,10);assert(fabs(measured-10.0)<0.0001);
    printf("CORRECTED: 10 wall-clock seconds -> world %.6f s at %u FPS.\n",measured,fps);
    float (THISCALL *get_step)(void*)=(void*)(code+848);
    assert(get_step(physics)==world_last);
  }
  reset();
  unsigned now=0;const unsigned delays[]={16,17,16,18,34,12,21,16,17,33};
  for(unsigned i=0;i<10000;i++){
    unsigned d=delays[i%10];now+=d;tick(now);
    assert(fabs(world_last-d*0.001)<0.000001);
  }
  assert(fabs(world_seconds-now*0.001)<0.001);
  double before=world_seconds;unsigned calls=world_calls;
  tick(now);assert(world_calls==calls&&world_seconds==before&&*(float*)(physics+32)==0);
  *(float*)(timer+24)=0;tick(now+17);assert(world_calls==calls);
  *(float*)(timer+24)=1;tick(now+267);assert(world_calls==calls+1&&fabs(world_last-0.1)<0.000001);
  puts("PASS: 10000 variable frames, native clock scaling/pause/100 ms clamp, event dispatch and timestep getter preserved.");
  return 0;
}
