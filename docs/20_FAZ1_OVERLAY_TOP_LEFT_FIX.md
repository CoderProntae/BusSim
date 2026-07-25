# Faz 1.10d — Overlay Sol-Üst Yerleşim Düzeltmesi

Telefon ekran görüntüsü, eksen dönüşümü sonrası overlay yazılarının/barlarının daha okunabilir olduğunu fakat panelin fiziksel olarak alt-sol bölgede kaldığını gösterdi.

## Sebep

Overlay quad üretimindeki `x = -logicalY, y = logicalX` dönüşümü metin/bar yönünü düzeltti; ancak panelin fiziksel dikey konumu artık mantıksal X koordinatından etkileniyordu. Önceki koordinatlar negatif X kullandığı için panel alt tarafa düşüyordu.

## Düzeltme

Overlay paneli için mantıksal X koordinatları pozitif bölgeye taşındı. Mevcut yüzey dönüşümüyle birleşince panelin fiziksel sol-üstte görünmesi hedeflenir.

## Kabul testi

1. Overlay fiziksel sol-üst köşede görünmeli.
2. `FPS`, `MS`, `SIM`, `DRP` okunabilir olmalı.
3. Barlar yatay görünmeli.
4. Diagnostic kare ve input renkleri değişmeden çalışmalı.
