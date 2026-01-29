# Statful_UNIX_Shell-

This project implements a **Unix-style interactive shell** in **C**, built from the ground up to behave like a real command interpreter. It provides a persistent command-line environment capable of launching programs, coordinating processes, managing input/output streams, and supporting interactive conveniences such as history navigation and autocompletion.

The emphasis is on understanding and controlling how Unix systems execute commands—at the level of processes, file descriptors, and terminal interaction.

---

## What the Shell Does

At runtime, the shell maintains an interactive session that continuously accepts user input, interprets it, and executes commands accordingly. It preserves internal state across commands, allowing navigation, history recall, and consistent environment behavior.

The shell distinguishes between commands that must run *inside* the shell process (such as directory changes) and those that should be delegated to child processes, mirroring the execution model of traditional Unix shells.

---

## Command Interpretation

User input is parsed into tokens with careful handling of:

- quoted strings
- escaped characters
- special operators such as pipes and redirections

This allows complex commands to be expressed naturally while ensuring arguments are passed exactly as intended.

---

## Executing Programs

### Internal Commands

Certain commands directly affect the shell’s state and are therefore executed without forking:

- terminating the shell session
- printing or changing the working directory
- inspecting command history
- echoing output
- identifying how a command would be resolved

These operations run in the parent process so their effects persist.

### External Programs

All other commands are executed by creating child processes. The shell:

- locates executables using the `PATH` environment
- verifies execution permissions
- spawns processes using `fork`
- replaces the child process image using `exec`
- synchronizes execution using `waitpid`

---

## Data Flow Between Commands

The shell allows commands to be connected so that the output of one becomes the input of another. This is achieved by explicitly constructing pipelines, managing intermediate file descriptors, and ensuring each process receives the correct standard streams.

Multi-stage pipelines are supported, and file descriptors are cleaned up carefully to avoid leaks or interference between commands.

---

## Input and Output Control

Commands can redirect their input and output streams to and from files. The shell supports:

- overwriting or appending output
- redirecting error streams independently
- restoring original terminal streams after execution

All redirection is handled through direct manipulation of file descriptors, making the data flow explicit and predictable.

---

## Interactive Experience

To behave like a usable shell rather than a command runner, the project includes interactive features:

### Command History
- Commands entered during a session are recorded
- History can be browsed interactively using the keyboard
- Previous commands can be listed on demand
- History data is retained across sessions

### Autocompletion
- Pressing tab attempts to complete commands or paths
- Built-in commands and executables are suggested
- Files and directories are completed contextually
- Multiple matches are handled gracefully without blocking input

---

## Internal Structure and Design Choices

- Process creation is minimized and only used when required
- The shell maintains ownership of the terminal whenever possible
- File descriptors are explicitly duplicated and restored
- No shell frameworks or high-level abstractions are used

These choices keep behavior transparent and make the code easier to reason about and extend.

---

## Technologies and Concepts

- **Language:** C++
- **System Interfaces:** POSIX system calls
- **Key mechanisms:**  
  process creation, execution replacement, pipes, file descriptors, environment variables, terminal I/O

---

## Sample Session

```sh
$ pwd
/home/user

$ cd /usr/bin

$ echo "Hello World"
Hello World

$ ls | grep bash > output.txt

$ history
1 pwd
2 cd /usr/bin
3 echo "Hello World"
4 ls | grep bash > output.txt

$ exit
```
## Summary
This shell is designed to be both functional and instructive. It demonstrates how interactive command environments are built at a low level, exposing the mechanics behind everyday shell usage while remaining practical enough to use for real tasks

