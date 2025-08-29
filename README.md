# Windows Dynamic Lighting Sync Plugin for OpenRGB

## What is this?

Windows Dynamic Lighting Sync is an [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) plugin that syncs the dynamic lighting of your RGB devices with the Windows 10/11 Dynamic Lighting feature. This plugin allows OpenRGB to control Windows Dynamic Lighting compatible devices through the Windows Runtime API.

## Features

- 🔄 **Windows Dynamic Lighting Integration**: Makes OpenRGB devices compatible with Windows Dynamic Lighting, allowing Windows to control all RGB devices
- 🎨 **Color Matching**: Accurate color reproduction across different lighting systems
- ⚡ **Real-time Updates**: Instant synchronization of lighting changes
- 🛡️ **Safe Operation**: Built-in error handling and crash prevention
- 🔧 **Easy Integration**: Seamless integration with existing OpenRGB setups
- 📱 **Windows 11 Compatible**: Full support for Windows 11 Dynamic Lighting API

## System Requirements

- Windows 10 (version 1903 or later) or Windows 11
- OpenRGB 0.7 or later
- Visual C++ Redistributable 2019 or later
- Devices compatible with Windows Dynamic Lighting

## Downloads

Download the latest release from the [Releases](../../releases) page.

**Latest Version**: v1.0.0
- `WindowsDynamicLightingSync.dll` - Plugin file for OpenRGB

## Installation

### Automatic Installation

1. Download the latest `WindowsDynamicLightingSync.dll` from the releases page
2. Copy the DLL file to your OpenRGB plugins directory:
   - Default location: `%APPDATA%\OpenRGB\plugins\`
   - Alternative: `<OpenRGB Installation>\plugins\`
3. Restart OpenRGB
4. The plugin should appear in the "Plugins" tab

## Usage

1. **Enable the Plugin**:
   - Open OpenRGB
   - Go to the "Plugins" tab
   - Enable "Windows Dynamic Lighting Sync"

2. **Configure Sync Settings**:
   - The plugin will automatically detect Windows Dynamic Lighting compatible devices
   - Configure OpenRGB devices to be controlled by Windows Dynamic Lighting
   - Set update frequency and color matching preferences

3. **Start Syncing**:
   - Click "Start Sync" to begin synchronization
   - Changes made in OpenRGB will be reflected in Windows Dynamic Lighting and vice versa

## Troubleshooting

### Plugin Not Loading
- Ensure OpenRGB is version 0.7 or later
- Check that the plugin DLL is in the correct plugins directory
- Verify Visual C++ Redistributable is installed

### Sync Not Working
- Confirm your devices support Windows Dynamic Lighting
- Check Windows Dynamic Lighting is enabled in Windows Settings
- Restart both OpenRGB and the plugin

### Performance Issues
- Reduce sync frequency in plugin settings
- Close unnecessary applications that might interfere with lighting control

## Development

### Project Structure
```
WindowsDynamicLightingSync/
├── src/                          # Source code
│   ├── WindowsDynamicLightingSync.cpp
│   ├── WindowsDynamicLightingSync.h
│   └── WindowsDynamicLightingSync.pro
├── scripts/                      # Build scripts
│   └── build-plugin.bat
├── dependencies/                 # Dependencies
│   ├── OpenRGBSamplePlugin/     # OpenRGB plugin template
│   └── OpenRGB-Qt-Packages/     # Qt packages
└── release/                      # Compiled binaries
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) - The amazing RGB lighting control software
- [OpenRGB Sample Plugin](https://gitlab.com/OpenRGBDevelopers/OpenRGBSamplePlugin) - Plugin development template
- Microsoft - For the Windows Dynamic Lighting API

## Support

If you encounter issues or have questions:
- Check the [Issues](../../issues) page
- Create a new issue with detailed information about your problem
- Include your OpenRGB version, Windows version, and device information

