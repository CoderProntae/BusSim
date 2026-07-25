plugins {
    id("com.android.application")
}

android {
    namespace = "com.roadforge.bussim"
    compileSdk = 35
    ndkVersion = "27.2.12479018"

    defaultConfig {
        applicationId = "com.roadforge.bussim"
        minSdk = 29
        targetSdk = 35
        versionCode = 25
        versionName = "0.1.23-faz1-closeout"

        ndk {
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++20", "-fexceptions", "-frtti", "-Wall", "-Wextra", "-Werror")
                arguments += listOf("-DANDROID_STL=c++_static")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}
