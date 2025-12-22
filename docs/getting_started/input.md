# Managing Input

> Every piece of code here will be inside of `engine.run()`. If you do not know the syntax for this function, read through [Rendering an Object](./graphics.md) or [Dispatching Compute Passes](./compute.md) first

Groot Engine stores the current input state of the keyboard and mouse buttons as well as the input state of the current frame. You can query each state as follows:

```c++
// current frame input
engine.just_pressed(Key / MouseButton);
engine.just_released(Key / MouseButton);

// current keyboard/mouse input
engine.is_pressed(Key / MouseButton);
```

The frame input state is reset at the end of each frame so it is better to check the frame input state for responses that should happen quickly and responsively like toggles, and check the overall input state if the response can be held down or happen over multiple frames.

> `Key` and `MouseButton` are both enums that are used to make accessing the input state more intuitive. For example, use `Key::W` to check if W is pressed or released. Similarly, use `MouseButton::Left` to check if the left mouse button is pressed or released.