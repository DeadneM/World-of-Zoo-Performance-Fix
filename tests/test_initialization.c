#ifdef NDEBUG
#undef NDEBUG
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static BYTE *fixture;
static BOOL refuse_physics_write;
static BOOL WINAPI checked_protect(LPVOID address,SIZE_T size,DWORD protection,PDWORD previous){
  if(refuse_physics_write&&address==fixture+0x3df807&&protection==PAGE_EXECUTE_READWRITE)return FALSE;
  return VirtualProtect(address,size,protection,previous);
}
static HMODULE WINAPI fixture_module(LPCWSTR name){return name?GetModuleHandleW(name):(HMODULE)fixture;}
#define GetModuleHandleW fixture_module
#define VirtualProtect checked_protect
#include "../src/woz_fix.c"
#undef GetModuleHandleW
#undef VirtualProtect
int main(int argc,char **argv){
  assert(argc==2||argc==3);
  FILE *f=fopen(argc==3?argv[2]:"work/exe-analysis/WoZRetail.exe.unpacked.exe","rb");assert(f);
  assert(!fseek(f,0,SEEK_END));long size=ftell(f);assert(size>0);rewind(f);
  BYTE *file=malloc((size_t)size);assert(file);assert(fread(file,1,(size_t)size,f)==(size_t)size);fclose(f);
  IMAGE_NT_HEADERS32 *nt=(void*)(file+((IMAGE_DOS_HEADER*)file)->e_lfanew);
  fixture=VirtualAlloc(NULL,nt->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(fixture);
  memcpy(fixture,file,nt->OptionalHeader.SizeOfHeaders);
  IMAGE_SECTION_HEADER *s=IMAGE_FIRST_SECTION(nt);
  for(unsigned i=0;i<nt->FileHeader.NumberOfSections;i++)if(s[i].SizeOfRawData){
    assert(s[i].PointerToRawData+s[i].SizeOfRawData<=(size_t)size);
    memcpy(fixture+s[i].VirtualAddress,file+s[i].PointerToRawData,s[i].SizeOfRawData);
  }
  free(file);
  put32(fixture+0xda40+29,(uintptr_t)(fixture+0x6a10c0));
  self=GetModuleHandleW(NULL);
  WCHAR file_path[MAX_PATH];DWORD n=GetModuleFileNameW(self,file_path,MAX_PATH);assert(n&&n<MAX_PATH);
  while(n&&file_path[n-1]!=L'\\')--n;
  lstrcpyW(file_path+n,L"WoZPerformanceFix.ini");
  const char *settings="[Timing]\r\nTargetFPS=60\r\n[Buffers]\r\nReadOnlyMeshQueries=1\r\n";
  if(!strcmp(argv[1],"native"))settings="[Timing]\r\nTargetFPS=0\r\n[Buffers]\r\nReadOnlyMeshQueries=0\r\n";
  if(!strcmp(argv[1],"invalid"))settings="[Timing]\r\nTargetFPS=361\r\n[Buffers]\r\nReadOnlyMeshQueries=1\r\n";
  HANDLE h=CreateFileW(file_path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);assert(h!=INVALID_HANDLE_VALUE);
  DWORD written;assert(WriteFile(h,settings,(DWORD)strlen(settings),&written,NULL));CloseHandle(h);
  if(!strcmp(argv[1],"mismatch"))fixture[0x22c916]^=1;
  if(!strcmp(argv[1],"fps_mismatch"))fixture[0xda40]^=1;
  if(!strcmp(argv[1],"query_mismatch"))fixture[0x433518]^=1;
  if(!strcmp(argv[1],"physics_mismatch"))fixture[0x3df7f0]^=1;
  if(!strcmp(argv[1],"step_mismatch"))fixture[0x3df840]^=1;
  refuse_physics_write=!strcmp(argv[1],"protect_failure");
  assert(initialize(NULL,NULL,NULL));assert(real_create);
  if(!strcmp(argv[1],"mismatch")){
    assert(fixture[0x3087e6]==0x6a&&fixture[0x4334b5]==0&&fixture[0x30fe80]==0x56&&fixture[0xda40]==0x80);
    assert(!memcmp(fixture+0x3df807,physics_site_original,7));
  }else{
    assert(fixture[0x3087e6]==0xe9&&fixture[0x4334b5]==1&&fixture[0x30ff68]==0&&fixture[0x30ff77]==0);
    BOOL native=!strcmp(argv[1],"native");
    BOOL fps_mismatch=!strcmp(argv[1],"fps_mismatch");
    BOOL query_mismatch=!strcmp(argv[1],"query_mismatch");
    BOOL timing_refused=!strcmp(argv[1],"physics_mismatch")||!strcmp(argv[1],"step_mismatch")||refuse_physics_write;
    assert(fixture[0x30fe80]==(native||query_mismatch?0x56:0xe9));
    assert(fixture[0xda40]==(native||timing_refused?0x80:fps_mismatch?0x81:0xe9));
    if(native||fps_mismatch||timing_refused)assert(!memcmp(fixture+0x3df807,physics_site_original,7));
    else {assert(pacing.fps==60);assert(fixture[0x3df807]==0xe9);}
  }
  if(pacing_timer)CloseHandle(pacing_timer);
  printf("PASS: full initialization fixture, case %s; INI and runtime signature checks.\n",argv[1]);
  return 0;
}
