# Telefonda APK almak için GitHub Actions workflow'u

Arena'nın GitHub bağlantısı workflow dosyası oluşturma/güncelleme yetkisine sahip değilse otomatik push şu hatayı verir:

```text
refusing to allow a GitHub App to create or update workflow `.github/workflows/android-debug.yml` without `workflows` permission
```

Bu durumda kod branch'e push edilebilir, ancak workflow dosyasını GitHub web arayüzünden kullanıcı elle eklemelidir.

## Telefonda yapılacak manuel adım

1. GitHub uygulamasında veya tarayıcıda repoyu aç: `CoderProntae/BusSim`.
2. Branch olarak `arena/019f95a6-bussim` seç.
3. `Add file` → `Create new file` seç.
4. Dosya adını şu yap:

```text
.github/workflows/android-debug.yml
```

5. Aşağıdaki içeriği aynen yapıştır.
6. Commit mesajı: `Add Android debug APK workflow`
7. Commit hedefi olarak yine `arena/019f95a6-bussim` seç.
8. GitHub Actions sekmesinden `Android Debug APK` workflow'unu çalıştır veya branch'e commit sonrası otomatik başlamasını bekle.

## Workflow içeriği

```yaml
name: Android Debug APK

on:
  push:
    branches: [ main, "arena/**" ]
  pull_request:
  workflow_dispatch:

jobs:
  build-debug-apk:
    name: Build RoadForge Bus Sim debug APK
    runs-on: ubuntu-latest

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Set up JDK 17
        uses: actions/setup-java@v4
        with:
          distribution: temurin
          java-version: "17"

      - name: Set up Android SDK
        uses: android-actions/setup-android@v3

      - name: Install Android packages
        run: |
          yes | sdkmanager --licenses >/dev/null
          sdkmanager \
            "platform-tools" \
            "platforms;android-35" \
            "build-tools;35.0.0" \
            "cmake;3.22.1" \
            "ndk;27.2.12479018"

      - name: Set up Gradle 8.9
        uses: gradle/actions/setup-gradle@v4
        with:
          gradle-version: "8.9"

      - name: Build debug APK
        run: gradle --no-daemon :app:assembleDebug

      - name: Upload debug APK
        uses: actions/upload-artifact@v4
        with:
          name: roadforge-bussim-debug-apk
          path: app/build/outputs/apk/debug/app-debug.apk
          if-no-files-found: error
```
