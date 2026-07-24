# Faz 1.6 — Transform, Quaternion ve Frustum Math Temeli

Bu teslim, 3D sahneye geçiş için math katmanını genişletir ve renderer'ın model transform ile frustum verisini tüketmesini sağlar.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/math/Quat.hpp
app/src/main/cpp/include/roadforge/math/Transform.hpp
app/src/main/cpp/include/roadforge/math/Frustum.hpp
```

## Quat

İlk kapsam:

- Identity quaternion
- Normalize
- Axis-angle quaternion
- Quaternion multiply
- Quaternion → matrix dönüşümü

Araç, kamera ve world object rotasyonları Euler açılarına sıkışmadan bu temel üzerinde ilerleyecek.

## Transform

İlk kapsam:

```text
Transform { position, rotation, scale }
```

ve `transformToMat4` fonksiyonu.

Renderer'daki debug yol plakası artık doğrudan identity matrix ile değil, `Transform` üzerinden model matrix üreterek çiziliyor. Görselde çok hafif bir yaw salınımı var; bu, transform/quaternion yolunun aktif olduğunu doğrulamak için eklendi.

## Frustum

İlk kapsam:

- `Plane`
- `Frustum`
- View-projection matrix'ten frustum çıkarımı
- Sphere/frustum testi

Şimdilik sadece debug yol plakasının görünürlük kontrolünde geniş toleranslı kullanılıyor. Faz 1 içinde gerçek culling/LOD sistemine bağlanacak.

## Telefon kabul testi

1. APK açılır.
2. Perspektif yol plakası görünür.
3. Yol plakası çok hafif canlı/rotasyonlu hissedilebilir; bu zor fark edilebilir ama çökme olmamalı.
4. Alt sağ/alt sol diagnostic renkleri çalışır.
5. Pause/resume çökmez.

## Sonraki teslim

Faz 1.7 hedefi:

1. Renderer buffer/image resource kodunu küçük RAII sınıflarına ayırmak.
2. ECS/world iskeletine giriş.
3. Debug kamera controller hazırlığı.
