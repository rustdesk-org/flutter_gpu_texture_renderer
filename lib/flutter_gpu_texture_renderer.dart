import 'flutter_gpu_texture_renderer_platform_interface.dart';

class FlutterGpuTextureRenderer {
  Future<int?> create() {
    return FlutterGpuTextureRendererPlatform.instance.create();
  }
}
