### DSL-based animation system

The animation system provides a compile-time validated DSL for defining property animations on scene objects, UI elements, and cameras. It supports keyframe-based animation with easing functions, custom interpolators, and runtime playback control.

## Core Components

### DSL (`dsl.h`)

The domain-specific language enables expressive animation definitions with compile-time validation:

```cpp
using namespace animation::dsl;

KeyframeCollection keyframes;
keyframes += keyframe(0s) | accessor::point(entity) / point::color << Color::cian();
keyframes += keyframe(300ms) | accessor::point(entity) / point::color << Color::magenta();
```

**Key DSL elements:**
- `accessor::point(id)` / `accessor::plane(id)` / `accessor::button(id)` / `accessor::camera(id)` / `accessor::ui()` — target selectors
- `/` operator — property path navigation (compile-time validated against the reflection registry)
- `<<` operator — binds a property value to create a key
- `|` operator — attaches a key to a keyframe
- `keyframe(ms)` — creates a keyframe at a specific time
- `key(value, easing)` / `key(value, interpolator)` — wraps values with easing/interpolation

### Reflection (`reflection.h`)

Static introspection system that maps scene types to their underlying ECS components and member offsets:

```
scene::Point
├── ecs::component::Position → Vec3 (x, y, z)
├── ecs::component::MainColor → Color (r, g, b, a)
└── ecs::component::Radius → float
```

The reflection registry stores a flat property tree with byte offsets, enabling direct memory writes during animation sampling without virtual dispatch.

### Float Span Adapter (`float_span_adaptor.h`)

Converts between typed values and flat float arrays for the animation pipeline. Specialized for:
- `float` (1 channel)
- `Vec3` (3 channels)
- `Color` (4 channels)
- `scene::Point` (8 channels)
- `Camera` (10 channels)

### Manager (`manager.h`)

Core lifecycle manager:

| Phase | Description |
|-------|-------------|
| **Build** | Sorts keys, groups by root property, creates `Asset` with flattened channels |
| **Play** | Creates `Instance` from `Clip`, returns `Handle` |
| **Tick** | Advances elapsed time, handles looping, markers, completion |
| **Animate** | Samples assets and applies values to ECS/UI state |

### Asset (`types.h`)

Immutable, pre-baked animation data:
- **Channels** — individual float properties with keyframe time arrays
- **Keys** — packed values in column-major layout for cache efficiency
- **Interpolations** — easing functions or custom multi-channel interpolators

### Instance (`types.h`)

Runtime playback state with bit-packed flags:
- Looping, reversed, paused, remove-on-complete, pause-on-marker, finished
- Marker set for step control
- Progress tracking (0.0–1.0)

### Handle (`handle.h`)

Lightweight controller for active animations:
```cpp
Handle h = manager.buildAndPlay(keyframes);
h.pause();
h.setProgress(0.5f);
h.playForward();
h.stepForwardToNextMarker();
```

### Sampler (`sampler.h`)

Fixed-buffer sampler (no heap allocation during playback):
- Finds bounding keyframes via binary search
- Supports scalar interpolation with easing
- Supports batched custom interpolators for multi-channel properties

### Service (`service.h`)

Factory for common animation patterns:
- `createHoverPointAnimation(entity)` — reversible hover effect
- `createSelectPointAnimation(entity)` — bounce selection pulse
- `createHoverPlaneAnimation(entity)` — opacity-based hover
- `createHoverButtonAnimation(element)` — shared asset for UI buttons
- `playAppearUIAnimation()` — fade-in for UI layout

### Player Manager (`player_manager.h`)

Bridges animation instances to UI controls:
- Play/pause/step/scrub controls
- Marker visualization on timeline slider
- Time formatting (`MM:SS.mmm`)

## Data Flow

```
KeyframeCollection (DSL)
    ↓ build()
AssetCollection { Target → Asset* }
    ↓ play()
Instance (runtime state)
    ↓ tick() → animate() → sample()
Sampled float channels
    ↓ apply()
Raw bytes written to ECS/UI memory
```

## Usage Example

```cpp
// Hover animation
ReversableAnimation hover = service.createHoverPointAnimation(entity);
hover(true);  // play forward
hover(false); // play backward

// Custom animation
KeyframeCollection keyframes;
keyframes += keyframe(0s) 
    | accessor::camera(0) / camera::position << Vec3{0,0,10};
keyframes += keyframe(1s) 
    | accessor::camera(0) / camera::position << key(Vec3{0,0,20}, ease::inOut::quad);

Handle anim = manager.buildAndPlay(keyframes, [](Clip& c) {
    c.loop = true;
    c.pauseOnMarker = true;
});
```

## Design Notes

- **Compile-time validation** — invalid property paths or type mismatches fail at compile time
- **No dynamic allocation during sampling** — fixed-size buffers sized at compile time
- **Data-oriented layout** — keys stored column-major for sequential memory access
- **Shared assets** — multiple instances can reference the same baked `Asset`
- **Reversible playback** — all animations can play forward or backward