# Ball Impulse

Qt application showcasing impulse forces acting on a ball.
Different terrains can be swapped for different interactions.
Sphere and dodecahedron ball models are available.

![ball-impulse](https://github.com/user-attachments/assets/b7f360b6-84b3-424a-afc7-d1e4da8a9506)

## Project Structure

```plaintext
ball-impulse/
├── src/                   # Source code
├── assets/                # Static assets (.dem and .bvh files)
├── ball-impulse.pro       # QMake project
└── README.md              # Project README
```

## Build

```bash
qmake
make
```

## Run

```bash
bin/ball-impulse
```

## Controls

| Key(s)    | Action                             |
|-----------|------------------------------------|
| `<` / `>` | Adjust launch angle around +Z      |
| `L`       | Re-launch ball                     |
| `W` / `S` | Move camera forwards and backwards |
| `A` / `D` | Move camera left and right         |
| `R` / `F` | Move camera up and down            |
| `Q` / `E` | Yaw camera left and right          |
| `X`       | Exit application                   |

## Technologies

* **C++**: `>= C++17`
* **Qt**: `5.12.x`
* **OpenGL**: `>= 4.0`

Newer versions of Qt might work correctly, if no breaking changes that affect the application were introduced.

## TODOs

* [ ] Proper Dodecahedron impulse computation
