# Knell

A 2D single-threaded main loop C99 game engine for prototyping and learning.

## Goal

- Speed the iteration time when creating 2D game demos and visual simulations.

## Primary Mechanism of Attainment

- Dynamically load games/demos/prototypes with minimal interfaces and
  boilerplate, so programmers can focus on the ideas they want to experiment
  with and iterate quickly before adding to production systems.

## Guiding Design Principle (ASAP/ASAP)

- "As simple as possible": Doing things with the engine should be
  straightforward and explicit.  It might be more verbose than languages with
  more features than C; let's not conflate terseness with simplicity.
- "As soon as possible": Use the ability to create prototypes to streamline
  the development process by trying ideas in fast-to-compile sandboxes.

## Limitations of Scope

- No true 3D support.  Many issues get much more complicated and harder to
  visualize and reason about in three dimensions.
- Core functionality shall only be written in C.  C provides mostly predictable
  symbol generation and more centralized control flow making it simpler to
  reason about program behavior while learning.  It also simplifies interfacing
  with other languages.
