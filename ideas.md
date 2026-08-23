# Ideen

## Assumptions
- app auf dem Zielrechner compilieren und optimale Konfiguration erstellen lassen
- mit optimaler Konf. nochmals compilieren
  - und optimale Konfiguration in der app speichern
- 2 Implementierung
  - default Implementierung
  - optimierte Implementierung
- beim Laden überprüfen ob die optimale Konf. noch gilt und opt. Impl. laden
- ansonsten mit default Impl. weitermachen

## Linked strings
- Speicher in Blöcke aufteilen
- Blöcke verlinken
- pro Block:
  - gesamte Länge
  - Länge im Block
  - String offset
  - ptr zum weiter entfernten/nächsten Block
  - ptr zum vorherigen Block für etwa den halben String offset
- sozusagen binary search in den Blöcken

## Managed locks
- Anstatt ein Lock zu halten, reserviert man viele
  - bspw. array mit 32 locks
- Mit Daten die Locks brauchen offset für Locks vergeben
- damit werden gleiche Locks seltener gelockt
- man spart einen Lock pro Objekt
- bei Operation die mehrere Locks brauchen 
  - nach Lock-ID sortieren und aufsteigen sperren.