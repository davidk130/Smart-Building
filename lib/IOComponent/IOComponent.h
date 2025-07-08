#ifndef IOCOMPONENT_H
#define IOCOMPONENT_H

/**
 * @brief Abstrakte Basisklasse für alle Ein-/Ausgabekomponenten.
 *
 * Jede konkrete Sensor- oder Aktorklasse muss begin() und handle() implementieren.
 * Damit wird eine einheitliche Schnittstelle sichergestellt.
 */
class IOComponent {
public:
    /**
     * @brief Wird im setup() aufgerufen. Dient zur Initialisierung der Komponente.
     */
    virtual void begin() = 0;

    /**
     * @brief Wird in jedem loop()-Durchlauf zyklisch aufgerufen.
     * Wird für Sensorabfragen oder Aktorsteuerung verwendet.
     */
    virtual void handle() = 0;

    /**
     * @brief Virtueller Destruktor.
     */
    virtual ~IOComponent() {}
};

#endif
