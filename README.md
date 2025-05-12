# adecc Scholar: Distributed Enterprise Time-Tracking System

## Übersicht

Dieses Open-Source-Projekt wird im Rahmen des kostenlosen Schulungsangebots **adecc Scholar** entwickelt, das im Herbst 2020 während der Corona-Zeit mit 
kostenlosen Streams auf dem [adecc Scholar Twitch-Kanal](https://www.twitch.tv/volker_adecc) startete. 

Ziel dieses Projektes ist es, anhand eines praxisnahen Beispiels — einer Zeiterfassung innerhalb eines Unternehmens — die Entwicklung moderner verteilter 
Systeme in **C++23** zu vermitteln.

## Hauptziele

- Vermittlung moderner C++-Techniken (einschließlich **C++23**) und Frameworks
- Design und Implementierung eines verteilten Systems über verschiedene Rechnerklassen (Server, Desktop, Raspberry Pi, mobile Geräte)
- Zugriff auf Datenbanken mit SQL
- Integration von Sensor- und Gerätesteuerung (GPIO, I2C, SPI)
- Diskussion von Projektsteuerung, Datenbankentwurf und Normalisierung
- Demonstration plattformübergreifender Entwicklungen (Windows Server, Ubuntu Server, Windows/Linux Desktops, Embedded)

## Architektur

1. **Application Server**
   - Betriebssystem: **Ubuntu Linux**
   - CORBA/TAO als Middleware für verteilte Objekte
   - ODBC-Zugriff auf MS SQL Server
   - zentrale Implementierung der Business Logik

2. **Datenbank-Server**
   - Betriebssystem: **Windows Server**
   - Microsoft SQL Server
   - Relationale Modellierung und Normalisierung

3. **Arbeitsplatz-Clients**
   - Windows- und Linux-Desktops
   - CORBA/TAO als Middleware 
   - Qt6-basierte GUI für Zeiterfassung und Berichtswesen

4. **Embedded Clients (Raspberry Pi)**
   - CORBA/TAO (Client und Server)
   - Zugriff auf GPIO-Pins für diskrete Signale
   - Nutzung von I2C/SPI für Sensoren, Anzeigen und RFID-Lesegeräte
   - Eigenständige Rollen als Client und (lokaler) Server
   - RFID-basierte Zugangskontrolle mittels endlicher Zustandsmaschine (Boost.SML)

5. **Zukünftige Erweiterungen**
   - Web-REST-Server zur Einbindung mobiler Geräte
   - Diskussion von Architekturfragen, Chancen und Risiken

## Verwendete Technologien und Bibliotheken

- **C++23**: Modernste Sprachfunktionen für Effizienz und Plattformunabhängigkeit
- **Visual Studio C++**: Entwicklungsumgebung für Streams und Debugging
  - VS C++ ist modern, es bietet die Möglichkeit zum Cross- Compiling 
  - steht in der Community Version auch kostenlos zur Verfügung 
- **GCC14** über Crosscompiling, auf Linux Plattformen, inklusive Raspberry PI
- **Cmake** Buildverarbeitung
  - Einbindung eigener Routinen, inklusive CORBA Workflow  
- **Boost**
  - **Boost.SML** für finte Zustandsautomaten 
  - **Boost.JSON** für JSON-Verarbeitung (in späterer Erweiterung)
  - **Boost.Beast** für alternative HTTP-Anbindungen (in späterer Erweiterung)
- **CORBA/TAO**: Aktuelle TAO-Version als Middleware
  - Arbeiten mit ORB und POA, Basistechnologien, transiente und nicht transiente Servlets
  - CORBA Naming Services
  - CORBA Event Service, später auch TAO Events zur direkten Kommunikation
- **Qt 6**: Datenbankzugriff (Qt SQL) und GUI-Entwicklung
  - QDatabase und QWidgets
  - kostenlose Version bei Open- Source Projekten, Möglichkeiten zum Download und eigenständigen Übersetzen  
- **Embarcadero C++ Builder**
  - Einbindung älterer Werkzeuge (VCL, FMX)
  - Brücke zwischen Legacy-Code und modernen Systemen
- **MS SQL Server**: Relationale Datenbank
  - kostenlose Express Version oder Develepment Edition für Entwickler 

### Embarcadero C++ Builder

Durch die Unterstützung von **Embarcadero C++ Builder** (VCL und FMX) zeigt das Projekt, wie sich auch ältere oder nicht zeitgemäße Werkzeuge 
in eine moderne Systemarchitektur integrieren lassen. Damit eröffnen sich Möglichkeiten, bereits existierende Legacy-Applikationen anzubinden 
und Schritt für Schritt zu migrieren.

## Erste Schritte

1. Repository klonen:
   ```bash
   git clone https://github.com/adeccscholar/WorkTimeRecording.git
   ```

2. C++-Server-Komponenten mit Visual Studio öffnen und auf Ubuntu Server deployen  
3. Qt-Clients kompilieren und starten  
4. Raspberry Pi-Images flashen und Konfiguration durchführen

## Mitmachen und Beiträge

- Fork des Repositories anlegen  
- Feature-Branches erstellen (z.B. `feat/my-feature`)  
- Pull Requests mit ausführlicher Beschreibung  
- Issues für Bugs und Verbesserungsvorschläge verwenden  

## Lizenz und Copyright

```text
Copyright © adecc Systemhaus GmbH 2020–2025

Dieses Repository steht größtenteils unter der GNU General Public License v3.0 (GPL-3.0).
Abweichende Open Source Lizenzen in Teilprojekten möglich
```
