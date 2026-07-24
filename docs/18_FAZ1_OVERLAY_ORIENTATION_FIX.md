# Faz 1.10b — Debug Overlay Yön/Konum Düzeltmesi

Telefon testinde debug overlay panelinin beklenen sol üst yerine sağ altta ve ters göründüğü doğrulandı.

## Sebep

Overlay verisi world/debug mesh ile aynı basit pipeline'dan geçiyordu. Bu pipeline'ın Vulkan viewport/projection yönü nedeniyle NDC overlay koordinatları cihazda X/Y eksenlerinde ters görünüyordu.

## Düzeltme

Overlay quad üretim fonksiyonu artık mantıksal ekran koordinatlarını kullanıyor:

```text
negative X = sol
positive Y = üst
```

Ardından bu koordinatları renderer'ın mevcut pipeline yönüne çeviriyor. Böylece panelin cihazda sol üstte ve düz okunması hedefleniyor.

## Telefon kabul testi

1. APK açılır.
2. `SOL / SAG / UST / ALT` etiketli kare görünür.
3. Debug overlay paneli sol üstte görünür.
4. `FPS`, `MS`, `SIM`, `DRP` yazıları ters değil, düz okunur.
5. Diagnostic input renkleri çalışır.
6. Pause/resume çökmez.
