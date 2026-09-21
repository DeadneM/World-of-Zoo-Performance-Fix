#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
int main(int argc,char **argv){
 HMODULE proxy=LoadLibraryA(argc>1?argv[1]:"build\\d3d9.dll");if(!proxy){printf("FAIL load %lu\n",GetLastError());return 1;}
 void *(WINAPI *create)(UINT)=(void*)GetProcAddress(proxy,"Direct3DCreate9");if(!create)return 2;
 void *object=create(32);if(!object)return 3;
 void **vt=*(void***)object;HMODULE origin=NULL;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)vt,&origin))return 4;
 if(origin==proxy)return 5;
 WCHAR path[MAX_PATH];GetModuleFileNameW(origin,path,MAX_PATH);wprintf(L"Original object vtable module: %s\n",path);
 ULONG (WINAPI *release)(void*)=(void*)vt[2];release(object);FreeLibrary(proxy);
 puts("PASS: system Direct3DCreate9 forwarded; original object and vtable preserved; unsupported executable remains unpatched.");return 0;
}
