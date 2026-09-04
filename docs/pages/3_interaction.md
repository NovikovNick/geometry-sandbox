### ECS-based interaction system

The interaction system handles mouse-based selection, hover detection, and translation gizmo manipulation for entities in the 3D scene. It bridges user input with ECS components and UI state, providing visual feedback and transformation capabilities.

## Core Components

### Interaction Service (`service.h`)

Central coordinator that manages selection and hover state, processes mouse input, and performs raycasting against scene objects.

**Responsibilities:**
- Mouse press/release handling for selection
- Raycasting against sphere and AABB colliders
- Hover state management with cursor feedback
- Translation gizmo activation and constraint

### ECS Components (`ecs_components.h`)

Marker and callback components for interaction state:

| Component | Purpose |
|-----------|---------|
| `Selected` | Marks entity as currently selected |
| `OnSelectCallback` | Invoked when entity becomes selected |
| `Hovered` | Marks entity as under mouse cursor |
| `OnHoverCallback` | Invoked with `true`/`false` on hover begin/end |
| `Translated` | Stores current position during gizmo drag |
| `OnTranslateCallback` | Invoked with new position on translation update |

### Translate Gizmo (`translate_gizmo.h`)

3D transformation widget with axis and plane manipulation:

**Services:**
- `ITranslateGizmoUpdateService` — hit testing, constraint setup, translation math
- `ITranslateGizmoRenderService` — visual rendering of axes, arrows, and planes

**Gizmo States:**
- `hoveredAxis` — which axis/plane is under cursor (X, Y, Z, XY, YZ, ZX, or None)
- `constrained` — whether gizmo is actively dragging
- `constrainedOnSingleAxis` — single-axis vs plane translation mode

## Interaction Flow

### Selection
```
Mouse Press → Check gizmo hover → If not, raycast scene
    → Update Selected component
    → Show/hide gizmo at entity position
```

### Hover
```
Mouse Move → Raycast all colliders → Find nearest hit
    → Update Hovered component
    → Update cursor type (Arrow ↔ PointingHand)
```

### Translation
```
Mouse Press on Gizmo → constrain() → Set constraint plane/axis
Mouse Drag → translate() → Update Translated component
Mouse Release → Clear selection and reset gizmo
```

## Raycasting

The service performs ray-sphere and ray-AABB intersection tests:

```cpp
// Sphere collider check
const Sphere sphere{.center = pos.val, .radius = collider.radius};
const Intersection intersection = intersect(ui.mouseRay, sphere);

// AABB collider check
const AABB aabb{.min = position.val - collider.size / 2, 
                .max = position.val + collider.size / 2};
const Intersection intersection = intersect(ui.mouseRay, aabb);
```

Nearest hit determines the hovered entity.

## Gizmo Math

### Constraint Setup
- **Single axis (X/Y/Z):** Creates a plane perpendicular to the axis, oriented toward the camera ray
- **Plane mode (XY/YZ/ZX):** Locks movement to the selected plane

### Drag Offset
```cpp
// Calculate initial offset between click point and gizmo position
const float projection = (intersection.point - gizmo.position).dot(gizmo.constraintAxis);
gizmo.dragOffset = res - gizmo.position;
```

### Translation
```cpp
// Apply offset during drag for smooth movement
const Vec3 res = gizmo.constrainedOnSingleAxis 
    ? gizmo.position + gizmo.constraintAxis * projection 
    : intersection.point;
gizmo.position = res - gizmo.dragOffset;
```

## Gizmo Rendering

Visual elements with distinct colors:
- **Arrows** — X (red), Y (green), Z (blue) axes
- **Planes** — XY, YZ, ZX translucent quads
- **Highlight** — hovered axis/plane changes to selected color

Scale factor adjusts gizmo size based on camera distance for consistent screen-space appearance.

## Usage Example

```cpp
// Entity with interaction components
registry.emplace<ecs::component::Position>(entity, Vec3{0, 0, 0});
registry.emplace<ecs::component::SphereCollider>(entity, 1.0f);
registry.emplace<ecs::component::OnSelectCallback>(entity, []() {
    // Handle selection
});
registry.emplace<ecs::component::OnHoverCallback>(entity, [](bool hovered) {
    // Handle hover state change
});
registry.emplace<ecs::component::OnTranslateCallback>(entity, [](Vec3 pos) {
    // Handle position update during gizmo drag
});
```

## Design Notes

- **Event-driven callbacks** — uses EnTT's `on_construct`/`on_update`/`on_destroy` signals for component lifecycle notifications
- **Gizmo priority** — gizmo interaction takes precedence over scene object selection
- **State reset** — gizmo fully resets after translation completes
- **Camera-aware scaling** — gizmo maintains consistent screen size regardless of camera distance
- **Direct memory access** — translation writes directly to `Translated` component, with `patch()` notifying observers





