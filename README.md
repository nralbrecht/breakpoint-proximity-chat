# Ghost Recon Breakpoint TS3 Plugin

A TS3 plugin and corresponding server for positional data and radio communication using TS3.

# Init Git Submodules
```
git submodule init
git submodule update
```

# TODO
- Distance correction
    - 72 meter ingame == 51.8 meter config
- Front and back are not distinguishable
- Propper server position logging
- Logging is disabled for the client
    - Introduce log level
    - Clear log file each run
    - Size cap?
- Radio blips
- Radio sound processing
- When quiting the game the own position is not removed from server

# Change log

## v0.1.2
- Every client is now moved to origin when server connection is lost
- Speeds set to:
    - 250 ms client send
    - 250 ms server send
    - might lead to consistent offset of up to 250 ms
- Log server report every 2 seconds
- Radio hotkey now works again
- Radio only froze mouth position when transmitting
