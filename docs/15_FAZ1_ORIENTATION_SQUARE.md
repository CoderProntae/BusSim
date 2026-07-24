# Faz 1.9b — Yön Etiketli Orientation Square

Kamera kontrol testinde yol plakasının hangi yöne baktığı anlaşılmadığı için debug mesh daha okunabilir bir teşhis objesine dönüştürüldü.

## Değişiklik

Önceki koyu yol plakası yerine 6x6 boyutunda renkli ve etiketli bir kare çiziliyor.

Kare bölgeleri:

| Bölge | Etiket | Renk |
|---|---|---|
| Sol | `SOL` | Mavi |
| Sağ | `SAG` | Sarı/turuncu |
| Üst / uzak taraf | `UST` | Mor |
| Alt / yakın taraf | `ALT` | Yeşil |
| Orta | - | Koyu gri |

Not: Şimdilik Türkçe karakterli gerçek font yok; bu yüzden `SAĞ` yerine `SAG`, `ÜST` yerine `UST` yazılıyor. Harfler bitmap/font texture ile değil, küçük vertex quad bloklarıyla çiziliyor.

## Kamera kontrol netliği

World tarafındaki debug kare artık kendi kendine yaw/rotasyon yapmıyor. Eğer SOL/SAĞ basılı tutunca perspektif değişiyorsa bu artık karenin dönmesi değil, kameranın kare etrafında orbit etmesidir.

Geçici kontrol:

| Input | Etki |
|---|---|
| Sol taraf basılı | Kamera sola orbit eder |
| Sağ taraf basılı | Kamera sağa orbit eder |
| Alt sağ basılı | Kamera yaklaşır |
| Alt sol basılı | Kamera uzaklaşır |

Diagnostic arka plan renkleri hâlâ aktif:

- Alt sağ: yeşil
- Alt sol: kırmızı
- Sol: mavi
- Sağ: sarı/turuncu

## Telefon kabul testi

1. APK açılır.
2. Ekranda etiketli kare görünür: `SOL`, `SAG`, `UST`, `ALT`.
3. Sol/sağ basılı tutunca karedeki etiketlerden kamera yönünün değiştiği anlaşılır.
4. Alt sağ/alt sol basılı tutunca kamera yaklaşır/uzaklaşır.
5. Pause/resume çökmez.

## Sonraki teslim

Faz 1.10 hedefi aynı kalıyor:

1. Debug overlay / frame stats görsel yüzeyi.
2. Ardından placeholder otobüs + yol debug sahnesi.
