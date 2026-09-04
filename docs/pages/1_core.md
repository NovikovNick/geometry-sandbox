# Core Concepts

### Dependency Injection
All components are registered in a single DI container using `boost::di`, enabling automatic injection and singleton management:

```cpp
auto injector = boost::di::make_injector(
    boost::di::bind<Settings>().in(boost::di::singleton),
    boost::di::bind<Application>().in(boost::di::singleton),
    di::inputManager(),
    di::windowManager(),
    di::cameraService()
    // ...
);
```

### Layer Separation
```
Application → Managers (stateful) → Services (stateless) → ECS (data)
```

### Manager/Service Pattern
- **Managers**: hold state and logic (`InputManager`, `WindowManager`, `ResourceManager`)
- **Services**: pure logic only (`CameraService`, `SceneService`, `JsonService`)

### Interface-Based Design
Every system exposes an interface and implementation:

```cpp
class ICameraService {
public:
    virtual Vec3 getForward(const Camera&) = 0;
    virtual Mat4 getViewMatrix(const Camera&) = 0;
    virtual ~ICameraService() = default;
};

class CameraService : public BaseService, public ICameraService {
    // implementation
};
```

### Centralized Configuration
All tunable parameters live in `Settings` struct, injected as singleton:

```cpp
struct Settings {
    std::string title;
    int width, height;
    bool fullscreen;
    Camera defaultCamera;
    float cameraMoveSensitivity;
    // ...
};
```

### Frame Loop Pipeline
```cpp
void Application::drawNextFrame(Nanoseconds timeDelta) {
    // 1. Update: input, UI, camera
    // 2. Animation: tick and animate
    // 3. Interaction: gizmos, selection
    // 4. Render: draw scene
}
```
