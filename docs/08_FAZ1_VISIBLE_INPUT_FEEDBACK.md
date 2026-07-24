# Faz 1.4b — Görünür Input Debug Geri Bildirimi

Telefon testinde alt sol / alt sağ dokunma farkının anlaşılması zor olduğu görüldü. Önceki sürümde input debug tonu bilinçli olarak çok hafifti; ancak test edilebilirlik için fazla belirsiz kaldı.

## Değişiklik

Input debug renkleri belirginleştirildi ve kısa tıklamalarda bile kaçmaması için son input yaklaşık `0.55` saniye ekranda tutuluyor.

## Yeni görsel mapping

| Dokunma | Beklenen renk |
|---|---|
| Alt sağ | Belirgin yeşil — throttle/gaz |
| Alt sol | Belirgin kırmızı — brake/fren |
| Sol taraf | Belirgin mavi — steering left |
| Sağ taraf | Sıcak turuncu/kahverengi ton — steering right |
| Orta | Morumsu debug tonu |

Dokunma pulse efekti hâlâ var, ancak artık input rengini tamamen bastırmaması için daha düşük yoğunlukta karıştırılıyor.

## Kabul testi

1. APK açılır.
2. Debug yol plakası görünür.
3. Alt sağa kısa tıkla: arka plan yaklaşık yarım saniye yeşil kalır.
4. Alt sola kısa tıkla: arka plan yaklaşık yarım saniye kırmızı kalır.
5. Sol/sağ taraflara tıkla: arka plan mavi/sıcak tonlara döner.
6. Pause/resume çökmez.
