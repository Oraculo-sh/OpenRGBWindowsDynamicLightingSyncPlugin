# Windows Dynamic Lighting Sync (OpenRGB Plugin)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE) [![Status](https://img.shields.io/badge/status-alpha-orange.svg)](#) [![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue.svg)](#) [![OpenRGB](https://img.shields.io/badge/OpenRGB-plugin-8A2BE2.svg)](#) [![Windows 11](https://img.shields.io/badge/Windows%2011-Compatible-00A4EF.svg)](#)

Synchronize your PC's RGB lighting with Windows Dynamic Lighting directly from OpenRGB. This project bridges RGB devices managed by OpenRGB with the native Windows Dynamic Lighting feature, delivering a unified, consistent, and low-latency experience.

## What this project is

Windows Dynamic Lighting Sync is an OpenRGB plugin that exposes your OpenRGB-controlled devices to Windows 10/11 through the Dynamic Lighting feature. In practice, it acts as a bridge between OpenRGB and Windows, allowing the OS to apply effects, colors, and lighting behaviors in an integrated way without sacrificing OpenRGB flexibility.

## Key features

- Native integration with Windows Dynamic Lighting (Windows 10/11)
- Real-time color synchronization between Windows and OpenRGB devices
- Consistent LED mapping (color and brightness) for predictable results
- Robust operation with error handling and resilience to disconnections
- Broad device compatibility via OpenRGB's ecosystem

## How it works (overview)

- The plugin connects to a lightweight local service that translates protocol messages between OpenRGB and Windows Dynamic Lighting.
- When Windows changes an effect/color, the plugin receives the update and applies it to devices present in OpenRGB.
- When OpenRGB changes a state, the plugin can reflect it to keep both sides consistent where applicable.
- All communication is local (on your machine), reducing latency and avoiding network dependency.

## Scope and goals

- Bring the wide ecosystem of OpenRGB-supported devices into the Windows Dynamic Lighting world.
- Preserve OpenRGB's configuration freedom while enabling compatibility with Windows features and apps.
- Provide a simple user experience with minimal steps to get started (no technical setup details in this document).

## Compatibility

- Windows 10 (1903+) and Windows 11 with Dynamic Lighting enabled
- Devices supported by OpenRGB (various brands and controllers)
- OpenRGB with plugin support enabled

## Project status

- Version: 0.1 alpha
- Current focus: protocol stability, coverage of common scenarios, and end-to-end validation
- Planned improvements: LED mapping refinements, optional telemetry metrics, sync customization options, and expanded compatibility

## Privacy & security

- All communication happens locally on your computer
- The project does not collect personal data or transfer information to the internet
- Logs and diagnostics (when enabled) remain local and are intended for troubleshooting only

## Acknowledgments

- [OpenRGB](https://openrgb.org/) — the solid, cross-platform RGB control foundation
- OpenRGB Community — contributions, references, samples, and documentation (see the project on [GitLab](https://gitlab.com/CalcProgrammer1/OpenRGB))
- Microsoft — for the Windows Dynamic Lighting feature

## License

Distributed under the MIT License. See the [LICENSE](LICENSE) file for details.
