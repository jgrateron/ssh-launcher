# SSH Launcher

TUI en C11 + ncurses para gestionar conexiones SSH. Lee `~/.ssh/config`, permite navegar, filtrar y conectarse a servidores desde una interfaz de terminal de doble panel.

## Requisitos

- GCC (C11)
- ncurses (`libncurses-dev`)
- Linux / Unix

## Instalación

```bash
make
sudo make install   # instala en /usr/local/bin/
```

## Uso

```bash
./ssh-launcher
```

Asegurate de tener un `~/.ssh/config` con hosts configurados. Podés usar `sample_ssh_config` como base:

```bash
cp sample_ssh_config ~/.ssh/config
```

## Controles

| Tecla | Acción |
|---|---|
| `↑` `↓` / `j` `k` | Navegar lista |
| `Tab` | Cambiar panel (Recientes ↔ Todos) |
| `Enter` | Conectar por SSH al host seleccionado |
| `/` | Buscar (filtro fuzzy incremental) |
| `F2` | Cambiar tema de color |
| `Esc` | Cancelar búsqueda / Salir |

## Temas

6 temas incluidos, ciclo con `F2`:

0. **Default** — cyan, verde y azul sobre negro
1. **Light** — texto oscuro sobre fondo claro
2. **Monochrome** — solo blanco y negro
3. **Ocean** — azules profundos y cyans
4. **Retro** — ámbar y verde estilo terminal vintage
5. **Solarized** — paleta solarized oscura

El tema se guarda en `~/.config/ssh-launcher/theme`.

## Archivos de datos

| Archivo | Ubicación |
|---|---|
| SSH config | `~/.ssh/config` |
| Historial (últimos 5) | `~/.config/ssh-launcher/history` |
| Tema guardado | `~/.config/ssh-launcher/theme` |

## Estructura del proyecto

```
src/
  main.c          # Punto de entrada
  app.h / app.c   # Estado, bucle de eventos, orquestación
  parser.h / .c   # Parser de ~/.ssh/config
  history.h / .c  # Persistencia de historial
  search.h / .c   # Búsqueda fuzzy (character-skip)
  ui.h / ui.c     # Interfaz ncurses (ventanas, temas, input)
```

## Compilación

```bash
make clean && make    # C11, -Wall -Wextra -pedantic, cero warnings
```

## Búsqueda fuzzy

La búsqueda usa **character-skip matching** (tipo fzf): todos los caracteres del patrón deben aparecer en orden en el nombre del host, sin necesidad de ser consecutivos.

Ejemplo: `prd` coincide con `prod-web-01` (p, r, d aparecen en orden).
