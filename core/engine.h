// Bangla Keyboard Switcher - shared engine interface.
// Pure C++17, no Windows dependency, so the same code is unit-tested on any OS
// and linked into the Windows TSF DLL.
#pragma once
#include <string>

namespace bks {

// Both engines work on "typed ASCII": the character a US-QWERTY keyboard would
// produce for the pressed key (Shift applied). The TSF layer converts virtual
// keys to this ASCII form, so the result does not depend on the active Windows
// layout.
class Engine {
public:
    virtual ~Engine() {}
    // True if 'c' belongs inside the composition (pre-edit) buffer.
    virtual bool IsComposable(char c) const = 0;
    // Convert the whole typed buffer to Bengali Unicode. Pure function:
    // Backspace = remove last char from the buffer and call Convert again.
    virtual std::wstring Convert(const std::string& typed) const = 0;
    // Mapping for a character that is NOT composable (digits, '.', ...).
    // Empty result = let the application receive the key unchanged.
    virtual std::wstring Standalone(char c) const { (void)c; return std::wstring(); }
};

const Engine& AvroEngine();
const Engine& BijoyEngine();

} // namespace bks
