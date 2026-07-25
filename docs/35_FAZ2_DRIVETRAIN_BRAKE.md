# Faz 2.8 — Motor / Şanzıman / Fren Modeli Başlangıcı

Bu teslim, kinematik sürüş prototipini daha otobüs benzeri hissettirmek için ilk drivetrain/fren/yakıt/hasar sinyallerini ekler.

## Eklenen VehicleState alanları

```text
engineTorqueNm
driveForce
brakeForce
odometerMeters
tripSeconds
fuelLiters
fuelCapacityLiters
damage01
offRoad01
```

## İlk otomatik vites modeli

`VehicleController` artık hıza göre basit otomatik vites seçer:

```text
1..6 ileri vites
0 neutral/park
-1 reverse
```

Vites, şimdilik gerçek şanzıman oranlarıyla kuvvet üretmiyor; fakat hızlanma ölçeğini ve RPM preview değerini etkiliyor.

## İlk motor/fren modeli

Eklenen basit modeller:

- Throttle → drive acceleration
- Brake → brake deceleration
- Retarder alanı hazır
- Rolling drag
- Aero drag
- Speed clamp
- Engine RPM clamp
- Steering wheel degree preview

Bu hâlâ final fizik değil; raycast süspansiyon kuvvetleri Faz 2.9+ içinde entegre edilecek.

## Yol grip / offroad sinyali

Kinematik sürüş artık aracın yaklaşık yol üzerinde olup olmadığını kontrol eder:

```text
main road
cross road
terminal pad
```

Yol dışındayken grip düşer ve yüksek hızda çok küçük `damage01` artışı başlar.

## Yakıt / odometre / süre

İlk telemetry:

- Odometer metre
- Trip seconds
- Fuel liters
- Damage 0..1

Bu değerler henüz UI'da görünmez, ancak Faz 3/Faz 4 ekonomi ve sefer sistemine temel oluşturur.

## Kullanıcı testi

Bu adım sürüş hissi testi gerektirir.

1. APK açılır.
2. Otobüs gazla daha ağır/hız sınırlı şekilde hızlanır.
3. Fren daha belirgin yavaşlatır.
4. Uzun sürüşte kamera takip eder.
5. Yol dışına çıkınca tutuş hissi düşebilir.
6. Çoklu dokunma gaz + direksiyon çalışır.
7. Pause/resume çökmez.

## Sonraki teslim

Faz 2.9 hedefi:

1. Collision/yakıt/hasar sinyallerini event/telemetry modeline ayırmak.
2. Sürüş state'ini Faz 3 ekonomi/sefer event'lerine hazırlanacak hale getirmek.
3. Faz 2.10 stabilizasyon testlerine hazırlık.
