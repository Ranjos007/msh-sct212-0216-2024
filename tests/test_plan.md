# msh Test Plan

| Input | Expected behaviour | Actual result |
|-------|--------------------|---------------|
| `(blank line)` | New prompt, no error | New prompt `msh[0]>` displayed, no error |
| `echo hello world` | Prints text; status 0 | Printed `hello world`; prompt `msh[0]>` |
| `ls /nonexistent` | Error from `ls`; non-zero status | Printed `ls` error; prompt `msh[2]>` |
| `nosuchcommand` | Error message; status 127 | Printed `msh: No such file or directory`; prompt `msh[127]>` |
| `cd /tmp` then `pwd` | Prints `/tmp` | Printed `/tmp`; prompt `msh[0]>` |
| `sleep 3 &` | Prompt returns at once; PID shown | Printed `[bg] PID <pid>` and returned prompt immediately |
| `echo hi > f.txt` then `cat f.txt` | File contains `hi` | `cat f.txt` printed `hi`; prompt `msh[0]>` |
| `sleep 100` then `Ctrl-C` | Sleep stops; shell survives; status 130 | Sleep terminated; shell stayed open; prompt `msh[130]>` |
| `Ctrl-D` at the prompt | Shell exits cleanly | Shell exited cleanly with no leaks |
| `cd` (no args) then `pwd` | Changes to `$HOME` | Printed user's `$HOME` directory; status `0` |
| `help` | Displays built-in commands | Printed help message; status `0` |
| `   echo    spaced   ` | Ignores extra whitespace | Printed `spaced`; status `0` |
