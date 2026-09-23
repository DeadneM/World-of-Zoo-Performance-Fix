/* V3 experimental additions. The V2 fixes remain separate and unchanged.
   No COM/vtable replacement, executable file edits, or simulation clock hooks. */
static const BYTE pair_lock_signature[]={
  0x56,0x8b,0xf1,0x0f,0xb7,0x56,0x18,0x0f,0xaf,0x56,0x10,
  0x8b,0x4e,0x04,0x8b,0x01,0x8b,0x40,0x20,0x57,0x6a,0x00,
  0x52,0x6a,0x00,0xff,0xd0,0x8b,0x7c,0x24,0x0c,0x89,0x07,
  0x8b,0x46,0x14,0x8b,0x4e,0x08,0x8b,0x11,0x8b,0x52,0x24,
  0x6a,0x00,0x03,0xc0,0x50,0x6a,0x00,0xff,0xd2,0x8b,0x4c,
  0x24,0x10,0x89,0x01,0x83,0x3f,0x00,0x74,0x0b,0x85,0xc0,
  0x74,0x07,0x5f,0xb0,0x01,0x5e,0xc2,0x08,0x00,0x8b,0x16,
  0x8b,0x42,0x10,0x8b,0xce,0xff,0xd0,0x5f,0x32,0xc0,0x5e,
  0xc2,0x08,0x00
};
static const BYTE limiter_signature[]={
  0x80,0xbe,0xa6,0x00,0x00,0x00,0x00,0x74,0x23,
  0x8d,0xa4,0x24,0x00,0x00,0x00,0x00,
  0x8b,0x17,0x8b,0x42,0x24,0x8b,0xcf,0xff,0xd0,0x2b,0xc5,
  0x3b,0x05,0xc0,0x10,0xaa,0x00,0x73,0x09,
  0x80,0xbe,0xa6,0x00,0x00,0x00,0x00,0x75,0xe4
};

static void put32(BYTE *p,uintptr_t n){uint32_t v=(uint32_t)n;memcpy(p,&v,4);}
static void branch32(BYTE *p,uintptr_t address,uintptr_t target){
  p[0]=0xe9;put32(p+1,target-address-5);
}
static BOOL seal_code(BYTE *code,size_t size){
  DWORD old;
  if(!VirtualProtect(code,size,PAGE_EXECUTE_READ,&old))return FALSE;
  return FlushInstructionCache(GetCurrentProcess(),code,size);
}
static BOOL patch_branch(BYTE *site,size_t size,BYTE *code){
  BYTE patch[16];DWORD old,unused;
  if(size<5||size>sizeof(patch))return FALSE;
  memset(patch,0x90,size);branch32(patch,(uintptr_t)site,(uintptr_t)code);
  if(!VirtualProtect(site,size,PAGE_EXECUTE_READWRITE,&old))return FALSE;
  memcpy(site,patch,size);FlushInstructionCache(GetCurrentProcess(),site,size);
  VirtualProtect(site,size,old,&unused);return TRUE;
}

/* At entry to 70FE80: [esp]=8330AB, [esp+12]=83352A only for the
   adapter's CPU triangle-query path. Guards also require readable,
   non-dynamic MANAGED VB and IB. All other cases run an exact clone of
   the native function. Both clones have only internal relative branches.
   The read clone differs at the two PUSH mode immediates (0 -> 3).
   Keeping native failure cleanup, sizes, output order and return ABI
   avoids duplicating the engine lock/unlock policy in C. */
static void make_readonly_stub(BYTE *out,uintptr_t address,
                               uintptr_t adapter_return,uintptr_t query_return){
  size_t n=0;
  const BYTE first[]={0x81,0x3c,0x24};
  memcpy(out+n,first,sizeof(first));n+=sizeof(first);put32(out+n,adapter_return);n+=4;
  #define FAIL_IF_NE() do{out[n++]=0x0f;out[n++]=0x85;put32(out+n,address+128-(address+n+4));n+=4;}while(0)
  FAIL_IF_NE();
  const BYTE outer[]={0x81,0x7c,0x24,0x0c};
  memcpy(out+n,outer,sizeof(outer));n+=sizeof(outer);put32(out+n,query_return);n+=4;
  FAIL_IF_NE();
  const BYTE vb[]={0x8b,0x41,0x04};memcpy(out+n,vb,3);n+=3;
  #define CHECK_FIELD(offset,value) do{out[n++]=0x80;out[n++]=0x78;out[n++]=(offset);out[n++]=(value);FAIL_IF_NE();}while(0)
  CHECK_FIELD(0x0c,0);CHECK_FIELD(0x0d,0);CHECK_FIELD(0x0e,1);
  const BYTE ib[]={0x8b,0x41,0x08};memcpy(out+n,ib,3);n+=3;
  CHECK_FIELD(0x10,0);CHECK_FIELD(0x11,0);CHECK_FIELD(0x12,1);
  #undef CHECK_FIELD
  #undef FAIL_IF_NE
  branch32(out+n,address+n,address+256);
  memcpy(out+128,pair_lock_signature,sizeof(pair_lock_signature));
  memcpy(out+256,pair_lock_signature,sizeof(pair_lock_signature));
  out[256+0x15]=3;out[256+0x2d]=3;
}
static BOOL install_readonly_fix(BYTE *site,uintptr_t adapter_return,uintptr_t query_return){
  if(memcmp(site,pair_lock_signature,sizeof(pair_lock_signature)))return FALSE;
  BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
  if(!code)return FALSE;
  make_readonly_stub(code,(uintptr_t)code,adapter_return,query_return);
  if(!seal_code(code,512)||!patch_branch(site,7,code)){
    VirtualFree(code,0,MEM_RELEASE);return FALSE;
  }
  return TRUE;
}

typedef struct PaceState {
  int64_t frequency,previous;
  unsigned fps,remainder;
  BOOL initialized;
} PaceState;
static int64_t pace_deadline(PaceState *p,int64_t now,BOOL enabled){
  if(!enabled){p->initialized=FALSE;p->remainder=0;return now;}
  if(!p->initialized){p->initialized=TRUE;p->previous=now;p->remainder=0;return now;}
  int64_t step=p->frequency/p->fps;
  p->remainder+=(unsigned)(p->frequency%p->fps);
  if(p->remainder>=p->fps){step++;p->remainder-=p->fps;}
  int64_t goal=p->previous+step;
  // A slow frame starts a fresh interval: no catch-up burst after a stall.
  if(now>=goal){p->previous=now;p->remainder=0;return now;}
  p->previous=goal;return goal;
}
static unsigned valid_target(unsigned n){return n==0||(n>=30&&n<=360)?n:60;}
static PaceState pacing;
static HANDLE pacing_timer;
static BOOL pacing_initialize(unsigned target){
  LARGE_INTEGER f;
  memset(&pacing,0,sizeof(pacing));
  if(!target||!QueryPerformanceFrequency(&f)||f.QuadPart<=0||f.QuadPart>10000000000LL)return FALSE;
  pacing.frequency=f.QuadPart;pacing.fps=target;
  // An unnamed, process-local timer. No global timer-resolution change.
  pacing_timer=CreateWaitableTimerExW(NULL,NULL,0x00000002,TIMER_MODIFY_STATE|SYNCHRONIZE);
  return TRUE;
}
static void __cdecl pace_frame(const BYTE *game){
  LARGE_INTEGER current;
  if(!QueryPerformanceCounter(&current))return;
  int64_t now=current.QuadPart;
  int64_t goal=pace_deadline(&pacing,now,game[0xa6]!=0&&game[0xa4]==0);
  const int64_t guard=pacing.frequency/2000; // final 0.5 ms in QPC/pause
  while(now<goal){
    int64_t remaining=goal-now;
    if(pacing_timer&&remaining>guard){
      LARGE_INTEGER due;
      due.QuadPart=-((remaining-guard)*10000000LL/pacing.frequency);
      if(due.QuadPart==0)due.QuadPart=-1;
      if(SetWaitableTimer(pacing_timer,&due,0,NULL,NULL,FALSE)){
        DWORD result=WaitForSingleObject(pacing_timer,1000);
        if(result!=WAIT_OBJECT_0){CloseHandle(pacing_timer);pacing_timer=NULL;}
      }else{CloseHandle(pacing_timer);pacing_timer=NULL;}
    }else if(!pacing_timer&&remaining>pacing.frequency/1000){
      // Fallback keeps the old game's active-wait behavior, yielding to
      // runnable threads, instead of relying on a coarse Sleep(1).
      Sleep(0);
    }else{
      YieldProcessor();
    }
    if(!QueryPerformanceCounter(&current)){pacing.initialized=FALSE;return;}
    now=current.QuadPart;
  }
  if(now-goal>pacing.frequency/pacing.fps/2){pacing.previous=now;pacing.remainder=0;}
}

/* Detour replaces the outer cap check, then resumes at TEST BL,BL.
   Preserve all integer registers, flags, x87/MMX/SSE state and stack.
   The 512-byte FXSAVE block and the callback call site are 16-byte aligned.
   ESI is the original main-loop object; no new ABI assumptions about EAX.
   callback must obey the x86 C ABI (including preserving EBX). */
static size_t make_pacer_stub(BYTE *out,uintptr_t address,uintptr_t callback,uintptr_t resume){
  const BYTE prefix[]={
    0x9c,0x60,0x8b,0xdc,                    // pushfd; pushad; mov ebx,esp
    0x81,0xec,0x10,0x02,0x00,0x00,          // sub esp,528
    0x83,0xe4,0xf0,                         // and esp,-16
    0x0f,0xae,0x04,0x24,0xdb,0xe3,          // fxsave [esp]; fninit
    0x83,0xec,0x0c,0x56,0xe8,0,0,0,0,     // align, push esi, call
    0x83,0xc4,0x10,0x0f,0xae,0x0c,0x24,   // add esp,16; fxrstor [esp]
    0x8b,0xe3,0x61,0x9d,0xe9,0,0,0,0      // restore; jump
  };
  memcpy(out,prefix,sizeof(prefix));
  put32(out+24,callback-(address+28));
  put32(out+40,resume-(address+44));
  return sizeof(prefix);
}
static BOOL limiter_matches(const BYTE *base,const BYTE *site){
  BYTE expected[sizeof(limiter_signature)];memcpy(expected,limiter_signature,sizeof(expected));
  put32(expected+29,(uintptr_t)(base+0x6a10c0)); // relocated absolute data operand
  return !memcmp(site,expected,sizeof(expected));
}
#if !defined(WOZ_TIMING_V4) || defined(WOZ_TEST)
static BOOL install_pacer(BYTE *base,BYTE *site,uintptr_t callback){
  if(!limiter_matches(base,site))return FALSE;
  BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
  if(!code)return FALSE;
  size_t n=make_pacer_stub(code,(uintptr_t)code,callback,(uintptr_t)(site+sizeof(limiter_signature)));
  if(!seal_code(code,n)||!patch_branch(site,7,code)){
    VirtualFree(code,0,MEM_RELEASE);return FALSE;
  }
  return TRUE;
}
#endif
