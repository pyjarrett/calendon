# Knell

A 2D single-threaded main loop C99 game engine for prototyping and learning.

**Under active development, not suitable for production use**.

## Goal

- Speed the iteration time when creating 2D game demos and visual simulations.

### Method

- Dynamically load games/demos/prototypes with minimal interfaces and
  boilerplate, so programmers can focus on the ideas they want to experiment
  with and iterate quickly before adding to production systems.

### Guiding Design Principle (ASAP/ASAP)

- "As simple as possible": Doing things with the engine should be
  straightforward and explicit, but it might be more verbose than languages with
  more features than C.  That's ok.
- "As soon as possible": Use the ability to create prototypes to streamline
  the development process by trying ideas in fast-to-compile sandboxes.

### Limitations of Scope

- No true 3D support.  Many issues get much more complicated and harder to
  visualize and reason about in three dimensions.
- Core functionality shall only be written in C.  C provides mostly predictable
  symbol generation and more centralized control flow making it simpler to
  reason about program behavior while learning.  It also simplifies interfacing
  with other languages.

## Building, Running Tests, and Running Demos

Keeping with the "bell" theme, "Hammer" is the project launch tool, aiming to
provide an interactive environment for building, running tests, and program
inspection.

```bash
tools/hammer.py

# Generate build directory
(hammer master) gen

# Do a build
(hammer master) build

# Run tests
(hammer master) check

# List the demos which can be run
(hammer master) set demo planets

# Run the demo
(hammer master) run demo
```

When developing Hammer, changes can be dynamically reloaded at runtime.

```bash
tools/hammer.py

# Make changes to the Hammer scripts in the kn/ directory.

(hammer master) reload

# << script saves config, restarts the REPL, and reloads configs >>

(hammer master)
```

