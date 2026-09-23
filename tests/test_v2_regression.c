#ifdef NDEBUG
#undef NDEBUG
#endif
#define WOZ_TEST
#include "../src/woz_fix.c"
#include <assert.h>

#define THISCALL __attribute__((thiscall))
static const char *analysis_exe="work/exe-analysis/WoZRetail.exe.unpacked.exe";
typedef uintptr_t (THISCALL *Upload)(void*,const void*,unsigned);
static unsigned mode_seen,offset_seen,length_seen,calls;
static void *object_seen;
static uintptr_t THISCALL lock_mock(void *obj,unsigned offset,unsigned length,unsigned mode){
    mode_seen=mode;offset_seen=offset;length_seen=length;object_seen=obj;calls++;
    return 0x12345678;
}
static BYTE *caller(BYTE *p, BYTE *upload){
    BYTE code[]={0xff,0x74,0x24,0x08,0xff,0x74,0x24,0x08,0xb8,0,0,0,0,0xff,0xd0,0xc2,0x08,0x00};
    uint32_t dest=(uint32_t)(uintptr_t)upload;memcpy(code+9,&dest,4);memcpy(p,code,sizeof(code));return p+15;
}
static void test_mesh(void);
int main(int argc,char **argv){
    if(argc>1)analysis_exe=argv[1];
    BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(code);
    const BYTE original[]={0x56,0x57,0x8b,0x7c,0x24,0x10,0x6a,0x00,0x8b,0xf1,0x8b,0x06,0x8b,0x50,0x24,0x57,0x6a,0x00,0xff,0xd2,0x5f,0x5e,0xc2,0x08,0x00};
    memcpy(code,original,sizeof(original));
    BYTE *return_address=caller(code+128,code);caller(code+256,code);
    void *table[16]={0};table[9]=(void*)lock_mock;
    BYTE obj[32]={0};*(void***)obj=table;
    Upload ui=(Upload)(code+128),other=(Upload)(code+256);
    obj[0x11]=1;
    assert(ui(obj,NULL,42)==0x12345678);assert(mode_seen==0);
    assert(install_detour(code+6,(uintptr_t)return_address));
    assert(ui(obj,NULL,42)==0x12345678);assert(mode_seen==1&&offset_seen==0&&length_seen==42&&object_seen==obj);
    obj[0x11]=0;
    assert(ui(obj,NULL,19)==0x12345678);assert(mode_seen==0&&length_seen==19);
    obj[0x11]=1;
    assert(other(obj,NULL,53)==0x12345678);assert(mode_seen==0&&length_seen==53);
    for(unsigned i=0;i<10000;i++){assert(ui(obj,NULL,i)==0x12345678);assert(mode_seen==1&&length_seen==i);}
    assert(!memcmp(code,original,6)&&!memcmp(code+12,original+12,sizeof(original)-12));
    BYTE mismatch[6]={0};assert(!install_detour(mismatch,(uintptr_t)return_address));
    test_mesh();
    puts("PASS: x86 thiscall stack and registers, exact caller guard, dynamic/static guard, preserved surrounding instructions, version mismatch refusal, 10000 repeated calls.");
    return 0;
}

static unsigned default_calls,managed_calls,created_sizes[2],write_only[2],managed_flags[2];
static void *buffers[2];
static unsigned buffer_number(void *object){return object==buffers[0]?0:1;}
static void *THISCALL new_vb(void *renderer){(void)renderer;return buffers[0];}
static void *THISCALL new_ib(void *renderer){(void)renderer;return buffers[1];}
static unsigned char THISCALL default_create(void *object,unsigned bytes){
    default_calls++;created_sizes[buffer_number(object)]=bytes;return 1;
}
static unsigned char THISCALL managed_create(void *object,unsigned bytes,unsigned wo,unsigned managed){
    unsigned n=buffer_number(object);managed_calls++;created_sizes[n]=bytes;write_only[n]=wo;managed_flags[n]=managed;return 1;
}
typedef void *(THISCALL *Ctor)(void*,void*,unsigned,unsigned,void*,unsigned,unsigned);
static void test_mesh(void){
    FILE *file=fopen(analysis_exe,"rb");assert(file);
    BYTE *code=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);assert(code);
    assert(!fseek(file,0x30ff00,SEEK_SET));assert(fread(code,1,0xa4,file)==0xa4);fclose(file);
    BYTE original[0xa4];memcpy(original,code,sizeof(original));
    BYTE *factory=code+512;memcpy(factory,mesh_factory_signature,sizeof(mesh_factory_signature));
    void *buffer_table[3]={(void*)0,(void*)default_create,(void*)managed_create};
    BYTE vb[32]={0},ib[32]={0},mesh[32]={0};
    *(void***)vb=buffer_table;*(void***)ib=buffer_table;buffers[0]=vb;buffers[1]=ib;
    void *renderer_table[7]={0};renderer_table[4]=(void*)new_vb;renderer_table[6]=(void*)new_ib;
    void **renderer=renderer_table;Ctor ctor=(Ctor)code;
    assert(ctor(mesh,&renderer,123,456,(void*)0x1234,40,0)==mesh);
    assert(default_calls==2&&managed_calls==0&&created_sizes[0]==4920&&created_sizes[1]==912);
    assert(ctor(mesh,&renderer,123,456,(void*)0x1234,40,1)==mesh);
    assert(managed_calls==2&&write_only[0]==1&&write_only[1]==1&&managed_flags[0]==1&&managed_flags[1]==1);
    BYTE bad[sizeof(mesh_factory_signature)];memcpy(bad,factory,sizeof(bad));bad[0]^=1;
    assert(!install_mesh_fix(bad,code+0x5b));assert(!memcmp(code,original,sizeof(original)));
    code[0x5b]^=1;
    assert(!install_mesh_fix(factory,code+0x5b));assert(!memcmp(factory,mesh_factory_signature,sizeof(mesh_factory_signature)));
    code[0x5b]^=1;
    assert(install_mesh_fix(factory,code+0x5b));
    assert(factory[6]==1);original[0x68]=0;original[0x77]=0;
    assert(!memcmp(code,original,sizeof(original)));
    for(unsigned j=0;j<10000;j++){
        unsigned vertices=1+j%1000,indices=3+j%3000;
        assert(ctor(mesh,&renderer,vertices,indices,(void*)0x1234,40,factory[6])==mesh);
        assert(created_sizes[0]==vertices*40&&created_sizes[1]==indices*2);
        assert(write_only[0]==0&&write_only[1]==0&&managed_flags[0]==1&&managed_flags[1]==1);
        assert(*(void**)(mesh+4)==vb&&*(void**)(mesh+8)==ib&&*(void**)(mesh+12)==(void*)0x1234);
    }
    unsigned previous=default_calls;
    assert(ctor(mesh,&renderer,1,3,(void*)0x1234,40,0)==mesh);assert(default_calls==previous+2);
    assert(!install_mesh_fix(factory,code+0x5b));
    puts("PASS: original x86 mesh constructor executed; managed/readable VB+IB, sizes and fields preserved; other dynamic callers unchanged; mismatch has no partial writes; 10000 constructions.");
    VirtualFree(code,0,MEM_RELEASE);
}
