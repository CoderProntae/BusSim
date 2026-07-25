# Faz 1.10c — Overlay Eksen Dönüşümü Düzeltmesi

Telefon testinde panel sol üstteydi fakat yazılar/barlar hâlâ ters/yanlış yöndeydi. Ekran görüntüsü overlay'in yalnızca X/Y tersliği değil, aynı zamanda 90 derece eksen dönüşümü yaşadığını gösterdi.

## Düzeltme

Overlay quad üretiminde mantıksal ekran koordinatları artık cihaza gönderilmeden önce şu dönüşümden geçiyor:

```text
logical screen: negative X = sol, positive Y = üst
submitted NDC : x = -logicalY, y = logicalX
```

Bu dönüşüm, mevcut Android/Vulkan yüzey dönüşümüyle birleştiğinde overlay'in cihazda düz ve beklenen eksende görünmesini hedefler.

## Kabul testi

1. Overlay sol üstte kalmalı.
2. `FPS`, `MS`, `SIM`, `DRP` düz okunmalı.
3. Barlar satırların yanında yatay görünmeli; dikey kolon gibi görünmemeli.
4. Kare diagnostic mesh ve input renkleri değişmeden çalışmalı.
