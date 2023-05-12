import 'flutter_gpu_texture_renderer_platform_interface.dart';

class FlutterGpuTextureRenderer {
  Future<int?> registerTexture() {
    return FlutterGpuTextureRendererPlatform.instance.registerTexture();
  }

  Future<int?> unregisterTexture() {
    return FlutterGpuTextureRendererPlatform.instance.unregisterTexture();
  }

  Future<int?> output() {
    return FlutterGpuTextureRendererPlatform.instance.output();
  }

  Future<int?> device() {
    return FlutterGpuTextureRendererPlatform.instance.device();
  }
}
