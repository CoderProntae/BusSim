# Faz 1.4c — Input Diagnostic Renkleri

Kullanıcı testinde alt sol / alt sağ input farkı hâlâ yeterince anlaşılır değildi. Bu nedenle input debug geri bildirimi artık oyun estetiği gibi değil, doğrudan teşhis ekranı gibi davranır.

## Yeni davranış

Her dokunma, ekran arka planını yaklaşık `1.25` saniye boyunca parlak bir renge çevirir:

| Dokunma bölgesi | Anlam | Ekran rengi |
|---|---|---|
| Alt sağ | Gaz / throttle | Parlak yeşil |
| Alt sol | Fren / brake | Parlak kırmızı |
| Sol taraf | Direksiyon sol | Parlak mavi |
| Sağ taraf | Direksiyon sağ | Parlak sarı/turuncu |
| Orta | Genel dokunma | Mor |

Bu renkler nihai oyun UI'ı değildir. Amaç sadece Android touch → JNI → C++ InputSystem → renderer hattının telefonda gözle kaçmayacak şekilde doğrulanmasıdır.

## Test

1. APK'yı aç.
2. Alt sağa bir kez kısa dokun: ekran yeşil kalmalı.
3. Alt sola bir kez kısa dokun: ekran kırmızı kalmalı.
4. Sol tarafa dokun: ekran mavi kalmalı.
5. Sağ tarafa dokun: ekran sarı/turuncu kalmalı.
6. Her renk yaklaşık 1 saniyeden biraz fazla görünmelidir.
