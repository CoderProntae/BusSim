# RoadForge Production Asset Direction

Bu belge, oyunun blok/debug görünümden çıkıp ETS/otobüs simülasyonu tarzı mobil kaliteye ilerlemesi için asset ve PBR yönünü sabitler.

## Hedef kalite

Hedef: PC ETS seviyesini birebir kopyalamak değil; mobilde çalışacak şekilde ETS/Bus Simulator hissine yaklaşan kaliteli, optimize ve ölçeklenebilir üretim hattı.

## Ana görsel kurallar

- Blok model yalnızca prototipte kalır.
- Final otobüs assetleri rounded body, cam şeritleri, far/tampon/kapı/panel detayları ve LOD varyantlarıyla gelir.
- Yol assetleri yalnızca düz plane değildir; segment, banket, işaretleme, bariyer, terminal pad ve collision verisi ayrılır.
- Her asset PBR material id ile eşleşir.
- Texture hedef formatı mobilde KTX2/Basis'tir.
- Her görsel asset için LOD ve collision proxy zorunludur.

## İlk asset seti

```text
assets/materials/roadforge_pbr_materials.json
assets/vehicles/intercity_bus_v1.asset.json
assets/environments/intercity_test_corridor_v1.asset.json
```

Bunlar runtime loader tarafından henüz okunmuyor; ancak üretim formatının ilk şemasıdır. Sonraki adımlarda loader ve pipeline'a bağlanacak.

## Otobüs V1 hedefi

- 12.4m şehirlerarası otobüs
- Turuncu ana gövde + koyu trim
- Mavi cam tint
- 4 teker placeholder'dan silindir/LOD mesh'e geçiş
- Headlight/brake light emissive materyal
- Physics proxy: box + wheel specs

## Yol/çevre V1 hedefi

- Ana straight corridor
- Cross/service road
- Terminal/parking pad
- Lane markings
- Curb/marker props
- İleride lane graph ve route hooks
