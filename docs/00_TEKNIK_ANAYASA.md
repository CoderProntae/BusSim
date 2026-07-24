# RoadForge Bus Sim — Teknik Anayasa

Bu belge motor ve oyun kodu için bağlayıcı mimari kararları toplar.

## Kesin kararlar

| Başlık | Karar |
|---|---|
| Motor stratejisi | Özel C++ motor |
| İlk platform | Android |
| İlk grafik API | Vulkan 1.1 |
| İlk ürün kapsamı | Mobil MVP |
| Performans hedefi | Üst segment Android cihazlarda 60 FPS |
| Dış bağımlılık | Az ve denetlenmiş middleware; mimari bizim |
| Bağlantı/kayıt | Tam offline, yerel kayıt |
| WebView/HTML | Kullanılmayacak |

## Teknik varsayımlar

- Android API 29+ / Android 10+
- İlk ABI: `arm64-v8a`
- C++20
- Vulkan 1.1 zorunlu
- İlk release: tek oyunculu, yerel kayıt; cloud/multiplayer yok
- Profil cihaz sınıfı: Snapdragon 8 Gen 1 / Dimensity 9000 benzeri üst segment
- İlk bellek hedefi: `<= 1.2 GB RSS`, sahne GPU kaynağı `<= 650 MB` — ölçümle revize edilir

## Modül sınırları

```text
Android Platform / JNI
        ↓
Engine Host
        ↓
Core | Platform | Renderer | Asset I/O
        ↓
Input | World/ECS | Physics | Audio
        ↓
Vehicle | Traffic | Passenger | Economy | Weather
        ↓
HUD | GPS | Menü | Şirket UI
```

## Kurallar

1. Oyun kodu Vulkan ayrıntılarını doğrudan görmez; renderer'a render proxy/scene verisi sağlar.
2. UI, oyun nesnelerine doğrudan yazmaz; `Command` üretir. Örnek: `SetThrottle`, `ChooseRoute`, `SetTicketPrice`.
3. Ekonomi sistemi fizik objelerine bağlı olmaz; yakıt, hasar ve sefer sonucu gibi saf event'leri dinler.
4. Android Java katmanı oyun kuralı içermez.
5. Fizik/sürüş sabit 60 Hz tick ile çalışır; render değişken hızda ve interpolasyonludur.
6. Rastgelelik tohumlanır; ekonomi/yolcu/AI tekrar üretilebilir olmalıdır.
7. Her faz telefonda APK ile test edilebilir küçük teslimlere bölünür.

## Middleware ilkesi

Henüz üçüncü taraf native kütüphane eklenmedi. İleride değerlendirilecek küçük bağımlılıklar:

- Jolt Physics (MIT): rigid body / çarpışma temeli
- cgltf (MIT): geliştirme aşamasında glTF yükleme
- KTX-Software / Basis Universal (Apache-2.0): mobil texture pipeline
- miniaudio (MIT): prototip ses

Araç fiziği hiçbir fizik kütüphanesine sıkı bağlanmayacak; `IPhysicsWorld` ve `IVehiclePhysicsBackend` adapter sınırı korunacaktır.
