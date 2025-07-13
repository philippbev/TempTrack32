🌡️ TempTrack32 - ESP32 Temperatur- und Luftfeuchtigkeit-Logger

TempTrack32 ist ein einfaches, lokal betriebenes Temperatur- und Luftfeuchtigkeits-Datenerfassungssystem auf Basis eines ESP32 und eines DHT11-Sensors. Die Daten werden regelmäßig gemessen, intern gespeichert und über ein integriertes Webinterface angezeigt oder als CSV-Datei heruntergeladen.

🔧 Features
	•	⏱️ Automatische Messung alle 10 Minuten (konfigurierbar)
	•	🌐 Lokaler WLAN Access Point ohne Internetverbindung
	•	📊 Tagesstatistiken (Minimum, Maximum, Durchschnitt)
	•	📝 Manuelle Messung per Button im Webinterface
	•	🕒 Manuelles Setzen der Systemzeit via Webinterface
	•	📥 CSV-Export der gespeicherten Daten (30 Tage Historie)

🧰 Verwendete Komponenten
	•	ESP32 (z. B. DevKitC oder NodeMCU)
	•	DHT11 Temperatur- und Feuchtigkeitssensor
	•	Arduino IDE + Board-Unterstützung für ESP32

🖥️ Webinterface

Das Webinterface läuft direkt auf dem ESP32 und ist über ein WLAN-Access-Point erreichbar.

🔌 Einrichtung

1.	📥 Klone das Repository:
2.	📦 Installiere die benötigten Bibliotheken über den Bibliotheksverwalter:
	•	WiFi.h
	•	WebServer.h
	•	DHT sensor library
	3.	🔌 Verbinde den DHT11 mit dem ESP32:
	•	Signal → GPIO 15
	•	VCC → 3.3 V
	•	GND → GND
	4.	🔧 Lade den Sketch auf den ESP32 hoch.
	5.	📶 Der ESP32 erstellt nun ein WLAN mit dem Namen TempTrack32 (Passwort: PhilippIstToll!). Verbinde dich damit.
	6.	🌐 Öffne im Browser 192.168.4.1 um das Interface aufzurufen.

📁 Datenaufzeichnung
	•	Die Daten werden intern in einem Ringpuffer gespeichert (dataLog[4320] = ca. 30 Tage bei 10-minütigem Intervall).
	•	Jede Messung wird mit einem Zeitstempel versehen, der auf einer manuell gesetzten Uhr basiert.

⚠️ Hinweise
	•	Die Zeit wird nicht automatisch synchronisiert, sondern muss über das Webinterface gesetzt werden.
	•	Die Daten gehen beim Neustart verloren (keine persistente Speicherung in SPIFFS oder SD-Karte).
	•	Der DHT11 ist nicht sehr präzise – für bessere Genauigkeit empfiehlt sich ein DHT22 oder BME280.





