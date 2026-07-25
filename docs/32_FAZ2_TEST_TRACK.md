# Faz 2.5 — Basit Test Pisti / Yol Segmentleri

Bu teslim, sonsuz/tek düz yol hissinden çıkıp ilk daha uzun debug test pistini ekler.

## Eklenen görsel pist parçaları

Debug road mesh artık şunları içerir:

- Uzun ana yol
- Ana yol kenar çizgileri
- Kesik orta şerit çizgileri
- Yatay cross-road segmenti
- Sağ tarafta terminal/servis pad alanı
- Parking guide çizgileri
- Kırmızı/sarı düşük curb/marker blokları
- Hareket algısını kolaylaştıran tekrar eden yol işaretleri

Bu hâlâ gerçek yol sistemi değildir; fakat Faz 2 kinematik sürüşü daha anlaşılır test etmek için görsel pist scaffold'udur.

## Teknik not

Pist hâlâ `DebugMeshAssets.cpp` içinde procedural debug mesh olarak üretilir. Collision, şerit graph ve rota verisi henüz yoktur. Faz 2.6+ içinde fizik adapter ve yol verisi ayrışacak.

## Kullanıcı testi

Bu adım görsel/sürüş test gerektirir.

1. APK açılır.
2. Uzun ana yol, kesik şeritler ve terminal/servis pad görünür.
3. Otobüs gaz ile ileri gider.
4. Çoklu dokunmayla gaz + direksiyon yapılabilir.
5. Yol üzerindeki curb/marker blokları hareketi algılamayı kolaylaştırır.
6. Sağ üst kamera toggle çalışmaya devam eder.
7. Pause/resume çökmez.

## Sonraki teslim

Faz 2.6 hedefi:

1. `IPhysicsWorld` ve `IVehiclePhysicsBackend` adapter arayüzleri.
2. Kinematik prototipi fizik backend'den bağımsız tutmak.
3. Jolt/özel raycast süspansiyon değerlendirmesine hazırlık.
