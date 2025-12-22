# Dispatching Compute Passes

> This section will cover dispatching compute shaders. If you haven't already, it is recommended to go through [Rendering and Object](./graphics.md) first to get a fuller understanding of how Groot Engine works.

[Resources](../resources/groot_engine_compute_tutorial_resources.zip)

With every Groot Engine project, create an engine. Then use this engine to compile your shaders.

```c++
// main.cpp

#include <groot/groot.hpp>

using namespace groot;

int main() {
  Engine engine;

  RID comp_shader = engine.compile_shader(ShaderType::Compute, "path/to/compute/shader");
  RID vert_shader = engine.compile_shader(ShaderType::Vertex, "path/to/vertex/shader");
  RID frag_shader = engine.compile_shader(ShaderType::Fragment, "path/to/fragment/shader");
}
```
> Note that it might be best to check that these shaders are valid with the `RID::is_valid()` method, as if there are any compilation errors with your shaders the program will display the errors but not exit.

Now lets create the storage texture. Storage textures are just textures that have 2 bindings. The first binding is for the UAV texture that you can read and write to. The second binding is for the sampler that can be read in the fragment shader. Groot Engine handles layout transitions of these images automatically so your storage textures are guaranteed to be in the correct layouts during your compute shader and your fragment shader.

```c++
// main.cpp
...
int main() {
  ...
  
  RID sampler = engine.create_sampler(SamplerSettings{});

  auto [width, height] = engine.viewport_dims();
  RID storage_texture = engine.create_storage_texture(width, height, sampler, Format::rgba16_sfloat);
}
```

Now create a descriptor set that uses this storage texture.

```c++
// main.cpp
...
int main() {
  ...

  RID descriptor_set = engine.create_descriptor_set({ storage_texture });
}
```

Now we have 2 pipelines to create: a compute pipeline and a graphics pipeline.

```c++
// main.cpp
...
int main() {
  ...

  RID compute_pipeline = engine.create_compute_pipeline(comp_shader, descriptor_set);
  RID graphics_pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex   = vert_shader,
    .fragment = frag_shader
  }, descriptor_set, GraphicsPipelineSettings{
    .cull_mode = CullMode::None
  });
}
```

For this example, we are creating a full screen triangle to display our compute shader output on. This is a standard practice for things like path tracers where the compute shaders do the rendering and you just need to display their output. For this reason, using no culling is fine since it doesnt matter which way the vertices are drawn.

The next step is to load the triangle mesh, create an object from it, the graphics pipeline, and the descriptor set, and then add it to the scene.

```c++
// main.cpp
...
int main() {
  ...

  RID mesh = engine.load_mesh("path/to/obj/file");
  
  Object triangle;
  triangle.set_mesh(mesh);
  triangle.set_pipeline(graphics_pipeline);
  triangle.set_descriptor_set(descriptor_set);

  engine.add_to_scene(triangle);
}
```

To dispatch the compute shader, you first create a compute command. Then, you tell the engine to dispatch all stored compute commands. For any commands that do not have barriers, they will be processed simultaneously, so make sure to specify that a barrier is needed if you have subsequent passes that depend on each other. After dispatching, run the engine and you should see a uv texture filling the whole viewport!

```c++
// main.cpp
...
int main() {
  ...
  engine.compute_command(ComputeCommand{
    .pipeline       = compute_pipeline,
    .descriptor_set = descriptor_set,
    .work_groups    = { (width + 7) / 8, (height + 7) / 8, 1 }
  });
  engine.dispatch();

  engine.run([&engine](double) {
    if (engine.just_pressed(Key::Escape))
      engine.close_window();
  });
}
```
> It is important to round up the thread count to make sure each thread gets one index in the output image. The formula for this is `(max_threads + local_threads - 1) / local_threads`. While this may produce a work group that goes over the thread count that you need, returning early in the compute shader if the thread index exceeds what you need makes it so that none of these extra threads actually get used.