# Laboratorium: Programowanie Aplikacji w Chmurze Obliczeniowej

## Zadanie 2 – Budowanie i publikacja obrazu kontenera w GitHub Actions

### Cel zadania

Celem było stworzenie łańcucha CI/CD w usłudze **GitHub Actions**, który:

* Buduje obraz kontenera na podstawie aplikacji z zadania nr 1
* Wspiera architektury `linux/amd64` oraz `linux/arm64`
* Wykorzystuje **cache z DockerHub**
* Wykonuje **lokalne skanowanie CVE** z użyciem Trivy
* Publikuje obraz tylko wtedy, gdy **nie zawiera zagrożeń CRITICAL lub HIGH**
* Przesyła obraz do publicznego repozytorium kontenerów GitHub (ghcr.io)

---

### Technologie

* Docker & Docker Buildx
* GitHub Actions
* GitHub Container Registry (GHCR)
* DockerHub (cache)
* Trivy (skanowanie podatności CVE)

---

###  Plik workflow

Plik `.github/workflows/docker-publish.yml` definiuje automatyczny łańcuch budowania i skanowania obrazu.

Główne etapy:

1. **Checkout kodu**
2. **Konfiguracja Buildx**
3. **Logowanie do GHCR i DockerHub**
4. **Budowanie obrazu tymczasowego (`localbuild:test`) dla skanu Trivy**

   * opcja `load: true` umożliwia późniejsze zeskanowanie lokalne
5. **Skanowanie lokalnego obrazu Trivy**

   * tylko jeśli nie wykryto podatności `CRITICAL` lub `HIGH`, pipeline przechodzi dalej
6. **Budowanie i wypchnięcie finalnego obrazu multi-arch**

---

### Skanowanie CVE – Trivy

Zastosowano **Trivy w wersji 0.11.2**, w trybie lokalnym (`image-ref: localbuild:test`). Jest to proste rozwiązanie, które:

* Nie wymaga wcześniejszego push obrazu
* Skutecznie wykrywa krytyczne i wysokie podatności
* Pozwala **zatrzymać pipeline**, jeśli takie zostaną wykryte

Obraz nie jest publikowany, jeśli nie przejdzie skanowania.

---

### Cache – DockerHub

Użyto mechanizmu cache BuildKit opartego o DockerHub (`derev/lab:buildcache`) w trybie:

* `cache-from`: umożliwia ponowne użycie warstw z poprzednich buildów
* `cache-to`: zapisuje cache do publicznego repozytorium
* `mode=max`: zapisuje maksymalnie dużo danych (warstwy + metadane)

Rozwiązanie to przyspiesza budowanie i jest kompatybilne z Buildx.

Źródło: [Docker docs – Remote cache](https://docs.docker.com/build/cache/backends/registry/)

---

### Tagowanie obrazów

Obrazy oznaczane są za pomocą:

* `latest` – najnowsza wersja z gałęzi `main`
* `ghcr.io/użytkownik/repo:commit-sha` – unikalny tag commit-a


* `latest` – ułatwia używanie zawsze aktualnej wersji
* `sha` – umożliwia śledzenie konkretnych wersji buildów

Źródło: [Docker best practices – Tags](https://docs.docker.com/engine/reference/commandline/tag/)

---

### Potwierdzenie działania

Łańcuch został uruchomiony w zakładce **Actions** i zakończył się sukcesem. Obraz został wypchnięty do:

```
ghcr.io/mykola-derevianko/cloud-zad2
```
