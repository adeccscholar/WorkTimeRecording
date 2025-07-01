/*
      🔧 Standortdaten:");
        🌍 Zeitzone
        ⏱️ UTC Offset
        🏔️ Höhe

      📅 Datum
        🌅 Sunrise
        🌇 Sunset
        🌡️ Temp
        🌧️ Rain
        💨 Wind
        🔆 UV
        🧭 Code

        ⚠️ Warnung
*/


/*


https://open-meteo.com/en/docs#daily
daily
1 bis 16 Tage in die Zukunft (kostenlos, ohne Authentifizierung, bis 10.000 Zugriffe)
Standardmäßig: 7 Tage, wenn forecast_days nicht explizit angegeben ist

| Parametername                | Einheit         | Beschreibung                             |
| ---------------------------- | --------------- | ---------------------------------------- |
| `temperature_2m_max`         | °C              | Tageshöchsttemperatur                    |
| `temperature_2m_min`         | °C              | Tagestiefsttemperatur                    |
| `apparent_temperature_max`   | °C              | Max. gefühlte Temperatur                 |
| `apparent_temperature_min`   | °C              | Min. gefühlte Temperatur                 |
| `sunrise`                    | ISO 8601-Zeit   | Sonnenaufgang (lokale Zeit)              |
| `sunset`                     | ISO 8601-Zeit   | Sonnenuntergang (lokale Zeit)            |
| `precipitation_sum`          | mm              | Gesamtniederschlag (Regen + Schnee)      |
| `rain_sum`                   | mm              | Nur Regenmenge                           |
| `showers_sum`                | mm              | Schaueranteil (modellabhängig)           |
| `snowfall_sum`               | cm              | Schneemenge                              |
| `precipitation_hours`        | Stunden         | Dauer des Niederschlags                  |
| `windspeed_10m_max`          | km/h            | Tageshöchste Windgeschwindigkeit in 10 m |
| `windgusts_10m_max`          | km/h            | Maximale Böen                            |
| `winddirection_10m_dominant` | ° (0–360)       | Vorherrschende Windrichtung              |
| `shortwave_radiation_sum`    | MJ/m²           | Globale Sonneneinstrahlung               |
| `et0_fao_evapotranspiration` | mm              | Verdunstung (nach FAO Penman-Monteith)   |
| `uv_index_max`               | Index (0–11+)   | Höchster UV-Index                        |
| `weathercode`                | Kategorie (int) | Symbolisches Wetter (☀️ 🌧️ ❄️ etc.)     |


hourly
Bis zu 120 Stunden (5 Tage) für stündliche Vorhersage

| Parameter              | Einheit   | Beschreibung (DE)                     |
| ---------------------- | --------- | ------------------------------------- |
| `temperature_2m`       | °C        | Lufttemperatur in 2 m Höhe            |
| `relative_humidity_2m` | %         | Relative Luftfeuchtigkeit in 2 m Höhe |
| `dew_point_2m`         | °C        | Taupunkt in 2 m Höhe                  |
| `apparent_temperature` | °C        | Gefühlte Temperatur                   |
| `precipitation`        | mm        | Gesamtniederschlag (Regen + Schnee)   |
| `rain`                 | mm        | Regenmenge                            |
| `showers`              | mm        | Regenschauer separat                  |
| `snowfall`             | cm        | Schneemenge                           |
| `weathercode`          | WMO code  | Wetterzustandscode (wie bei `daily`)  |
| `pressure_msl`         | hPa       | Luftdruck auf Meereshöhe              |
| `surface_pressure`     | hPa       | Tatsächlicher Luftdruck auf Bodenhöhe |
| `cloudcover`           | %         | Gesamte Bewölkung                     |
| `cloudcover_low`       | %         | Tiefe Wolken                          |
| `cloudcover_mid`       | %         | Mittlere Wolken                       |
| `cloudcover_high`      | %         | Hohe Wolken                           |
| `shortwave_radiation`  | W/m²      | Globalstrahlung                       |
| `direct_radiation`     | W/m²      | Direkte Sonnenstrahlung               |
| `diffuse_radiation`    | W/m²      | Streustrahlung                        |
| `windspeed_10m`        | km/h      | Windgeschwindigkeit in 10 m Höhe      |
| `windgusts_10m`        | km/h      | Windböen in 10 m Höhe                 |
| `winddirection_10m`    | ° (0–360) | Windrichtung (meteorologisch)         |
| `uv_index`             | -         | UV-Index stündlich                    |
| `is_day`               | 0 oder 1  | Tag-/Nacht-Flag (1 = Tag, 0 = Nacht)  |

*/

/*
| UV-Index | Bedeutung | Schutzempfehlung                        |
| -------- | --------- | --------------------------------------- |
| 0–2      | Gering    | Kein Schutz nötig                       |
| 3–5      | Mäßig     | Sonnencreme, Sonnenbrille, Schatten     |
| 6–7      | Hoch      | Schutzkleidung, Kopfbedeckung, Schatten |
| 8–10     | Sehr hoch | Meiden der Sonne zwischen 11–15 Uhr     |
| 11+      | Extrem    | Aufenthalt im Freien vermeiden          |
*/


/*
| Kategorie                 | Regenmenge in mm/Tag  | Beschreibung                                |
| ------------------------- | --------------------- | ------------------------------------------- |
| Leicht                    | 0 – 10                | Normales Nieseln oder leichter Regen        |
| Mäßig                     | 10 – 30               | Anhaltender, mäßiger Regen                  |
| Stark                     | 30 – 50               | Kräftiger Regen, z. B. Gewitter              |
| **Sehr stark**            | 50 – 100              | Unwetter, Starkregenwarnung                 |
| **Extrem (z. B. Monsun)**  | **100 – 300**         | Tropische Regionen, kurze Zeiträume möglich |
| **Weltrekord** 🌍         | **\~1.800 mm / 24 h** | La Réunion (Frankreich), Tropensturm 1966   |

*/
