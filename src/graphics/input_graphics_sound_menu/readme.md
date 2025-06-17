# input graphics sound menu
This is a basic menu that could used in almost any 3d program.

it requires the following sounds to be defined:

```cpp
std::unordered_map<SoundType, std::string> sound_type_to_file = {
    {SoundType::UI_HOVER, "assets/sounds/hover.wav"},
    {SoundType::UI_CLICK, "assets/sounds/click.wav"},
    {SoundType::UI_SUCCESS, "assets/sounds/success.wav"},
};
```
