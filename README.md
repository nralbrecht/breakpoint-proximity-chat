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
- Use voice rolloff to hear clients using the radio

# Change log

## v0.1.4
- Add config
    - grb/config.json
- Logger log levels
    - OFF: 0, ERROR: 1, WARN: 2, VERBOSE: 3
- Logger add method to log without formating
- Add ClientStateManager
- Make server report a simple list

## v0.1.3
- Add custom voice rolloff calculation

## v0.1.2
- Every client is now moved to origin when server connection is lost
- Speeds set to:
    - 250 ms client send
    - 250 ms server send
    - might lead to consistent offset of up to 250 ms
- Log server report every 2 seconds
- Radio hotkey now works again
- Radio only froze mouth position when transmitting
