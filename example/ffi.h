#ifndef FFI_H
#define FFI_H

typedef void (*DuplicateCallback)(void *output, void* texture);

void StartDuplicateThread(void *pDevice) {};

void StopDuplicateThread() {};

void AddOutput(void *output) {};

void RemoveOutput(void *output) {};

void* FlutterGpuTextureRendererPluginCApiGetDevice() {};

#endif