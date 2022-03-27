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
    - Likely a problem with local state
    - List of other users not correctly initialized
- Make voice unidirectional
    - when using drone
    - when hearing radio
    - when driving in a vehicle?
- Radio jamming
    - when detected by azrael
    - when stuned by EMP
    - Could mybe be implement by detecting when the drone cant be launched
- Make enemy detection range larger when clients are speaking
- Radio should get more quiet with distance
    - Configurable Min/Max distance
- Set mouth position to avatar when in vehicle
- Create class diagram
- Look into publishing a TS3 addon
- Look into ZeroMQ for message transport

# Change log

## v0.1.8
- Change default port to 9099
- Make server compatible with linux
- Add Dockerfile to build server container
- Radio destruction audio effect is increased with distance
    - radio_destruction_min @ radio_distance_min
    - radio_destruction_max @ radio_distance_max
- Add linebrakes to end of network message as an atempt to reduce mixed and broken JSON
- Decuple game and process from GameHandler
- Make logger thread safe

## v0.1.7
- Fix plugin crash on unload due to wrong order of unloading components
- Fix radio click on new client join
- Fix double radio clicks
- Fix swaped avatar position and camera position pointer for GRB_vulcan.exe

## v0.1.6
- Radio sound processing
- Configuration options for processing
- Fix broken logging system

## v0.1.5
- Radio clicks on start and stop transmitting
    - grb/mic_click_on.wav
    - grb/mic_click_off.wav
- Server position logging into sqlite3
- Make radio click volume configurable
- Use relative paths to logfile and expect config in grb/config.json

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
