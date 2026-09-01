# LUFT

LUFT is an ESP32-S3 air-quality monitoring prototype created by Vinícius Ferreira. It reads environmental sensors, exposes the latest measurements through a JSON endpoint, and hosts two responsive web pages directly from the ESP32:

- a live dashboard for the current readings; and
- an educational page about air pollution, health, and practical ways to reduce exposure.

The device creates its own Wi-Fi access point, so the dashboard can be used locally without an internet connection or an external web server.

## Desafio Liga Jovem

LUFT was conceived as a proposed project for the **Desafio Liga Jovem** (Liga Jovem Challenge).

Desafio Liga Jovem is a free national student entrepreneurship competition promoted by Sebrae. It challenges teams to use technology to create solutions for real problems in their schools or communities. The official challenge material treats technology broadly—it may include websites, applications, games, products, services, and other practical methods—and emphasizes understanding the target audience, creativity, community impact, financial sustainability, and a working prototype.

LUFT responds to that mission by turning otherwise invisible local air-quality conditions into accessible information. Its self-hosted interface makes the prototype portable and usable in places where a conventional internet connection may not be available. The educational page also connects the measurements to the broader community problem: understanding pollution and making better-informed decisions about exposure.

This repository describes LUFT's connection to the challenge but does not claim that the project was selected, awarded, or officially endorsed by Sebrae.

Further information:

- [Official Desafio Liga Jovem website](https://www.desafioligajovem.com.br/)
- [Sebrae: Desafio Liga Jovem and its community-focused purpose](https://agenciasebrae.com.br/cultura-empreendedora/maior-competicao-de-empreendedorismo-estudantil-do-pais-tem-inscricoes-prorrogadas/)
- [Official second-edition regulations (PDF)](https://rn.agenciasebrae.com.br/wp-content/uploads/sites/20/2024/06/DLJ2_Regulamento_13_03.pdf)

## How it works

```text
BME680 ──> Bosch BSEC2 ──> eCO₂ and breath-VOC estimates ──┐
                                                            ├──> ESP32 web server ──> browser
Optical dust sensor ──> analog sampling ──> particle value ─┘           │
                                                                        └──> JSON /data

Latest readings ──> prototype air-quality logic ──> Nextion display
```

The web pages and stylesheet are stored in program memory and compiled into the firmware. No separate filesystem upload is required.

## Hardware

The current firmware is configured for:

| Component | Purpose | Default connection |
| --- | --- | --- |
| 4D Systems ESP32-S3 Gen4 | Main controller and Wi-Fi web server | — |
| BME680 | Gas and environmental sensing | SDA: GPIO 12, SCL: GPIO 11, address `0x76` |
| Optical dust sensor | Particle-density prototype | Analog signal: GPIO 5, LED control: GPIO 6 |
| Nextion display | Local status display | ESP RX: GPIO 44, ESP TX: GPIO 43, 9600 baud |

These pin assignments are constants near the beginning of [`main.cpp`](firmware/src/main.cpp) and should be adjusted if the wiring changes.

## Web interface

After startup, the ESP32 creates the open Wi-Fi network `LUFT-AP`. Connect to it and open the IP address printed in the serial monitor. With the default ESP32 SoftAP configuration, this is normally [http://192.168.4.1](http://192.168.4.1).

| URL | Content |
| --- | --- |
| `/` | Live sensor dashboard |
| `/sophia` | Air-quality information page |
| `/sophia/` | Alternate URL for the information page |
| `/sophia/index.html` | File-style URL for the information page |
| `/sophia/styles.css` | Information-page stylesheet |
| `/data` | Latest readings as JSON |

The page-switch buttons move between the dashboard and information page. The dashboard requests `/data` every two seconds.

Example response:

```json
{
  "co2": 612.4,
  "bvoc": 0.48,
  "pm": 18.7
}
```

## Building and uploading

The firmware is an Arduino project managed with [PlatformIO](https://platformio.org/).

1. Install PlatformIO Core or the PlatformIO extension for Visual Studio Code.
2. Connect the ESP32-S3 over USB.
3. Open a terminal in the `firmware` directory.
4. Build and upload the firmware:

   ```bash
   pio run
   pio run --target upload
   ```

5. Open the serial monitor to see startup information and the access-point IP address:

   ```bash
   pio device monitor --baud 115200
   ```

PlatformIO downloads the sensor libraries declared in [`platformio.ini`](firmware/platformio.ini), including Bosch BSEC2 and the BME68x sensor library.

## Project structure

```text
LUFT/
├── .gitignore
├── LICENSE
├── README.md
└── firmware/
    ├── include/
    │   └── web_pages.h     # Dashboard, educational page, and CSS
    ├── src/
    │   └── main.cpp        # Sensors, Wi-Fi, routes, dashboard, and Nextion logic
    └── platformio.ini      # Board and library configuration
```

## Prototype limitations

LUFT is an educational prototype, not a certified environmental or medical instrument.

- The `co2` field is Bosch BSEC's **CO₂-equivalent estimate** from the BME680, not a direct CO₂ measurement from an NDIR sensor.
- The `bvoc` field is BSEC's breath-VOC-equivalent estimate.
- The optical particle conversion depends on the exact sensor, ADC reference, supply voltage, and calibration. The constants in the firmware must be validated for the final hardware.
- BSEC readings require stabilization and become more useful after the sensor has warmed up and learned its environment.
- `LUFT-AP` is currently an open network intended for demonstrations and local prototype use. Add authentication before using the device in an untrusted location.
- The current Nextion classification logic is a prototype and should be calibrated before it is used to communicate health guidance.

## Acknowledgments

**Sophia Lopes helped Vinícius Ferreira develop this project.** The educational page's `/sophia` route and related code identifiers use her name in recognition of that contribution; `sophia` is not a separate product or software dependency.

LUFT was inspired by the Desafio Liga Jovem goal of helping students use technology, entrepreneurship, and creativity to improve their communities.

## License

The project is available under the [MIT License](LICENSE).
