# EMC
Repositorio para almacenar el código, blueprints, y datasheets de la Estación Meteorológica Casera (EMC)

## Lista de Materiales

* Placa ESP32 (Steren, mayormente parecido al modelo ESP32 DEVKIT V1 - DOIT)
* Protoboard (Steren)
* Sensor de Temperatura y Humedad DHT11 (Steren)
* Sensor de Gas / Calidad de Aire MQ135 (Steren)
* Sensor de Humedad de Suelo tipo FC-28 / YL-69 (Steren)
* Display 7 segmentos, 4 dígitos (Steren)
* Cable de Alambre
* Cables Dupont
* Carcasa de plástico

## Librerías utilizadas

```Arduino

#include "DHT.h"

```

## Estructura del repositorio

```bash
.
├── arduino/
│   └── emc/
│       └── emc.ino
├── blueprints/
│   ├── complete-model.png
│   ├── dht11-connection.png
│   ├── display-connection.png
│   ├── esp32-connection.png
│   ├── mq135-connection.png
│   └── soil-sensor-connection.png
├── datasheets/
│   ├── 5461as.pdf
│   ├── dht11.pdf
│   ├── esp32-v1-doit.pdf
│   ├── fc-28.pdf
│   └── mq135.pdf
└── README.md
```

## Integrantes del equipo

* García Romero Angel 22760920
* González Navarro Oscar Eduardo 22760560
* Martínez Monge Saúl Guillermo 22760566
* Pardo Rubalcaba Andrés 22760571
