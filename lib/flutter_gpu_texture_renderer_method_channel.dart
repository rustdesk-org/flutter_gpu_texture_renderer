import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_gpu_texture_renderer_platform_interface.dart';

/// An implementation of [FlutterGpuTextureRendererPlatform] that uses method channels.
class MethodChannelFlutterGpuTextureRenderer
    extends FlutterGpuTextureRendererPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('flutter_gpu_texture_renderer');

  @override
  Future<int?> registerTexture() async {
    return await methodChannel.invokeMethod<int?>('registerTexture');
  }

  @override
  Future<int?> unregisterTexture() async {
    return await methodChannel.invokeMethod<int?>('unregisterTexture');
  }

  @override
  Future<int?> output() async {
    return await methodChannel.invokeMethod<int?>('output');
  }

  @override
  Future<int?> device() async {
    return await methodChannel.invokeMethod<int?>('device');
  }
}
