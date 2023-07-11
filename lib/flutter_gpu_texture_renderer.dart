import 'flutter_gpu_texture_renderer_platform_interface.dart';

class FlutterGpuTextureRenderer {
  Future<int?> registerTexture() {
    return FlutterGpuTextureRendererPlatform.instance.registerTexture();
  }

  Future<int?> unregisterTexture(int id) {
    return FlutterGpuTextureRendererPlatform.instance.unregisterTexture(id);
  }

  Future<int?> output(int id) {
    return FlutterGpuTextureRendererPlatform.instance.output(id);
  }

  Future<int?> device(int id) {
    return FlutterGpuTextureRendererPlatform.instance.device(id);
  }

  Future<int?> fps(int id) {
    return FlutterGpuTextureRendererPlatform.instance.fps(id);
  }
}
