# XGospel-2.0 Modern Protocol Support

This document describes the modern protocol enhancements added to XGospel-2.0, based on improvements from the q5Go client.

## Overview

The modern protocol support provides:

- **Enhanced Server Compatibility**: Support for IGS, NNGS, LGS, PandaNet, and other Go servers
- **Improved Authentication**: Better login handling with multiple authentication methods
- **Extended Protocol Parsing**: Support for modern server commands and message formats
- **UTF-8 Encoding Support**: International character support for player names and messages
- **Robust Connection Management**: Better error handling and reconnection capabilities

## New Files Added

### Core Modern Protocol Files
- `modern_connect.h/c` - Enhanced connection handling
- `modern_parser.h/c` - Modern protocol message parsing
- `modern_integration.c` - Integration with existing XGospel architecture
- `modern_xgospel_patch.c` - Application-level integration patches

### Key Features

#### 1. Server Type Detection
The system automatically detects server types based on login messages:
- IGS (Internet Go Server)
- NNGS (No Name Go Server) 
- LGS (Local Go Server)
- PandaNet
- Tygem
- WBG

#### 2. Enhanced Authentication
- Automatic username/password handling
- Guest login support
- Authentication state tracking
- Login failure detection

#### 3. Modern Command Processing
Support for extended server commands including:
- Command 1: Login information
- Command 5: Error messages
- Command 7: Games list
- Command 11: Kibitz messages
- Command 15: Move notifications
- Command 19: Say messages
- Command 21: Shout messages
- Command 22: Status updates
- Command 27: Match requests
- Command 42: Time additions
- Command 48: Seek listings
- Command 49: Channel tells

#### 4. Protocol Improvements
- Better message parsing with regular expressions
- UTF-8 character encoding support
- Enhanced error handling
- Connection stability monitoring
- Automatic keepalive functionality

## Integration with Existing Code

The modern protocol support is designed to be backward compatible. The integration works as follows:

1. **Connection Layer**: `ModernConnection` wraps the existing `Connection` structure
2. **Parser Integration**: Modern parsing runs first, falls back to original parser
3. **Message Processing**: Enhanced message handling with original system fallback
4. **Command Sending**: Modern command transmission with legacy support

## Usage

### Basic Integration

To enable modern protocol support, the following integration points are available:

```c
// Initialize modern features (call from main())
InitModernXGospel();

// Enhanced connection (replacement for Connect())
Connection conn = ModernConnect_Wrapper(site, port);

// Enhanced output processing (replacement for Output())
ModernOutput(server_message);

// Enhanced command sending (replacement for SendCommand())
ModernSendCommand("who");

// Cleanup on exit
CleanupModernXGospel();
```

### Status Information

Check connection status and server capabilities:

```c
// Check if modern connection is active
if (IsModernConnectionActive()) {
    ShowModernConnectionStatus();
}

// Get server capabilities
int capabilities = GetModernServerCapabilities();
```

## Backward Compatibility

The modern protocol support is designed to maintain full backward compatibility:

- Original XGospel functionality remains unchanged
- Existing command parsing still works
- Falls back gracefully when modern features are unavailable
- No changes required to existing user interface code

## Configuration

The modern protocol features use the same configuration as the original XGospel:

- `appdata.User` - Username for authentication
- `appdata.Password` - Password for authentication  
- `appdata.Site` - Server hostname
- `appdata.Port` - Server port (default 6969)

## Building

The modern protocol files are automatically included in the build process through the updated `Makefile.in`. No additional configuration is required.

```bash
./configure
make
```

## Debugging

Enhanced debugging features:

- Connection status logging
- Protocol message tracing  
- Authentication state monitoring
- Server capability detection logging

Enable verbose output by setting debug flags or checking the console output for modern protocol messages.

## Protocol Command Reference

### Supported Commands (from q5Go)

| Command | Type | Description |
|---------|------|-------------|
| 1 | LOGIN | Login and connection information |
| 2 | BEEP | Audio notification |
| 5 | ERROR | Error messages from server |
| 7 | GAMES | List of games in progress |
| 8 | HELP | Help text display |
| 9 | INFO | General information messages |
| 11 | KIBITZ | Game commentary |
| 14 | MESSAGE | Private messages |
| 15 | MOVE | Game moves |
| 19 | SAY | In-game chat |
| 20 | SCORE | Game scoring information |
| 21 | SHOUT | Server-wide announcements |
| 22 | STATUS | Game status updates |

### Extended Commands

| Command | Type | Description |
|---------|------|-------------|
| 27 | MATCH_REQUEST | Match requests from players |
| 42 | TIME_ADDED | Time additions to games |
| 48 | SEEK_LIST | Seek advertisements |
| 49 | CHANNEL_TELL | Channel-based messaging |

## Server Compatibility

The modern protocol support has been tested with:

- **IGS**: Full compatibility with Internet Go Server protocol
- **NNGS**: Enhanced support for No Name Go Server features
- **LGS**: Local Go Server compatibility
- **Other Servers**: Basic protocol support with graceful degradation

## Future Enhancements

Planned improvements:
- SSL/TLS connection support
- WebSocket protocol support for modern servers
- Enhanced game analysis integration
- Real-time game streaming capabilities
- Mobile-friendly protocol optimizations

## Troubleshooting

### Common Issues

1. **Connection Fails**: Check server hostname and port, verify credentials
2. **Authentication Problems**: Ensure username/password are correct, check for guest access
3. **Protocol Errors**: Enable debug logging to trace message parsing issues
4. **Encoding Issues**: Verify UTF-8 support is enabled for international characters

### Debug Information

Enable debugging by checking console output for messages prefixed with:
- "Modern connection..."
- "Enhanced connection..."
- "Server type detected..."
- "Authentication status..."

## Version History

- **v1.0**: Initial modern protocol integration
- Based on q5Go 2.1.3 protocol improvements
- Integrated with XGospel-2.0 (1999-2025 update)

---

*This enhancement maintains the spirit of the original XGospel while providing modern server compatibility and protocol support.*