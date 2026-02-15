# Quick Start Guide

Get started with the Glu VSCode extension in 3 simple steps!

## Step 1: Install the Extension

### Option A: Via Command Line (Fastest)
```bash
code --install-extension glu-syntax-0.1.0.vsix
```

### Option B: Via VSCode GUI
1. Open VSCode
2. Press `Ctrl+Shift+X` (Windows/Linux) or `Cmd+Shift+X` (macOS)
3. Click the `...` menu at the top
4. Select "Install from VSIX..."
5. Choose `glu-syntax-0.1.0.vsix`

## Step 2: Open a Glu File

Create a new file with `.glu` extension or open an existing one:

```bash
touch hello.glu
code hello.glu
```

## Step 3: Start Coding!

Try this example:

```glu
// Simple Glu program
import io::printLine;

@inline
func greet(name: String) -> Void {
    printLine("Hello, " + name + "!");
}

func main() -> Int {
    let name = "World";
    greet(name);
    return 0;
}
```

You should see:
- ✨ Keywords highlighted (`func`, `let`, `return`)
- 🎨 Types colored (`String`, `Int`, `Void`)
- 💬 Comments in muted colors
- 🔧 Functions and attributes clearly visible

## What's Next?

- 📖 Read [FEATURES.md](FEATURES.md) to see all supported language features
- 🔧 Check [INSTALL.md](INSTALL.md) if you encounter any issues
- 💡 Look at [test-syntax.glu](test-syntax.glu) for more examples

## Common Tasks

### Toggle Line Comment
- Windows/Linux: `Ctrl+/`
- macOS: `Cmd+/`

### Toggle Block Comment
- Windows/Linux: `Shift+Alt+A`
- macOS: `Shift+Option+A`

### Change Color Theme
1. Press `Ctrl+K Ctrl+T` (Windows/Linux) or `Cmd+K Cmd+T` (macOS)
2. Select your preferred theme

Recommended themes: Dark+, Monokai, One Dark Pro

## Troubleshooting

### Extension Not Working?
1. Check the language mode shows "Glu" (bottom-right corner)
2. If not, click it and select "Glu" from the list
3. Reload window: `Ctrl+Shift+P` → "Reload Window"

### Need More Help?
See the full [INSTALL.md](INSTALL.md) guide for detailed troubleshooting.

## Resources

- 🌐 [Glu Website](https://glu-lang.org/)
- 📚 [Glu Documentation](https://glu-lang.org/reference/)
- 📖 [The Glu Book](https://glu-lang.org/theBook/)
- 🐛 [Report Issues](https://github.com/glu-lang/glu/issues)

---

**Happy Coding with Glu! 🚀**
