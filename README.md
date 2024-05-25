# Vergleich-Vulkan-OpenGL

- Dieses Repository ist entstanden für den Kurs An. zum Wissenschaftlichem Arbeiten.
- In den einzelnen Branches ist die jeweilige Methode in OpenGL und Vulkan implementiert.
- Es kann mit dem Makro VULKAN_TEST/OPENGL_TEST zwischen den APIs gewechselt werden.
- Die Vulkan Implementierung verwendet eine Abstraktionsschicht. Diese hat keinen Einfluss auf die Runtime performance.
- Das Projekt wurde nur unter Windows mit einer NVIDIA Grafikkarte getestet. 

# Verwendete Bibliotheken

- GLFW
- Vulkan Memory Allocator
- Glad

# Build Tutorial

1. Vorraussetzungen:
    - Visual Studio 2022
    - Vulkan SDK 1.2 oder neuer
    - OpenGL 4.0 oder neuer
2. Projektanpassungen
    - Bibliotheksverzeichnisse zum Vulkan SDK und OpenGL setzten
    - Inkludpfad zum Vulkan SDK hinzufügen
