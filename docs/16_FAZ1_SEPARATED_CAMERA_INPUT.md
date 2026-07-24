# Faz 1.9c — Kamera Input Ayrımı

Telefon testinde alt sağ / alt sol ile yaklaşma-uzaklaşma yapılırken kare aynı anda dönüyormuş gibi görünüyordu.

## Sebep

İlk debug input mapping'inde yatay konum her dokunmada `steering` üretiyordu. Bu yüzden:

- Alt sağ = throttle + sağ steering
- Alt sol = brake + sol steering

oluyordu. Yani zoom testi yapılırken kamera aynı anda orbit de ediyordu.

## Düzeltme

Engine artık kamera inputu üretirken zoom aktifse steering'i sıfırlar:

```text
Alt sağ / alt sol → sadece zoom
Sol / sağ         → sadece orbit
```

Böylece:

| Input | Yeni davranış |
|---|---|
| Sol taraf basılı | Kamera kare etrafında sola orbit eder |
| Sağ taraf basılı | Kamera kare etrafında sağa orbit eder |
| Alt sağ basılı | Sadece yaklaşır, dönmez |
| Alt sol basılı | Sadece uzaklaşır, dönmez |

## Orbit açıklaması

"Orbit" kameranın objenin etrafında dolaşması demektir. Ekrandan bakınca bu, bazen objenin dönmesi gibi algılanabilir; ama bu debug sürümde dönen şey kare değil, kameranın bakış açısıdır.

## Telefon kabul testi

1. APK açılır.
2. `SOL / SAG / UST / ALT` etiketli kare görünür.
3. Sol veya sağ basılı tutunca perspektif yön değiştirir.
4. Alt sağ basılı tutunca kamera yaklaşır ama kare sağa/sola dönmez.
5. Alt sol basılı tutunca kamera uzaklaşır ama kare sağa/sola dönmez.
6. Diagnostic renkler ve pause/resume çalışmaya devam eder.
