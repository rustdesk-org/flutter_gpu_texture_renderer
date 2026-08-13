#ifndef FLUTTER_PLUGIN_FLUTTER_GPU_TEXTURE_RENDERER_PLUGIN_C_API_H_
#define FLUTTER_PLUGIN_FLUTTER_GPU_TEXTURE_RENDERER_PLUGIN_C_API_H_

#include <flutter_plugin_registrar.h>
#include <stdint.h>

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FLUTTER_PLUGIN_EXPORT __declspec(dllimport)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_PLUGIN_EXPORT void FlutterGpuTextureRendererPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

FLUTTER_PLUGIN_EXPORT void FlutterGpuTextureRendererPluginCApiSetTexture(void *output, void *texture);

FLUTTER_PLUGIN_EXPORT int64_t FlutterGpuTextureRendererPluginCApiGetAdapterLuid();

// Frames for which the engine fetched this output's surface descriptor; an
// EGL bind failure still advances it, so 0 means "never composited", not
// "rendered correctly". Also 0 if the output is unknown/unregistered.
FLUTTER_PLUGIN_EXPORT uint64_t FlutterGpuTextureRendererPluginCApiGetConsumed(void *output);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_PLUGIN_FLUTTER_GPU_TEXTURE_RENDERER_PLUGIN_C_API_H_
