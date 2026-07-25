# Faz 2.4 — Takip / Kabin Kamera Prototipi

Bu teslim, sürüş vertical slice için ilk kamera modlarını ekler.

## Kamera modları

World artık iki kamera modunu destekler:

```text
Follow
Cabin
```

### Follow

Otobüsün arkasından takip eden basit kamera.

```text
eye    = bus position - forward * distance + height
target = bus position + forward * lead
```

### Cabin

Otobüsün içinde/sürücü konumuna yakın ilk kabin kamera prototipi.

```text
eye    = bus position + forward * small offset - right * driver offset + cabin height
target = eye + forward * look distance
```

Bu henüz gerçek kabin modeli değildir. Sadece kamera davranışının ilk prototipidir.

## Kamera değiştirme

Geçici Faz 2.4 kontrolü:

| Bölge | Davranış |
|---|---|
| Sağ üst köşe | Follow/Cabin kamera toggle |

Sağ üst köşe araç komutu üretmez. Sadece kamera modunu değiştirir.

## Input notu

Sürüş inputları Faz 2.3 ile aynı kalır:

| Bölge | Davranış |
|---|---|
| Sol %50 | Direksiyon |
| Sağ alt %25 | Gaz |
| Orta-sağ alt %25 | Fren |

## Telefon kabul testi

Bu adım görsel test gerektirir.

1. APK açılır.
2. Otobüs ve yol görünür.
3. Sağ alt + sol direksiyon ile otobüs sürülebilir.
4. Sağ üst köşeye kısa dokun: kamera kabin moduna geçmelidir.
5. Sağ üst köşeye tekrar dokun: takip kameraya dönmelidir.
6. Kabin modunda otobüsün içinden/yakınından ileri bakma hissi oluşmalıdır.
7. Pause/resume çökmez.

## Sonraki teslim

Faz 2.5 hedefi:

1. Basit test pisti / yol segmentleri.
2. Otobüsün sonsuz düz yoldan çıkmadan küçük pistte sürülebilmesi.
3. Şerit/yol referanslarının iyileştirilmesi.
