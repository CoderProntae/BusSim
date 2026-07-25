# Faz 1.10f — Debug Kamerayı Sabitleme

Telefon testinde geçici kamera kontrollerinin cihaz yön dönüşümleri nedeniyle güvenilir hissettirmediği görüldü:

- Sol/sağ beklenen yatay hareket yerine farklı eksende algılanabiliyordu.
- Yakınlaşma/uzaklaşma hâlâ kareyi ana karakter/odak gibi hissettiriyordu.

Bu noktada debug kamera kontrolü üzerinde daha fazla zaman harcamamak için geçici hareket kontrolleri devre dışı bırakıldı.

## Yeni karar

Şimdilik kamera sabit kalır.

| Input | Yeni davranış |
|---|---|
| Sol / sağ | Sadece diagnostic renk verir, kamera hareket etmez |
| Alt sağ / alt sol | Sadece diagnostic renk verir, kamera zoom yapmaz |

Bu, Faz 1'in amacına daha uygundur: renderer, world, mesh ve input hattını doğrulamak. Gerçek sürüş kamerası Faz 2'de araç kontrol modeliyle birlikte ayrı tasarlanacak.

## Overlay durumu

Debug overlay hâlâ kapalıdır. Ayrı 2D overlay/UI pipeline gelince yeniden eklenecek.

## Kabul testi

1. APK açılır.
2. `SOL / SAG / UST / ALT` etiketli kare görünür.
3. Sol/sağ/alt inputlarda arka plan diagnostic renkleri çalışır.
4. Kamera/kare sağa-sola dönmez, yukarı-aşağı kaymaz, zoom yapmaz.
5. Pause/resume çökmez.
