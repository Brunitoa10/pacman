# Pac-Man en C

Pac-Man en C11 con dos "backends" intercambiables, sobre la misma lógica de juego:

- **Consola** (`pacman.exe`): sin dependencias, terminal ANSI (Windows 10+, Linux, macOS).
- **Gráfico** (`pacman_gui.exe`): ventana con [raylib](https://www.raylib.com), paredes estilo arcade, sprites de Pac-Man y fantasmas.

## Compilar y jugar

```bat
build.bat          :: compila pacman.exe
build.bat test     :: compila y corre los tests
pacman.exe
```

Versión gráfica: instalá raylib para Windows (el instalador oficial deja todo en `C:\raylib`) y corré:

```bat
build_raylib.bat   :: compila pacman_gui.exe
```

Si raylib está en otro lado: `set RAYLIB_DIR=C:\ruta\a\raylib` antes de compilar (sirve la carpeta `src` de la instalación o una release `raylib-5.5_win64_mingw-w64` descomprimida).

Con make: `mingw32-make` (consola), `mingw32-make gui` (raylib), `mingw32-make test`.

Controles: **WASD / flechas** mover · **P / espacio** pausa · **R / Enter** reiniciar tras Game Over · **Q / Esc** salir.
La ventana necesita al menos 56×33 caracteres; si se corta, agrandala o maximizala.

## Arquitectura

```
include/ src/
├── core/      Dominio puro: reglas del juego. Sin I/O.
├── ports/     Interfaces (Renderer, InputSource) que el dominio/vista necesitan.
├── view/      Traduce el estado del juego en llamadas al Renderer.
├── platform/  Implementaciones concretas: consola ANSI, teclado, reloj.
│   └── raylib/  Backend gráfico: renderer, entrada y fábrica con raylib.
└── main.c     Composition root: pide la plataforma a la fábrica y corre el bucle.
tests/         Tests del core + vista, compilados SIN la capa de plataforma.
```

Las dependencias apuntan hacia adentro: `platform → ports ← view → core`. El core no sabe que existe una consola.

## SOLID

| Principio | Dónde |
|---|---|
| **S** Responsabilidad única | `maze` (mapa), `pacman`, `ghost`, `mode_schedule` (cronograma), `scoreboard` (puntaje), `game_view` (dibujo), cada uno en su módulo. |
| **O** Abierto/cerrado | Fantasma nuevo = una función + una fila en `ghost_ai.c`. Mapa nuevo = un layout de texto. Estado nuevo = una fila en la tabla. |
| **L** Sustitución | Cualquier `Renderer`/`InputSource` es intercambiable (los tests usan un renderer falso). |
| **I** Segregación de interfaces | `Renderer` e `InputSource` son chicas y separadas; la vista no ve el teclado ni el juego ve la pantalla. |
| **D** Inversión de dependencias | `game` recibe `Command`s abstractos; `main.c` es el único que conoce la consola. |

## Patrones de diseño

| Patrón | Archivo | Para qué |
|---|---|---|
| **Strategy** | `core/ghost_ai.c` | Blinky, Pinky, Inky y Clyde sólo difieren en la función que elige su objetivo. |
| **State** (tabla) | `core/ghost.c`, `core/game.c` | Estados del fantasma (en casa, saliendo, persiguiendo, asustado, comido...) y fases del juego (READY, PLAYING, DYING...). Sin `switch` gigantes. |
| **Observer** | `core/event_bus.c`, `core/scoreboard.c` | El juego publica eventos; el marcador (y quien quieras: sonido, logros) se suscribe. |
| **Command** | `core/command.h`, `core/game.c` | La entrada se convierte en órdenes; cada una tiene su handler en una tabla. |
| **Bridge / Adapter** | `ports/renderer.h`, `platform/console_renderer.c` | Abstracción de dibujo separada de su implementación. El backend raylib se agregó sin tocar una línea de `core/` ni `view/`. |
| **Abstract Factory** | `platform/platform.h` + `platform_console.c` / `raylib/platform_raylib.c` | Crea la familia renderer + entrada de un backend. `main.c` es idéntico para consola y raylib; el backend lo decide qué archivo se compila. |
| **Factory** | `ghost_create`, `console_renderer_create`, `console_input_create` | Encapsulan la construcción y el cableado de cada objeto. |
| **Parameter Object** | `GhostWorld` | Agrupa lo que un fantasma necesita saber del mundo en cada tick. |
| **Game Loop (paso fijo)** | `main.c` | Simulación determinista a 40 ticks/s, independiente de la PC. |

## Clean code aplicado

- Funciones cortas con nombres que dicen qué hacen; sin números mágicos (todo en `core/config.h`).
- `static` para todo lo privado de cada `.c`; los `.h` exponen sólo la API.
- Include guards, `const` correctness y compilación limpia con `-Wall -Wextra -Wpedantic`.
- RNG inyectado (no `rand()` global) ⇒ partidas reproducibles y testeables.
- Tests de robustez: p. ej. se verifica que un fantasma comido vuelve a casa desde **cada** casilla alcanzable.
