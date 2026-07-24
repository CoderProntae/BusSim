# Faz 1.1 — Core Clock ve Math Temeli

Faz 0 gerçek Android cihazda doğrulandıktan sonra Faz 1'in ilk küçük teslimi olarak motor çekirdeğine iki temel parça eklendi.

## Eklenenler

### `core::SimulationClock`

Dosyalar:

```text
app/src/main/cpp/include/roadforge/core/SimulationClock.hpp
app/src/main/cpp/src/core/SimulationClock.cpp
```

Görevi:

- Değişken render frame delta değerini sabit 60 Hz simulation ticklerine çevirir.
- Her frame kaç adet fixed update çalışması gerektiğini hesaplar.
- Render interpolation alpha üretir.
- Uzun takılmalarda spiral-of-death riskini azaltmak için frame başına maksimum simulation step sınırı uygular.

Bu yapı ileride şu sistemlerin ortak zamanı olacak:

- Araç fiziği
- Trafik AI
- Yolcu/sefer state machine
- Ekonomi event işleme
- Input recording / replay

### `math::Vec`

Dosya:

```text
app/src/main/cpp/include/roadforge/math/Vec.hpp
```

İlk kapsam:

- `Vec2`
- `Vec3`
- `Vec4`
- `lerp`
- `length`
- `normalize`

Bu aşamada bilinçli olarak küçük tutuldu. Matrix, quaternion, frustum ve transform tipleri 3D debug sahneye geçerken eklenecek.

### Renderer timing bağlantısı

`Engine`, her Choreographer frame'inde:

```text
Android frameTimeNanos
→ deltaSeconds
→ SimulationClock.advance(deltaSeconds)
→ fixed 60 Hz tick sayacı
→ renderer.setSimulationTiming(...)
→ Vulkan present
```

Renderer bu veriyi şimdilik yalnızca görsel doğrulama için kullanır: lacivert clear color üzerinde çok hafif pulse/breathe animasyonu vardır. Dokunma hâlâ turuncu tepki üretir.

## Telefon kabul testi

APK kurulduğunda beklenenler:

1. Uygulama açılır.
2. Lacivert Vulkan ekran görünür.
3. Arka planda çok hafif renk canlılığı/pulse fark edilir.
4. Dokununca turuncu tepki gelir.
5. Pause/resume çökmez.

## Sonraki küçük teslim

Faz 1.2 için önerilen sıradaki parça:

1. Native input action/axis sistemi.
2. Debug frame stats toplama: CPU frame time, fixed tick count, dropped time count.
3. Basit shader pipeline hazırlığı.
4. İlk üçgen/quad çizimi.
