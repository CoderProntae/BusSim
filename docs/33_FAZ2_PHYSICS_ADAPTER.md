# Faz 2.6 — Fizik Backend Adapter Tasarımı

Bu teslim, araç fiziğini ileride Jolt veya özel raycast backend'e bağlamadan önce motor tarafındaki soyut fizik sınırlarını kurar.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/physics/PhysicsTypes.hpp
app/src/main/cpp/include/roadforge/physics/IPhysicsWorld.hpp
app/src/main/cpp/include/roadforge/physics/IVehiclePhysicsBackend.hpp
app/src/main/cpp/include/roadforge/physics/NullPhysicsWorld.hpp
app/src/main/cpp/src/physics/NullPhysicsWorld.cpp
```

## IPhysicsWorld

Temel dünya fiziği arayüzü:

```text
reset
step
createRigidBody
destroyRigidBody
isAlive
setTransform
getTransform
raycast
```

Oyun kodu ileride doğrudan Jolt veya başka bir fizik kütüphanesine bağlanmayacak; bu interface üzerinden çalışacak.

## NullPhysicsWorld

Middleware gerektirmeyen deterministik test backend'i eklendi.

Özellikleri:

- Body handle / generation sistemi
- Transform saklama
- No-op step
- Basit `y=0` ground-plane raycast

Bu backend, raycast teker/süspansiyon prototipine başlamadan önce compile-time ve mimari sınırı doğrulamak için yeterli.

## IVehiclePhysicsBackend

Araç fiziği için ayrı adapter sınırı eklendi:

```text
VehiclePhysicsConfig
WheelSpec
VehiclePhysicsTelemetry
IVehiclePhysicsBackend
```

Amaç:

- Vehicle gameplay komut/state sahibi olur.
- Fizik backend temas, süspansiyon, rigid body ve entegrasyon detaylarını yönetir.
- Jolt veya özel çözüm değiştirilebilir kalır.

## Kullanıcı testi gerekli mi?

Hayır. Bu adım görsel değişiklik üretmez. CI build başarısı yeterlidir.

## Sonraki teslim

Faz 2.7 hedefi:

1. Teker konumları ve raycast süspansiyon taslağı.
2. `NullPhysicsWorld` ground raycast ile ilk wheel contact telemetry.
3. Otobüsün kinematik state'ine suspension/grounded wheel debug sinyallerini bağlamak.
