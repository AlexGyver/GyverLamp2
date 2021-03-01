#pragma once
// добавлена синхронизация обнвления по миллисекундам
// добавлен вывод миллисекунд

#include "Arduino.h"

#include <Udp.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

class NTPClient {
  private:
    UDP*          _udp;
    bool          _udpSetup       = false;

    const char*   _poolServerName = "pool.ntp.org"; // Default time server
    int           _port           = NTP_DEFAULT_LOCAL_PORT;
    long          _timeOffset     = 0;

    unsigned long _updateInterval = 60000;  // In ms

    unsigned long _currentEpoc    = 0;      // In s
    unsigned long _lastUpdate     = 0;      // In ms

    byte          _packetBuffer[NTP_PACKET_SIZE];

    void          sendNTPPacket();

  public:
    NTPClient(UDP& udp);
    NTPClient(UDP& udp, long timeOffset);
    NTPClient(UDP& udp, const char* poolServerName);
    NTPClient(UDP& udp, const char* poolServerName, long timeOffset);
    NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval);

    /**
       Set time server name

       @param poolServerName
    */
    void setPoolServerName(const char* poolServerName);

    /**
       Starts the underlying UDP client with the default local port
    */
    void begin();

    /**
       Starts the underlying UDP client with the specified local port
    */
    void begin(int port);

    /**
       This should be called in the main loop of your application. By default an update from the NTP Server is only
       made every 60 seconds. This can be configured in the NTPClient constructor.

       @return true on success, false on failure
    */
    bool update();

    /**
       This will force the update from the NTP Server.

       @return true on success, false on failure
    */
    bool forceUpdate();

    int getDay() const;
    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;
    int getMillis() const;
    int getMillisLastUpd() const;

    /**
       Changes the time offset. Useful for changing timezones dynamically
    */
    void setTimeOffset(int timeOffset);

    /**
       Set the update interval to another frequency. E.g. useful when the
       timeOffset should not be set in the constructor
    */
    void setUpdateInterval(unsigned long updateInterval);

    /**
       @return time formatted like `hh:mm:ss`
    */
    String getFormattedTime() const;

    /**
       @return time in seconds since Jan. 1, 1970
    */
    unsigned long getEpochTime() const;

    /**
       Stops the underlying UDP client
    */
    void end();
};
