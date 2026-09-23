#ifdef NDEBUG
#undef NDEBUG
#endif
#define WOZ_TEST
#include "../src/woz_fix.c"
#include <assert.h>
#include <math.h>
#define THISCALL __attribute__((thiscall))
static const char *analysis_exe;
static BYTE *allocate(void){BYTE *p=VirtualAlloc(NULL,8192,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(p);return p;}
static void read_original(unsigned rva,void *out,size_t length){
  FILE *f=fopen(analysis_exe,"rb");assert(f);
  assert(!fseek(f,(long)rva,SEEK_SET));assert(fread(out,1,length,f)==length);fclose(f);
}
static void *vb_object;
static unsigned modes[2],sizes[2],lock_calls,cleanup_calls;
static int fail_buffer=-1;
static void *THISCALL lock_mock(void *self,unsigned offset,unsigned size,unsigned mode){
  unsigned i=self==vb_object?0:1;assert(offset==0);modes[i]=mode;sizes[i]=size;lock_calls++;
  return (int)i==fail_buffer?NULL:(void*)(uintptr_t)(0x11110000+i);
}
static void THISCALL cleanup_mock(void *self){assert(self);cleanup_calls++;}
typedef BYTE (THISCALL *Adapter)(void*,void**,void**,void**,void**);
typedef BYTE (THISCALL *Pair)(void*,void**,void**);
static BYTE *make_caller(BYTE *p,BYTE *adapter){
  // Four stack arguments copied without changing ECX; callee pops them.
  const BYTE push[]={0xff,0x74,0x24,0x10};
  for(unsigned i=0;i<4;i++)memcpy(p+4*i,push,4);
  p[16]=0xb8;put32(p+17,(uintptr_t)adapter);p[21]=0xff;p[22]=0xd0;
  p[23]=0xc2;p[24]=0x10;p[25]=0;return p+23;
}
static void test_readonly(void){
  BYTE *code=allocate();read_original(0x30fe80,code,sizeof(pair_lock_signature));
  assert(!memcmp(code,pair_lock_signature,sizeof(pair_lock_signature)));
  read_original(0x433080,code+512,46);memcpy(code+768,code+512,46);
  BYTE *query_ret=make_caller(code+1024,code+512);
  make_caller(code+1152,code+512);make_caller(code+1280,code+768);
  BYTE vb[32]={0},ib[32]={0},mesh[32]={0},adapter[160]={0};
  void *vb_table[16]={0},*ib_table[16]={0},*pair_table[8]={0};
  vb_table[8]=(void*)lock_mock;ib_table[9]=(void*)lock_mock;
  pair_table[3]=code;pair_table[4]=(void*)cleanup_mock;
  *(void***)vb=vb_table;*(void***)ib=ib_table;*(void***)mesh=pair_table;
  *(void**)(mesh+4)=vb;*(void**)(mesh+8)=ib;
  *(unsigned*)(mesh+0x10)=100;*(unsigned*)(mesh+0x14)=180;*(WORD*)(mesh+0x18)=32;
  *(void**)(adapter+0x88)=mesh;vb[0x0e]=ib[0x12]=1;vb_object=vb;
  Adapter query=(Adapter)(code+1024),update=(Adapter)(code+1152),other_adapter=(Adapter)(code+1280);
  void *a,*b,*c,*d;
  assert(query(adapter,&a,&b,&c,&d));assert(modes[0]==0&&modes[1]==0);
  BYTE bad[sizeof(pair_lock_signature)];memcpy(bad,code,sizeof(bad));bad[1]^=1;
  assert(!install_readonly_fix(bad,(uintptr_t)(code+512+43),(uintptr_t)query_ret));
  assert(install_readonly_fix(code,(uintptr_t)(code+512+43),(uintptr_t)query_ret));
  assert(query(adapter,&a,&b,&c,&d));assert(modes[0]==3&&modes[1]==3);
  assert(a==(void*)0x11110001&&c==(void*)0x11110000&&!b&&!d);
  assert(sizes[0]==3200&&sizes[1]==360);
  assert(update(adapter,&a,&b,&c,&d));assert(modes[0]==0&&modes[1]==0);
  // Outer caller differs for this wrapper; inner return differs too.
  assert(other_adapter(adapter,&a,&b,&c,&d));assert(modes[0]==0&&modes[1]==0);
  assert(((Pair)code)(mesh,&c,&a));assert(modes[0]==0&&modes[1]==0);
  BYTE *guards[]={vb+12,vb+13,vb+14,ib+16,ib+17,ib+18};
  for(unsigned i=0;i<6;i++){
    *guards[i]^=1;assert(query(adapter,&a,&b,&c,&d));assert(modes[0]==0&&modes[1]==0);*guards[i]^=1;
  }
  for(int i=0;i<2;i++){
    fail_buffer=i;unsigned n=cleanup_calls;
    assert(!query(adapter,&a,&b,&c,&d));assert(cleanup_calls==n+1);
    assert(modes[0]==3&&modes[1]==3);
  }
  fail_buffer=-1;
  for(unsigned i=0;i<10000;i++){assert(query(adapter,&a,&b,&c,&d));assert(modes[0]==3&&modes[1]==3);}
  assert(!memcmp(code+7,pair_lock_signature+7,sizeof(pair_lock_signature)-7));
  puts("PASS: original paired-buffer lock + adapter executed; guarded READONLY, writable fallback, sizes, outputs, failure cleanup, 10000 calls.");
}

static void test_deadlines(void){
  assert(valid_target(0)==0&&valid_target(29)==60&&valid_target(30)==30&&valid_target(360)==360&&valid_target(361)==60&&valid_target(UINT_MAX)==60);
  const unsigned rates[]={30,60,90,120,144,165,240,360};
  for(unsigned r=0;r<sizeof(rates)/sizeof(rates[0]);r++){
    PaceState p={0};p.frequency=10000003;p.fps=rates[r];
    int64_t start=1000000000,now=start;
    assert(pace_deadline(&p,now,TRUE)==now);
    for(unsigned i=0;i<p.fps*10;i++){
      int64_t goal=pace_deadline(&p,now+10,TRUE);assert(goal>now);now=goal;
    }
    assert(now-start==p.frequency*10);
    int64_t late=now+p.frequency*3;
    assert(pace_deadline(&p,late,TRUE)==late);
    int64_t next=pace_deadline(&p,late+10,TRUE);assert(next>late+p.frequency/p.fps-2);
    assert(pace_deadline(&p,next+100,FALSE)==next+100&&!p.initialized);
    assert(pace_deadline(&p,next+100000,TRUE)==next+100000);
  }
  puts("PASS: rational pacing at 30-360 FPS, exact 10-second schedules, slow-frame reset, disable/re-enable, INI bounds.");
}

static unsigned mock_time;
static unsigned THISCALL time_mock(void *obj){(void)obj;return mock_time;}
static void test_native_clock(void){
  BYTE *code=allocate();read_original(0xefe90,code,0x82);
  struct {uint32_t old;BYTE *replacement;unsigned count;} refs[]={
    {0xa18a20,code+512,0},{0xa1c588,code+528,0},{0xa13900,code+544,0},{0xa17600,code+560,0}
  };
  *(float*)(code+512)=4294967296.0f;*(double*)(code+528)=0.0010000000474974513;
  *(float*)(code+544)=0.10000000149011612f;*(double*)(code+560)=0.10000000149011612;
  for(unsigned i=0;i<0x82-3;i++)for(unsigned j=0;j<4;j++){
    if(!memcmp(code+i,&refs[j].old,4)){put32(code+i,(uintptr_t)refs[j].replacement);refs[j].count++;}
  }
  for(unsigned j=0;j<4;j++)assert(refs[j].count==1);
  void *table[2]={NULL,(void*)time_mock};void **clock=table;
  typedef void (THISCALL *Update)(void*);
  const unsigned rates[]={30,60,120,144,240,360};
  for(unsigned r=0;r<sizeof(rates)/sizeof(rates[0]);r++){
    BYTE state[32]={0};*(void**)(state+4)=&clock;*(float*)(state+24)=1.0f;
    double total=0;
    for(unsigned i=1;i<=rates[r];i++){
      mock_time=i*1000/rates[r];((Update)code)(state);total+=*(float*)(state+8);
    }
    assert(fabs(total-1.0)<0.00001);assert(*(unsigned*)(state+12)==1000);
    mock_time+=500;((Update)code)(state);assert(fabs(*(float*)(state+8)-0.1)<0.00001);
  }
  puts("PASS: original simulation-clock code accumulates one second at 30/60/120/144/240/360 updates; native 100 ms clamp preserved.");
}

static unsigned pacer_calls;
static const BYTE *pacer_object;
static void __cdecl clobber_callback(const BYTE *object){
  pacer_calls++;pacer_object=object;
  __asm__ __volatile__("fninit\n\tfldz\n\tpxor %%xmm0,%%xmm0\n\tpxor %%xmm1,%%xmm1\n\tpxor %%xmm2,%%xmm2\n\tpxor %%xmm3,%%xmm3\n\tpxor %%xmm4,%%xmm4\n\tpxor %%xmm5,%%xmm5\n\tpxor %%xmm6,%%xmm6\n\tpxor %%xmm7,%%xmm7\n\txor %%eax,%%eax\n\txor %%ecx,%%ecx\n\txor %%edx,%%edx"
    :::"eax","ecx","edx","st","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","memory","cc");
}
static BYTE *emit_abs(BYTE *p,const BYTE *op,size_t count,uintptr_t address){memcpy(p,op,count);put32(p+count,address);return p+count+4;}
static BYTE *emit_capture(BYTE *p,BYTE *state){
  *p++=0x9c;const BYTE popflags[]={0x8f,0x05};p=emit_abs(p,popflags,2,(uintptr_t)(state+32));
  for(unsigned i=0;i<8;i++){BYTE op[]={0x89,(BYTE)(0x05+i*8)};p=emit_abs(p,op,2,(uintptr_t)(state+4*i));}
  return p;
}
/* Test-only installation of the pacing stub in a synthetic code buffer.
   The DLL always installs pacing and physics together. */
static BOOL install_test_pacer(BYTE *base,BYTE *site,uintptr_t callback){
  if(!limiter_matches(base,site))return FALSE;
  BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
  if(!code)return FALSE;
  size_t n=make_pacer_stub(code,(uintptr_t)code,callback,(uintptr_t)(site+sizeof(limiter_signature)));
  if(!seal_code(code,n)||!patch_branch(site,7,code)){
    VirtualFree(code,0,MEM_RELEASE);return FALSE;
  }
  return TRUE;
}
static void test_pacer_abi(void){
  BYTE *code=allocate(),*p=code,*original_fx=code+2048,*before_fx=code+2560,*after_fx=code+3072;
  BYTE *before=code+3600,*after=code+3664,*game=code+3800,*site=code+512;
  read_original(0xda40,site,sizeof(limiter_signature));
  assert(!memcmp(site,limiter_signature,sizeof(limiter_signature)));
  // Rebase the one absolute operand in the copied native limiter.
  put32(site+29,(uintptr_t)(code+0x6a10c0));site[44]=0xc3;
  BYTE bad[sizeof(limiter_signature)];memcpy(bad,site,sizeof(bad));bad[0]^=1;
  assert(!install_test_pacer(code,bad,(uintptr_t)clobber_callback));
  assert(install_test_pacer(code,site,(uintptr_t)clobber_callback));
  *p++=0x9c;*p++=0x60;
  const BYTE save[]={0x0f,0xae,0x05},restore[]={0x0f,0xae,0x0d};
  p=emit_abs(p,save,3,(uintptr_t)original_fx);
  *p++=0xdb;*p++=0xe3;*p++=0xd9;*p++=0xe8;*p++=0xd9;*p++=0xeb;
  for(unsigned i=0;i<8;i++){
    memset(code+4096+i*16,(int)(0x10+i),16);
    BYTE load[]={0x66,0x0f,0x6f,(BYTE)(0x05+i*8)};
    p=emit_abs(p,load,4,(uintptr_t)(code+4096+i*16));
  }
  for(unsigned i=0;i<8;i++)if(i!=4){*p++=(BYTE)(0xb8+i);put32(p,i==6?(uintptr_t)game:0x11220000+i);p+=4;}
  *p++=0x68;put32(p,0x247);p+=4;*p++=0x9d;
  p=emit_capture(p,before);p=emit_abs(p,save,3,(uintptr_t)before_fx);
  *p=0xe8;put32(p+1,(uintptr_t)site-(uintptr_t)(p+5));p+=5;
  p=emit_capture(p,after);p=emit_abs(p,save,3,(uintptr_t)after_fx);
  p=emit_abs(p,restore,3,(uintptr_t)original_fx);
  *p++=0x61;*p++=0x9d;*p++=0xc3;assert(p<code+512);
  FlushInstructionCache(GetCurrentProcess(),code,8192);
  for(unsigned iteration=0;iteration<10000;iteration++){
    ((void(*)(void))code)();assert(pacer_object==game);assert(!memcmp(before,after,36));
    assert(!memcmp(before_fx,after_fx,24));assert(!memcmp(before_fx+24,after_fx+24,8));
    for(unsigned i=0;i<8;i++)assert(!memcmp(before_fx+32+i*16,after_fx+32+i*16,10));
    assert(!memcmp(before_fx+160,after_fx+160,128));
  }
  assert(pacer_calls==10000);
  puts("PASS: actual x86 limiter detour preserves 8 GPRs/ESP, EFLAGS, x87/MMX/SSE and callback argument; 10000 executions.");
}
static void test_real_pacing(void){
  BYTE game[192]={0};game[0xa6]=1;
  const unsigned rates[]={60,120,144,240};
  for(unsigned r=0;r<sizeof(rates)/sizeof(rates[0]);r++){
    unsigned fps=rates[r],frames=fps/2;assert(pacing_initialize(fps));
    LARGE_INTEGER start,end;pace_frame(game);QueryPerformanceCounter(&start);
    for(unsigned i=0;i<frames;i++)pace_frame(game);
    QueryPerformanceCounter(&end);
    double achieved=(double)frames*pacing.frequency/(end.QuadPart-start.QuadPart);
    printf("PACE: requested %u FPS, measured %.2f in CPU-only timer test; high-res timer %s.\n",fps,achieved,pacing_timer?"available":"unavailable");
    assert(achieved<fps*1.03); // no too-fast cap; a loaded host may run slower
    if(pacing_timer){CloseHandle(pacing_timer);pacing_timer=NULL;}
  }
}
int main(int argc,char **argv){
  if(argc!=2){fprintf(stderr,"Usage: %s <analysis-exe>\n",argv[0]);return 2;}
  analysis_exe=argv[1];
  test_deadlines();test_native_clock();test_readonly();test_pacer_abi();test_real_pacing();return 0;
}
