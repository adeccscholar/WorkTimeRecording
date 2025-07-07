# Finiten Automaten und Boost.Statechart

## 1. Grundbegriffe der finiten Statemaschine

**Endlicher Automat (Finite State Machine, FSM)**  
Ein endlicher Automat ist ein mathematisches Modell zur Beschreibung des Verhaltens eines Systems, das sich in endlich vielen Zuständen befinden kann und in Abhängigkeit von Eingaben zwischen diesen Zuständen wechselt.

### Zentrale Begriffe:

- **Zustand (State)**  
  Ein Zustand beschreibt eine Konfiguration des Systems zu einem bestimmten Zeitpunkt. Er bestimmt, wie das System auf Ereignisse reagiert.

- **Ereignis (Event)**  
  Ein Ereignis ist ein externer oder interner Stimulus, der einen Zustandswechsel auslösen kann.

- **Übergang (Transition)**  
  Ein Übergang ist die Regel, die beschreibt, wie und wann das System von einem Zustand in einen anderen wechselt. Er wird durch ein Ereignis ausgelöst.

- **Startzustand (Initial State)**  
  Der Zustand, in dem die FSM beginnt.

- **Endzustand (Final State)**  
  Ein spezieller Zustand, der das Ende der Verarbeitung markiert.

- **Aktion (Action/Reaction)**  
  Aktionen können beim Eintritt in einen Zustand, beim Verlassen eines Zustands oder während eines Übergangs ausgeführt werden.

---

## 2. Konzepte einer FSM

### 2.1 Deterministische vs. Nichtdeterministische Automaten
- **Deterministisch:** Zu jedem Zustand und jedem möglichen Eingabesymbol gibt es höchstens einen Übergang.
- **Nichtdeterministisch:** Für einen Zustand und ein Eingabesymbol können mehrere mögliche Übergänge existieren.

### 2.2 Hierarchische und Nebenläufige Zustände
Fortgeschrittene Frameworks (wie Boost.Statechart) erlauben hierarchische Zustände, d.h. Zustände können selbst wieder Zustandsautomaten enthalten. Auch Nebenläufigkeit (orthogonale Regionen) kann modelliert werden.

### 2.3 Mealy- und Moore-Automaten
- **Mealy:** Die Aktionen hängen von Zustand **und** Eingabe ab.
- **Moore:** Die Aktionen hängen nur vom Zustand ab.

---

## 3. Boost.Statechart: Umsetzung einer FSM in C++

**Boost.Statechart** ist eine C++-Bibliothek, mit der sich komplexe endliche Automaten elegant und typsicher implementieren lassen. Sie folgt dem Konzept der **State Machine** nach UML.

### 3.1 Grundstruktur mit Boost.Statechart

#### \brief Zentrale Typen und Klassen

- **State Machine**  
  Die Basisklasse der eigentlichen Automaten-Instanz, abgeleitet von `boost::statechart::state_machine`.
- **State**  
  Zustände werden als Klassen modelliert, die von `boost::statechart::state` ableiten.
- **Event**  
  Ereignisse sind typisierte Klassen, die von `boost::statechart::event` abgeleitet werden.
- **Transition Table**  
  Definiert mögliche Übergänge (`transition<>`) zwischen Zuständen.

#### \details Beispiel

Ein einfacher Automat, der einen Türzustand (offen/geschlossen) modelliert:

```cpp
// Event-Klassen
struct EvOpen : boost::statechart::event<EvOpen> {};
struct EvClose : boost::statechart::event<EvClose> {};

// Vorwärtsdeklaration der States
struct StOpen;
struct StClosed;

// Die State Machine
struct DoorStateMachine
   : boost::statechart::state_machine<DoorStateMachine, StClosed> {};

// Zustände
struct StClosed : boost::statechart::state<StClosed, DoorStateMachine>
{
   StClosed(my_context ctx) : my_base(ctx) {}
   typedef boost::mpl::list<
      boost::statechart::transition<EvOpen, StOpen>
   > reactions;
};

struct StOpen : boost::statechart::state<StOpen, DoorStateMachine>
{
   StOpen(my_context ctx) : my_base(ctx) {}
   typedef boost::mpl::list<
      boost::statechart::transition<EvClose, StClosed>
   > reactions;
};
```

#### \note
- Die **reactions**-Liste definiert für jeden Zustand die möglichen Ereignisse und die Zielzustände.
- Die State Machine `DoorStateMachine` startet im Zustand `StClosed`.
- Der Automat wechselt bei `EvOpen` von `StClosed` nach `StOpen` und bei `EvClose` wieder zurück.

---

## 4. Wichtige Konzepte in Boost.Statechart

- **Hierarchie:** Zustände können Submaschinen oder substates enthalten.
- **Ein- und Austrittsaktionen:** Konstruktions- und Destruktormethoden eines States erlauben das Ausführen von Code beim Zustandswechsel.
- **Zustandsabfrage:** Mit Methoden wie `state_cast` kann der aktuelle Zustand zur Laufzeit überprüft werden.
- **Nebenläufigkeit:** Orthogonale Regionen sind abbildbar (mehrere Zustandsautomaten in einem Objekt).

---

## 5. Zusammenfassung

Finiten Automaten sind fundamentale Modelle zur Zustandsbeschreibung und -steuerung.  
Boost.Statechart stellt eine mächtige, moderne und typsichere Lösung bereit, um auch komplexe und hierarchische FSMs in C++ elegant und wartbar umzusetzen.

**Wenn Du ein konkretes Beispiel in Deinem Stil oder eine Erweiterung für bestimmte Anwendungsfälle sehen möchtest, sag Bescheid.**
