#ifdef NDEBUG
#undef NDEBUG
#endif
#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct Vertex { float x,y,z,rhw; DWORD color; } Vertex;
enum { VERTICES=6000, ITERATIONS=120 };
static double seconds(void){LARGE_INTEGER t,f;QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return (double)t.QuadPart/(double)f.QuadPart;}
static void checked(HRESULT result,const char *operation){
    if(FAILED(result)){fprintf(stderr,"FAIL %s: %08lx\n",operation,(unsigned long)result);ExitProcess(2);}
}
static void exercise(IDirect3DDevice9 *device,BOOL managed){
    IDirect3DVertexBuffer9 *vb=NULL;IDirect3DIndexBuffer9 *ib=NULL;
    DWORD usage=managed?0:(D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY);
    D3DPOOL pool=managed?D3DPOOL_MANAGED:D3DPOOL_DEFAULT;
    checked(IDirect3DDevice9_CreateVertexBuffer(device,sizeof(Vertex)*VERTICES,usage,0,pool,&vb,NULL),"CreateVB");
    checked(IDirect3DDevice9_CreateIndexBuffer(device,sizeof(WORD)*VERTICES,usage,D3DFMT_INDEX16,pool,&ib,NULL),"CreateIB");
    D3DVERTEXBUFFER_DESC vd;D3DINDEXBUFFER_DESC id;
    checked(IDirect3DVertexBuffer9_GetDesc(vb,&vd),"VBdesc");checked(IDirect3DIndexBuffer9_GetDesc(ib,&id),"IBdesc");
    assert(vd.Usage==usage&&id.Usage==usage&&vd.Pool==pool&&id.Pool==pool);
    Vertex *vertices;WORD *indices;
    checked(IDirect3DVertexBuffer9_Lock(vb,0,0,(void**)&vertices,0),"InitialVBLock");
    checked(IDirect3DIndexBuffer9_Lock(ib,0,0,(void**)&indices,0),"InitialIBLock");
    for(unsigned i=0;i<VERTICES;i++){
        vertices[i]=(Vertex){i%3==1?20.f:2.f,i%3==2?20.f:2.f,0.5f,1.f,0xffffffff};indices[i]=(WORD)i;
    }
    checked(IDirect3DIndexBuffer9_Unlock(ib),"InitialIBUnlock");checked(IDirect3DVertexBuffer9_Unlock(vb),"InitialVBUnlock");
    checked(IDirect3DDevice9_SetStreamSource(device,0,vb,0,sizeof(Vertex)),"Stream");
    checked(IDirect3DDevice9_SetIndices(device,ib),"Indices");
    checked(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZRHW|D3DFVF_DIFFUSE),"FVF");
    checked(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE),"Cull");
    checked(IDirect3DDevice9_SetRenderState(device,D3DRS_LIGHTING,FALSE),"Lighting");
    double start=seconds(),locks=0,maximum=0;unsigned done=0;
    for(unsigned n=0;n<ITERATIONS;n++){
        checked(IDirect3DDevice9_BeginScene(device),"Begin");
        checked(IDirect3DDevice9_DrawIndexedPrimitive(device,D3DPT_TRIANGLELIST,0,0,VERTICES,0,VERTICES/3),"Draw");
        checked(IDirect3DDevice9_EndScene(device),"End");
        double t=seconds();
        checked(IDirect3DVertexBuffer9_Lock(vb,0,0,(void**)&vertices,0),"ReadWriteVBLock");
        checked(IDirect3DIndexBuffer9_Lock(ib,0,0,(void**)&indices,0),"ReadWriteIBLock");
        double elapsed=seconds()-t;locks+=elapsed;if(elapsed>maximum)maximum=elapsed;
        // Check actual retained contents after rendering and partial edits.
        for(unsigned i=0;i<VERTICES;i++){
            assert(indices[i]==i);assert(vertices[i].rhw==1.f&&vertices[i].z==0.5f);
        }
        assert(vertices[VERTICES-1].color==0xffffffff);
        vertices[0].color=0xff000000|n;
        checked(IDirect3DIndexBuffer9_Unlock(ib),"IBUnlock");checked(IDirect3DVertexBuffer9_Unlock(vb),"VBUnlock");
        done++;
        if(seconds()-start>5.0)break;
    }
    // Wait for submitted work to finish, with a bounded timeout.
    IDirect3DQuery9 *query=NULL;checked(IDirect3DDevice9_CreateQuery(device,D3DQUERYTYPE_EVENT,&query),"Query");
    checked(IDirect3DQuery9_Issue(query,D3DISSUE_END),"QueryIssue");
    double deadline=seconds()+5.0;HRESULT ready;
    while((ready=IDirect3DQuery9_GetData(query,NULL,0,D3DGETDATA_FLUSH))==S_FALSE&&seconds()<deadline)Sleep(1);
    checked(ready,"QueryData");assert(ready==S_OK);IDirect3DQuery9_Release(query);
    printf("PASS %s: %u updates/draws/readbacks, data preserved; lock total %.3f ms, max %.3f ms, total %.3f ms\n",
        managed?"readable MANAGED":"original DYNAMIC WRITEONLY DEFAULT",done,locks*1000,maximum*1000,(seconds()-start)*1000);
    checked(IDirect3DDevice9_SetStreamSource(device,0,NULL,0,0),"UnbindVB");
    checked(IDirect3DDevice9_SetIndices(device,NULL),"UnbindIB");
    IDirect3DVertexBuffer9_Release(vb);IDirect3DIndexBuffer9_Release(ib);
}
int main(void){
    WCHAR path[MAX_PATH];UINT n=GetSystemDirectoryW(path,MAX_PATH);assert(n&&n+11<MAX_PATH);lstrcatW(path,L"\\d3d9.dll");
    HMODULE d3d=LoadLibraryW(path);assert(d3d);
    IDirect3D9 *(WINAPI *create)(UINT)=(void*)GetProcAddress(d3d,"Direct3DCreate9");assert(create);
    IDirect3D9 *api=create(D3D_SDK_VERSION);assert(api);
    WNDCLASSW cls={0};cls.lpfnWndProc=DefWindowProcW;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"WoZManagedMeshTest";
    assert(RegisterClassW(&cls));HWND window=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,32,32,NULL,NULL,cls.hInstance,NULL);assert(window);
    D3DPRESENT_PARAMETERS pp={0};pp.BackBufferWidth=32;pp.BackBufferHeight=32;pp.BackBufferFormat=D3DFMT_UNKNOWN;pp.BackBufferCount=1;
    pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.hDeviceWindow=window;pp.Windowed=TRUE;pp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
    IDirect3DDevice9 *device=NULL;
    checked(IDirect3D9_CreateDevice(api,D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device),"CreateDevice");
    exercise(device,FALSE);exercise(device,TRUE);
    IDirect3DDevice9_Release(device);IDirect3D9_Release(api);DestroyWindow(window);FreeLibrary(d3d);
    return 0;
}
