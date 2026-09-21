#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* World of Zoo, Steam executable SHA256
   622cd4914c4f85f8af949100746078be6a6c111303758cc74206f210e813bfc3

   The Scaleform full-index upload at RVA 22C916 tail-calls the engine's
   RALIndexBufferD3D::upload function at RVA 3087E0. It locks offset zero
   without flags, although the UI index buffer is DYNAMIC|WRITEONLY.
   Change that one caller's dynamic-buffer lock to engine mode 1, which
   RALIndexBufferD3D::lock translates to D3DLOCK_DISCARD (0x2000).

   No COM object/vtable changes. No timer or FPS-limit changes. A six-byte
   detour in the already unpacked in-memory EXE preserves the original
   upload, memcpy, unlock and bind code. Static buffers and other callers
   keep mode zero. The EXE on disk is untouched.

   V2: the CPU-deformed mesh factory at RVA 4334B4 requests dynamic,
   write-only DEFAULT vertex/index buffers, then both animation updates
   and CPU triangle queries lock/read their retained contents (70FE80,
   8334F0, 8343C0). DISCARD is not valid for this preserving/read path.
   Select the existing managed creation branch for that factory only.
   In the paired-mesh constructor, managed buffers are made readable
   (writeOnly=false). Other dynamic allocations retain their old branch.
   Direct3D maintains the system-memory copy and uploads changes. This
   trades an extra CPU copy for removal of GPU readback on these locks.
*/
static const BYTE original_site[6]={0x6a,0x00,0x8b,0xf1,0x8b,0x06};

/* At this point ESI/EDI have already been saved by the original prologue.
   The caller return address is [ESP+8]. Reproduce the overwritten
   instructions, conditionally select the mode in EDX, push it, then jump
   back to the original MOV EDX,[EAX+24h]. EDX and EFLAGS are volatile. */
static void make_stub(BYTE out[29], const BYTE *address, uintptr_t caller, uintptr_t resume) {
    const BYTE template[29]={
        0x8b,0xf1, 0x8b,0x06, 0x33,0xd2,
        0x81,0x7c,0x24,0x08, 0,0,0,0,
        0x75,0x07, 0x80,0x7e,0x11,0x00, 0x0f,0x95,0xc2,
        0x52, 0xe9,0,0,0,0
    };
    memcpy(out,template,29);
    uint32_t c=(uint32_t)caller;
    uint32_t jump=(uint32_t)resume-(uint32_t)(uintptr_t)(address+29);
    memcpy(out+10,&c,4);memcpy(out+25,&jump,4);
}

static BOOL install_detour(BYTE *site, uintptr_t caller) {
    if(memcmp(site,original_site,sizeof(original_site)))return FALSE;
    BYTE *stub=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!stub)return FALSE;
    make_stub(stub,stub,caller,(uintptr_t)(site+6));
    DWORD old,unused;
    if(!VirtualProtect(stub,4096,PAGE_EXECUTE_READ,&old)){VirtualFree(stub,0,MEM_RELEASE);return FALSE;}
    FlushInstructionCache(GetCurrentProcess(),stub,29);
    BYTE replacement[6]={0xe9,0,0,0,0,0x90};
    uint32_t jump=(uint32_t)(uintptr_t)stub-(uint32_t)(uintptr_t)(site+5);
    memcpy(replacement+1,&jump,4);
    if(!VirtualProtect(site,6,PAGE_EXECUTE_READWRITE,&old)){VirtualFree(stub,0,MEM_RELEASE);return FALSE;}
    memcpy(site,replacement,sizeof(replacement));
    FlushInstructionCache(GetCurrentProcess(),site,6);
    VirtualProtect(site,6,old,&unused);
    return TRUE;
}

static const BYTE mesh_factory_signature[]={
    0x8b,0x48,0x14,0x8b,0x2b,0x6a,0x00,0x51,0x53,0x8b,0xc8,
    0xe8,0xb1,0x1c,0xd0,0xff,0x8b,0x17,0x50,0x8b,0x42,0x10
};
static const BYTE mesh_constructor_signature[]={
    0x80,0x7c,0x24,0x28,0x00,0x74,0x25,0x8b,0x52,0x08,
    0x6a,0x01,0x6a,0x01,0x50,0xff,0xd2,0x8b,0x4e,0x08,
    0x8b,0x01,0x8b,0x40,0x08,0x6a,0x01,0x6a,0x01,
    0x8d,0x14,0x1b,0x52,0xff,0xd0
};

static BOOL mesh_matches(const BYTE *factory,const BYTE *constructor){
    return !memcmp(factory,mesh_factory_signature,sizeof(mesh_factory_signature)) &&
           !memcmp(constructor,mesh_constructor_signature,sizeof(mesh_constructor_signature));
}

static BOOL install_mesh_fix(BYTE *factory,BYTE *constructor){
    if(!mesh_matches(factory,constructor))return FALSE;
    DWORD factory_old,ctor_old,unused;
    if(!VirtualProtect(factory,sizeof(mesh_factory_signature),PAGE_EXECUTE_READWRITE,&factory_old))return FALSE;
    if(!VirtualProtect(constructor,sizeof(mesh_constructor_signature),PAGE_EXECUTE_READWRITE,&ctor_old)){
        VirtualProtect(factory,sizeof(mesh_factory_signature),factory_old,&unused);
        return FALSE;
    }
    // All signatures and both writable spans are checked before mutation.
    factory[6]=1;       // 8334B5: paired mesh requests the managed branch.
    constructor[13]=0;  // 70FF68: managed vertex buffer is readable.
    constructor[28]=0;  // 70FF77: managed index buffer is readable.
    FlushInstructionCache(GetCurrentProcess(),factory,sizeof(mesh_factory_signature));
    FlushInstructionCache(GetCurrentProcess(),constructor,sizeof(mesh_constructor_signature));
    VirtualProtect(constructor,sizeof(mesh_constructor_signature),ctor_old,&unused);
    VirtualProtect(factory,sizeof(mesh_factory_signature),factory_old,&unused);
    return TRUE;
}

#ifndef WOZ_TEST
static HMODULE self;
static void *(WINAPI *real_create)(UINT);
static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
static WCHAR logpath[MAX_PATH];

static void logline(const char *text){
    HANDLE f=CreateFileW(logpath,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(f!=INVALID_HANDLE_VALUE){DWORD n;WriteFile(f,text,(DWORD)strlen(text),&n,NULL);CloseHandle(f);}
}

static BOOL supported_image(BYTE *base) {
    IMAGE_DOS_HEADER *dos=(void*)base;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE||dos->e_lfanew<0||dos->e_lfanew>4096)return FALSE;
    IMAGE_NT_HEADERS *nt=(void*)(base+dos->e_lfanew);
    if(nt->Signature!=IMAGE_NT_SIGNATURE||nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||nt->OptionalHeader.SizeOfImage<0x6c0000)return FALSE;
    const BYTE ui_call[]={0xe8,0xa5,0x71,0xf0,0xff,0x88,0x5e,0x25};
    const BYTE upload[]={0x56,0x57,0x8b,0x7c,0x24,0x10,0x6a,0x00,0x8b,0xf1,0x8b,0x06,0x8b,0x50,0x24,0x57,0x6a,0x00,0xff,0xd2};
    const BYTE mapping[]={0xbe,0x00,0x20,0x00,0x00};
    const BYTE dynamic_usage[]={0x68,0x08,0x02,0x00,0x00,0x53,0x50,0xff,0xd2};
    const BYTE dynamic_field[]={0x88,0x46,0x11,0x88,0x46,0x10};
    const BYTE vertex_wrapper[]={0x8b,0x44,0x24,0x0c,0x8b,0x54,0x24,0x08,0x50,0x8b,0x44,0x24,0x08,0x52,0x6a,0x00,0x50,0xe8,0x1a,0xfd,0xff,0xff,0xc2,0x0c,0x00};
    const BYTE index_wrapper[]={0x8b,0x44,0x24,0x0c,0x8b,0x54,0x24,0x08,0x50,0x8b,0x44,0x24,0x08,0x52,0x6a,0x00,0x50,0xe8,0x4a,0xfd,0xff,0xff,0xc2,0x0c,0x00};
    return !memcmp(base+0x22c916,ui_call,sizeof(ui_call)) &&
           !memcmp(base+0x3087e0,upload,sizeof(upload)) &&
           !memcmp(base+0x308846,mapping,sizeof(mapping)) &&
           !memcmp(base+0x308979,dynamic_usage,sizeof(dynamic_usage)) &&
           !memcmp(base+0x30899c,dynamic_field,sizeof(dynamic_field)) &&
           mesh_matches(base+0x4334af,base+0x30ff5b) &&
           !memcmp(base+0x308680,vertex_wrapper,sizeof(vertex_wrapper)) &&
           !memcmp(base+0x3089b0,index_wrapper,sizeof(index_wrapper));
}

static BOOL CALLBACK initialize(PINIT_ONCE a,PVOID b,PVOID *c) {
    WCHAR path[MAX_PATH];DWORD n=GetModuleFileNameW(self,path,MAX_PATH);
    if(!n||n>=MAX_PATH)return TRUE;
    while(n&&path[n-1]!=L'\\')--n;
    if(n+32>=MAX_PATH)return TRUE;
    path[n]=0;lstrcpyW(logpath,path);lstrcatW(logpath,L"WoZIndexFix.log");
    logline("\r\nWoZ Index and Mesh Fix V2\r\n");
    n=GetSystemDirectoryW(path,MAX_PATH);
    if(!n||n+11>=MAX_PATH)return TRUE;
    lstrcatW(path,L"\\d3d9.dll");
    HMODULE system=LoadLibraryW(path);
    if(system)real_create=(void*)GetProcAddress(system,"Direct3DCreate9");
    if(!real_create){logline("ERROR: Windows Direct3DCreate9 unavailable\r\n");return TRUE;}
    BYTE *base=(BYTE*)GetModuleHandleW(NULL);
    if(!supported_image(base)){logline("NOT APPLIED: executable signatures do not match; original rendering retained\r\n");return TRUE;}
    // The first Direct3DCreate9 happens before UI buffers or rendering exist.
    // Pin the proxy so the detour cannot outlive the module during the game.
    HMODULE pinned;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(LPCWSTR)(uintptr_t)&initialize,&pinned)){
        logline("NOT APPLIED: module lifetime could not be secured\r\n");return TRUE;
    }
    if(install_detour(base+0x3087e6,(uintptr_t)(base+0x22c91b)))
        logline("APPLIED: dynamic UI index uploads use DISCARD. Native 33 ms limiter unchanged. No Direct3D object/vtable hooks.\r\n");
    else logline("NOT APPLIED: memory patch unavailable; original rendering retained\r\n");
    if(install_mesh_fix(base+0x4334af,base+0x30ff5b))
        logline("APPLIED: CPU-accessed meshes use readable MANAGED vertex/index buffers. Native mesh lock/unlock and 33 ms limiter unchanged.\r\n");
    else logline("MESH FIX NOT APPLIED: memory patch unavailable; mesh allocation retained\r\n");
    return TRUE;
}

void *WINAPI Direct3DCreate9(UINT version){
    InitOnceExecuteOnce(&once,initialize,NULL,NULL);
    if(!real_create)return NULL;
    void *result=real_create(version);
    logline(result?"Original Windows Direct3DCreate9: OK\r\n":"Original Windows Direct3DCreate9: NULL\r\n");
    return result;
}

BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved){
    if(reason==DLL_PROCESS_ATTACH){self=module;DisableThreadLibraryCalls(module);}
    return TRUE;
}
#endif
