> **IMPORTANT NOTE:** This project is not yet compatible with the latest Ghost Recon Breakpoint update. See: https://github.com/nralbrecht/breakpoint-proximity-chat/issues/1

# Ghost Recon Breakpoint Proximity Chat

A TS3 plugin and corresponding server for poximity based radio communication while playing Ghost Recon Breakpoint in co-op.

# Usage

To use the proximity chat, each player must have TeamSpeak 3 running on the same computer as Ghost Recon Breakpoint, be in the same channel, and have the plugin installed and enabled. The server must be running on a computer that is accessible over the internet via the configured port for all participating players.

#### Setting Up TS3 Plugin

Compile the plugin using CMake:
```
# Initialize dependencies
git submodule init
git submodule update

# I can't remember the exact compile commands, but likely:
mkdir build
cd build
cmake ..
make
```

Copy the resulting `gbr_0.1.3.dll` to the TS3 plugin directory at `%appdata%\TS3Client\plugins`.
Create a directory next to the DLL named `gbr`. Copy all files from the [resources](https://github.com/nralbrecht/breakpoint-proximity-chat/blob/main/resources) directory in the repository into this new directory. Rename `default-config.json` to `config.json`.
Open the configuration file in a text editor and update `server_host` and `server_port` to match the IP address/hostname and port of your server. All other settings can be customized as you prefer. Note that these settings only apply to your local plugin. It is recommended (but not strictly required) that all players use the same settings for the best experience.

Finally, start the game and enable the plugin in the TeamSpeak 3 settings. The plugin should then send and receive position updates via the server and begin adjusting the volume of other players who also have the plugin running.

To debug any errors, check the `grb_latest.log` file generated alongside the configuration file.

#### Setting Up Server

The easiest way to run the server is via Docker using the provided `docker-compose.yml`. Run the following command:
```
docker-compose up
# or run it in the background using
docker-compose up -d
```

The server should start and become available for the TS3 plugin to connect to. Make sure the computer is accessible from the internet and that the required port (default: `9099`) is not blocked by your router or any firewall.

# TODO
- Upgrade dependencies
    - [ ] asio2
    - [ ] sqlite3
    - [ ] teamspeak 3 sdk
- Create GitHub release with precompiled TS plugin
- Distance correction
    - 72 meter ingame == 51.8 meter config
- Quiting game mutes people repeatedly
    - Own position is not removed from server until unloading plugin
- TS3 has to be restarted instead of reloading to hear the other person reliably
    - Likely a problem with local state
    - List of other users not correctly initialized
- Make voice unidirectional
    - When using drone
    - When hearing radio
    - When driving in a vehicle?
- Radio jamming
    - When detected by Azrael
    - When stuned by EMP
    - Could mybe be implement by detecting when the drone cant be launched
- Make enemy detection range larger when clients are speaking
- Radio should get more quiet with distance
    - Configurable Min/Max distance
- Set mouth position to avatar when in vehicle
- Create class diagram
- Look into publishing a TS3 addon
- Make the networking settup easier
    - Add a lobby feature to the server so that not everybody needs to deploy their own server
        - Maybe simply add a lobby code in the addon config?
    - Move to using peer to peer networking

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
