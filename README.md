# NodeMCU-ESP8266-Captive-Portal-&-DNS-Spoofing
Tento mini projekt slúži ako testovanie známeho útoku captive portal na Wi-Fi. Zariadenie ESP8266 slúži ako AP, ktoré vysiela AP a v prípade, že sa pripojíme vvyzve nás na registráciu pre používanie internetu. 

## Využitý hardware
- `NodeMCU ESP8266 with 0.96 OLED Display`
- `Prepojovacie káble samec-samec`
- `1x Zelená Led, 1x Červená Led, 1x Modrá Led`
- `3x 1k ohm rezistor`
- `Breadboard`
- `Powerbanka`

## Program
- Vytvára otvorenú WiFi prístupovú sieť s pevnou IP adresou.
- Spúšťa DNS server, ktorý presmeruje všetky požiadavky na zariadenie.
- Hostí jednoduchý webový server s prihlasovacím formulárom pre registráciu používateľov.
- Ukladá zadaný email a heslo, sleduje celkový počet pokusov o pripojenie a úspešných registrácií.
- Používa tri LED diódy pre indikáciu stavu:
  - Modrá LED stále bliká, signalizuje aktívny AP režim.
  - Zelená LED bliká po úspešnej registrácii.
  - Červená LED bliká pri chybe.
- Na OLED displeji zobrazuje štatistiky pripojení a IP adresu.
- Stránka so zoznamom uložených údajov je chránená základným HTTP overením.

## Nahratie kódu do ESP8266
- Ako prvé je potrebne v Arduino IDE exportovať compilovany binárny súbor: `Sketch/Export Compiled Binary`. Vznikne nám súbor s koncovkou `.bin`.
- Následne je potrebné nainštalovať ESP Flasher: `https://github.com/nodemcu/nodemcu-flasher`.
- Po spustení flasheru nastavíme v sekci `Config` cestu kde máme `.bin` kód uloženy a adresu necháme na `0x00000`.
- V sekcií `Advanced` nastavíme `Baudrate - 115200`, `Flash size - 4MByte`, `Flash speed - 40MHt` a `SPI Mode - DIO`.

<p align="center">
  <img src="Pictures/Advanced.png" alt="Obrázok 1 Advanced" width="800"/>
  <br>
  <i>Obrázok 1 NodeMCU Flasher - Advanced</i>
</p>

- Následne pripojíme NodeMCU ESP8266 do PC a prejdeme vo Flasheri do sekcie `Operation`, kde zvolíme `COM Port` a spustíme `Flash`.

## Zapojenie
- Zapojíme NodeMCU ESP8266 do Breadboardu. Čevená led je zapojená do `D1`, zelená led do `D2` a modrá led do `D7`, pričom každá led využíva aj 1k ohm rezistor.

<p align="center">
  <img src="Pictures/Zapojenie.jpg" alt="Obrázok 2 Zapojenie a spustenie" width="800"/>
  <br>
  <i>Obrázok 2 Zapojenie a spustenie</i>
</p>


