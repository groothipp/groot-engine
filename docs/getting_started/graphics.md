# Rendering an Object

> This section will cover rendering an object with a texture on it. Grab the `cube.obj` and `test.png` files from the `tests/dat` folder in the Groot Engine repository if you do not have another object and/or image to use in mind. Before starting, make sure to set up a C++ project to use CMake and confirm linking to Groot Engine works.

In your main.cpp file, include `groot/groot.hpp` add a new `Engine` object

```c++
// main.cpp

#include <groot/groot.hpp>

using namespace groot;

int main() {
  Engine engine;
}
```

The `Engine` object is your interface to the GPU. Everything you do in Groot Engine is controlled by this one class. It can be constructed with a `Settings` struct that allows you to specify different things like window size and whether to use VSync or not.

Now create a new file called `shader.vert` and open it. This is the object's vertex shader that will run for every vertex specified in the mesh. In Groot Engine, vertices are defined by a `vec3` position, `vec2` UV coordinate, and `vec3` normal.

> If your obj file does not define UV's or normals, Groot Engine will generate them when loading the mesh


Here is the base vertex shader. We will add to this later once the object is set up.
```glsl
// shader.vert

#version 460

layout(location = 0) in vec3 _VertexPosition;
layout(location = 1) in vec2 _VertexUV;
layout(location = 2) in vec3 _VertexNormal;

layout(location = 0) out vec2 _UV;
layout(location = 1) out vec3 _Normal;

void main() {
  
}
```

Now create a new file called `shader.frag` and open it. This will be the object's fragment shader which will run for every pixel mapped to by the rasterizer.
> If you do not know what the rasterizer is, please watch [this video](https://www.youtube.com/watch?v=_riranMmtvI) as you will learn quite a bit from it.

Here is the base fragment shader. We will add to this later as well after setting the object up.
```glsl
// shader.frag

#version 460

layout(location = 0) in vec2 _UV;
layout(location = 1) in vec3 _Normal;

layout(location = 0) out vec4 _FragColor;

void main() {
  
}
```

Now back to `main.cpp`, we can add a compilation step for these shaders:

```c++
// main.cpp
...

int main() {
  Engine engine;
  
  RID vertex_shader = engine.compile_shader(ShaderType::Vertex, "<path/to/vertex/shader>");
  RID fragment_shader = engine.compile_shader(ShaderType::Fragment, "<path/to/fragment/shader>");
}
```

The `RID` object ("resource ID") is the base of Groot Engine's resource management system. Groot Engine owns all of the Vulkan resources that you create so that you don't have to mange them yourself. Holding on to the resource's `RID` allows you to communicate with the engine about when you want to use the resource, as well as simplifies the setup of using the same resource for multiple purposes (ie. sharing a buffer between two descriptor sets). If the engine ever fails to create a resource, the RID will be invalid and you will get a message in the console.

Now, lets create a uniform buffer to keep track of the object's model matrix, camera view matrix, camera projection matrix, and the object's normal matrix

```c++
// main.cpp
...

struct UniformBuffer {
  mat4 model = mat4::identity();
  mat4 view = mat4::identity();
  mat4 proj = mat4::identity();
  mat4 norm = mat4::identity();
};

int main() {
  ...
  
  RID uniform_buffer = engine.create_uniform_buffer(sizeof(UniformBuffer));
}
```

Now that we have access to the transform matrices, we can update the vertex shader to use them

```glsl
// shader.vert

layout(binding = 0) uniform transform {
  mat4 _Model;
  mat4 _View;
  mat4 _Proj;
  mat4 _Norm;
};

...

void main() {
  gl_Position = _Proj * _View * _Model * vec4(_VertexPosition, 1.0);
  _UV = _VertexUV;
  _Normal = normalize(mat3(_Norm) * _VertexNormal);
}
```

Next, lets make a texture. In order to do this, we have to first create a sampler, and then use the sampler when creating the texture.

```c++
// main.cpp
...
int main() {
  ...
  
  RID sampler = engine.create_sampler(SamplerSettings{
    // feel free to explore different sampler settings here!
  });
  RID texture = engine.create_texture("<path/to/image>", sampler);
}
```

Now, lets update the fragment shader to use the texture (and normals passed in from the vertex shader)

```glsl
// shader.frag

layout(binding = 1) uniform sampler2D _Texture;

...

void main() {
  _FragColor = texture(_Texture, _UV);
}
```

Now the shaders are finished for this tutorial!

The next step is to create the descriptor set that uses the uniform buffer and texture that were just made.
Descriptor sets are just sets of buffers and images, or 'descriptors', that are pushed all at once to the GPU. It is more efficient to do it this way rather than sending the descriptors over one at a time.

```c++
// main.cpp
...
int main() {
  ...
  
  RID descriptor_set = engine.create_descriptor_set({ uniform_buffer, texture });
}
```

Descriptor sets order their binding by the order of RID's that you submit to them. In this example, `uniform_buffer` has binding 0 and `texture` has binding 1.
> There are also storage texture resources which are textures that are sampled in the graphics pipeline but are also storage images in the compute pipeline. These take up 2 consecultive bindings. For example, if `texture` was a storage texture in the above example, it would use binding 1 in the compute shader and binding 2 in the fragment shader

To full complete the graphics setup, we must create a graphics pipeline that uses the vertex and fragment shaders and the descriptor set

```c++
...
int main() {
  ...
  
  RID pipeline = engine.create_graphics_pipeline(GraphicsPipelineShaders{
    .vertex   = vertex_shader,
    .fragment = fragment_shader
  }, descriptor_set, GraphicsPipelineSettings{});
}
```

I recommend using the default `GraphicsPipelineSettings` for now, but if you want to change some settings for future objects, that struct is where you would do it.

Now load the mesh. Meshes are just a resource that keeps track of a vertex and index buffer retrieved from an obj file.

```c++
// main.cpp
...
int main() {
  ...
  
  RID mesh = engine.load_mesh("<path/to/obj/file>");
}
```

Using the mesh, descriptor set, and pipeline, you can create an `Object`

```c++
// main.cpp
...
int main() {
  ...
  
  Object object;
  object.set_mesh(mesh);
  object.set_pipeline(pipeline);
  object.set_descriptor_set(descriptor_set);
}
```

Now add the object to the scene

```c++
// main.cpp
...
int main() {
  ...
  
  engine.add_to_scene(object);
}
```

Now we can run the engine and make the cube rotate while were at it! Feel free to keep track of the object's transform however you want, but Groot Engine provides a `Transform` struct that has methods to do this automatically.

```c++
// main.cpp
...
int main() {
  ...
  
  Transform transform;
  UniformBuffer buffer{
    .proj = engine.camera_proj()
  };
  
  engine.run([&engine, &uniform_buffer, &transform, &buffer](double dt) {
    // add this to close out of the window by pressing escape
    // If you are curious about checking for keyboard/mouse input, read through
    // the 'input' section of Getting Started in the docs
    if (engine.just_pressed(Key::Escape))
      engine.close_window();
    
    transform.rotation += radians(10.0) * dt;
    
    buffer = UniformBuffer{
      .model  = transform.matrix(),
      .view   = engine.camera_view(),
      .norm   = transform.matrix().inverse().transpose()
    };
    
    engine.write_buffer(uniform_buffer, buffer);
  });
}
```

The `run` method takes in a function that returns void and accepts a double argument. The double argument is the frame time and will be constant since the engine loop runs at a fixed time step.

Now just build your target (make sure to link to Groot Engine) and run it and you'll see your object spinning in the scene!