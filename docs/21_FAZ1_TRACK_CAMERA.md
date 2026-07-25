# Faz 1.10e — Debug Kamerayı Orbit Yerine Track/Dolly Yapma

Telefon testinde sol/sağ basılı tutunca obje/kare dönüyormuş gibi görünüyordu. Bu, debug kameranın orbit mantığıyla objenin etrafında dönmesinden kaynaklanıyordu. Kullanıcı açısından bu "konumum değişmiyor, küp dönüyor" hissi verdi.

## Değişiklik

Debug kamera artık orbit yapmaz.

Yeni geçici kontrol:

| Input | Davranış |
|---|---|
| Sol taraf basılı | Kamera ve hedef birlikte sola kayar; kare dönmez |
| Sağ taraf basılı | Kamera ve hedef birlikte sağa kayar; kare dönmez |
| Alt sağ basılı | Kamera yaklaşır; sağ/sol dönme yok |
| Alt sol basılı | Kamera uzaklaşır; sağ/sol dönme yok |

Teknik olarak `cameraOrbitYawRadians` yerine `cameraLateralOffset` kullanılıyor. Kamera `eye.x` ve `target.x` birlikte hareket eder, bu yüzden bakış açısı dönmez.

## Overlay kararı

Debug overlay paneli geçici olarak kapatıldı. Sebep: mevcut 3D pipeline ile screen-space UI çizmek Android surface transformlarında gereksiz zaman kaybettirdi. Overlay daha sonra ayrı bir 2D UI/overlay pipeline ile geri gelecek.

## Kabul testi

1. APK açılır.
2. Yön etiketli kare görünür.
3. Sol/sağ basılı tutunca kare dönmek yerine ekranda yatay kayma hissi vermeli.
4. Alt sağ/alt sol sadece yaklaşma/uzaklaşma yapmalı.
5. Sol üst/alt panel görünmemeli.
6. Diagnostic renkler çalışmalı.
7. Pause/resume çökmez.
