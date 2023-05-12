#ifndef FFI_H
#define FFI_H

typedef void (*DuplicateCallback)(void *output, void* texture);

void SetAndStartDuplicateThread(void *pOutput, void *pDevice) {};

void StopDuplicateThread() {};

#endif