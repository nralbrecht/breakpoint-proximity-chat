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
- Quiting game mutes people repeated
    - Own position is not removed from server until unloading plugin
- TS3 has to be restarted instead of reloading to hear the other person reliably
- Fix sound direction when using drone
- Fix double radio clicks
- Radio quality dependent on distance
- Radio jamming
    - By azrael
    - EMP's
- Enemys detection range is better when player are louder
- Radio should get more quiet with distance
    - Configurable Min/Max distance

# Change log


## v0.1.6
- Radio sound processing
- Configuration options for processing
- Fix broken logging system

## v0.1.5
- Radio clicks
- Server position logging into sqlite3
- Make radio click volume configurable

## v0.1.4
- Add config
    - grb/config.json
- Logger log levels
    - OFF: 0, ERROR: 1, WARN: 2, VERBOSE: 3
- Logger add method to log without formating
- Add ClientStateManager
- Make server report a simple list
- Use voice rolloff to hear clients using the radio
- Add sqlite server position loging

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

# Credits
- [Radio Clicks from W2SJW](http://www.w2sjw.com/radio_sounds.html)
    - [mic_click_on](http://www.w2sjw.com/sounds/MDC-1200_DOS.mp3)
    - [mic_click_off](http://www.w2sjw.com/sounds/MDC-600-DOS.mp3)
