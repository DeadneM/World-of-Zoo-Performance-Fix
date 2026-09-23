/* The original frame update stores the measured frame delta in its
   accumulator, but always calls the world step with the constructor's
   1/30-second value at controller+0x20. At 60 updates per wall-clock
   second this advances the physics world by about two seconds.

   Copy the same measured delta to +0x20 before the original accumulator
   call. The original step, timestep getters, pause check, event dispatch
   and world integration then use the actual duration of this frame.
   The real-time clock and its existing 100 ms clamp are unchanged. */
static const BYTE physics_update_signature[]={
  0x53,0x56,0x8b,0xf1,0x8b,0x4e,0x18,0x8b,0x46,0x0c,0x8b,0x11,
  0x8b,0x18,0x8b,0x42,0x08,0x57,0x8b,0x79,0x0c,0xff,0xd0,
  0x8b,0x53,0x04,0x51,0x8b,0x4e,0x0c,0xd9,0x1c,0x24,0x57,0xff,0xd2,
  0x8b,0x4e,0x0c,0x8b,0x01,0x8b,0x50,0x20,0xff,0xd2,
  0xd8,0x1d,0x6c,0x10,0xa1,0x00,0x5f,0xdf,0xe0,0xf6,0xc4,0x41,
  0x75,0x0b,0x8b,0x06,0x8b,0x50,0x14,0x8b,0xce,0x5e,0x5b,0xff,0xe2,
  0x5e,0x5b,0xc3
};
static const BYTE physics_step_signature[]={
  0x83,0xec,0x0c,0x56,0x8b,0xf1,0x8b,0x0d,0xcc,0x64,0xab,0x00,
  0x8b,0x46,0x28,0x57,0x8b,0x38,0x51,0x8d,0x4c,0x24,0x0c,
  0xe8,0xd4,0x24,0xd1,0xff,0x8b,0x4e,0x28,0x8b,0x57,0x08,
  0x50,0xff,0xd2,0x8d,0x4c,0x24,0x08,0xe8,0xe2,0x24,0xd1,0xff,
  0xd9,0x46,0x20,0x51,0x8b,0x4e,0x1c,0xd9,0x1c,0x24,
  0xe8,0xe3,0xb7,0x01,0x00,0x5f,0x5e,0x83,0xc4,0x0c,0xc3
};
static const BYTE physics_site_original[]={0x8b,0x53,0x04,0x51,0x8b,0x4e,0x0c};
static BOOL physics_matches(const BYTE *base){
  const BYTE constructor[]={0xd9,0x05,0xd8,0x86,0xa4,0x00,0x8b,0x4e,0x0c,0xd9,0x56,0x20};
  const BYTE getter[]={0xd9,0x41,0x20,0xc3};
  return !memcmp(base+0x3df7f0,physics_update_signature,sizeof(physics_update_signature))&&
    !memcmp(base+0x3df840,physics_step_signature,sizeof(physics_step_signature))&&
    !memcmp(base+0x3dfb4b,constructor,sizeof(constructor))&&
    !memcmp(base+0x3df950,getter,sizeof(getter));
}
static void make_physics_stub(BYTE out[15],uintptr_t address,uintptr_t resume){
  out[0]=0xd9;out[1]=0x56;out[2]=0x20; // FST [ESI+20h], leaves ST(0) intact
  memcpy(out+3,physics_site_original,sizeof(physics_site_original));
  branch32(out+10,address+10,resume);
}
static BYTE *prepare_physics_stub(BYTE *site){
  if(memcmp(site,physics_site_original,sizeof(physics_site_original)))return NULL;
  BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
  if(!code)return NULL;
  make_physics_stub(code,(uintptr_t)code,(uintptr_t)(site+7));
  if(!seal_code(code,15)){VirtualFree(code,0,MEM_RELEASE);return NULL;}
  return code;
}
/* Validate and prepare both patches before mutating either site. Acquire
   write access to both first, so a refused protection change cannot leave
   only the high-FPS limiter enabled. Initialization precedes rendering. */
static BOOL install_timing_v4(BYTE *base,uintptr_t callback){
  BYTE *frame=base+0xda40,*physics=base+0x3df807;
  if(!limiter_matches(base,frame)||!physics_matches(base))return FALSE;
  BYTE *physics_code=prepare_physics_stub(physics);
  if(!physics_code)return FALSE;
  BYTE *frame_code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
  if(!frame_code){VirtualFree(physics_code,0,MEM_RELEASE);return FALSE;}
  size_t n=make_pacer_stub(frame_code,(uintptr_t)frame_code,callback,(uintptr_t)(frame+sizeof(limiter_signature)));
  DWORD frame_old,physics_old,unused;
  if(!seal_code(frame_code,n))goto refuse;
  if(!VirtualProtect(frame,7,PAGE_EXECUTE_READWRITE,&frame_old))goto refuse;
  if(!VirtualProtect(physics,7,PAGE_EXECUTE_READWRITE,&physics_old)){
    VirtualProtect(frame,7,frame_old,&unused);goto refuse;
  }
  BYTE p[7];memset(p,0x90,sizeof(p));
  branch32(p,(uintptr_t)physics,(uintptr_t)physics_code);memcpy(physics,p,7);
  branch32(p,(uintptr_t)frame,(uintptr_t)frame_code);memcpy(frame,p,7);
  FlushInstructionCache(GetCurrentProcess(),physics,7);
  FlushInstructionCache(GetCurrentProcess(),frame,7);
  VirtualProtect(physics,7,physics_old,&unused);VirtualProtect(frame,7,frame_old,&unused);
  return TRUE;
refuse:
  VirtualFree(frame_code,0,MEM_RELEASE);VirtualFree(physics_code,0,MEM_RELEASE);return FALSE;
}
