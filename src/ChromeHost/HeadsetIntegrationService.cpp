/*
Jabra Browser Integration
https://github.com/gnaudio/jabra-browser-integration

MIT License

Copyright (c) 2017 GN Audio A/S (Jabra)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "HeadsetIntegrationService.h"
#include "CmdOffHook.h"
#include "CmdOnHook.h"
#include "CmdRing.h"
#include "CmdMute.h"
#include "CmdUnmute.h"
#include "CmdHold.h"
#include "CmdResume.h"
#include "CmdGetDevices.h"
#include "CmdGetActiveDevice.h"
#include "CmdSetActiveDevice.h"
#include "CmdGetVersion.h"
#include "EventMicMute.h"
#include "EventOffHook.h"
#include "EventOnline.h"
#include "EventLineBusy.h"
#include "EventReject.h"
#include "EventFlash.h"
#include "./SDK/JabraDeviceConfig.h"

HeadsetIntegrationService* g_thisHeadsetIntegrationService;

HeadsetIntegrationService::HeadsetIntegrationService()
{
  g_thisHeadsetIntegrationService = this;

  m_currentDeviceId = 0;

  m_commands.push_back(new CmdOffHook(this));
  m_commands.push_back(new CmdOnHook(this));
  m_commands.push_back(new CmdRing(this));
  m_commands.push_back(new CmdMute(this));
  m_commands.push_back(new CmdUnmute(this));
  m_commands.push_back(new CmdHold(this));
  m_commands.push_back(new CmdResume(this));

  m_commands.push_back(new CmdGetDevices(this));
  m_commands.push_back(new CmdGetActiveDevice(this));
  m_commands.push_back(new CmdSetActiveDevice(this));

  m_commands.push_back(new CmdGetVersion(this));

  m_events[Jabra_HidInput::Mute] = new EventMicMute(this);
  m_events[Jabra_HidInput::OffHook] = new EventOffHook(this);
  m_events[Jabra_HidInput::Online] = new EventOnline(this);
  m_events[Jabra_HidInput::LineBusy] = new EventLineBusy(this);
  m_events[Jabra_HidInput::RejectCall] = new EventReject(this);
  m_events[Jabra_HidInput::Flash] = new EventFlash(this);
}

HeadsetIntegrationService::~HeadsetIntegrationService()
{
  // TODO: Cleanup m_commands and m_events

  // Stop Jabra USB stack SDK
  Jabra_SetSoftphoneReady(false);
  Jabra_DisconnectFromJabraApplication();
  Jabra_Uninitialize();
}

void HeadsetIntegrationService::SendCmd(std::string msg)
{
  std::lock_guard<std::mutex> lock(m_mtx);

  using Iter = std::vector<CmdInterface*>::const_iterator;
  for (Iter it = m_commands.begin(); it != m_commands.end(); ++it) {
    if ((*it)->CanExecute(msg))
    {
      (*it)->Execute(msg);
      return;
    }
  }

  Error("Unknown cmd");
}

void HeadsetIntegrationService::AddHandler(std::function<void(std::string)> callback)
{
  m_callback = callback;
}

bool HeadsetIntegrationService::Start()
{
  Jabra_SetAppID("HKiKNeRIdH/8s+aIRdIVuRoi0vs5TkCXaOmwIqr0rMM=");

  Jabra_Initialize(
    NULL,
    StaticJabraDeviceAttachedFunc,
    StaticJabraDeviceRemovedFunc,
    NULL,
    StaticButtonInDataTranslatedFunc,
    0
  );

  Jabra_ConnectToJabraApplication(
    "D6B42896-E65B-4EC1-A037-27C65E8CFDE1",
    "Google Chrome Browser"
  );

  Jabra_SetSoftphoneReady(true);

  return true;
}

unsigned short HeadsetIntegrationService::GetCurrentDeviceId()
{
  if (m_devices.size() == 0)
    return USHRT_MAX;

  return m_currentDeviceId;
}

void HeadsetIntegrationService::SetCurrentDeviceId(unsigned short id)
{
  m_currentDeviceId = id;
}

std::string HeadsetIntegrationService::GetDevicesAsString()
{
  std::string devicesAsString;

  for (std::vector<int>::size_type i = 0; i != m_devices.size(); i++) {

    if (devicesAsString.length() > 0) {
      devicesAsString += ",";
    }

    // Battery information
    int levelInPercent;
    bool charging;
    bool batteryLow;
    Jabra_ReturnCode r = Jabra_GetBatteryStatus(
      m_devices[i].deviceID,
      &levelInPercent,
      &charging,
      &batteryLow
    );

    std::string test;

    const char *imageFilePath = Jabra_GetDeviceImageThumbnailPath(m_devices[i].deviceID);
    if (imageFilePath != NULL)
    {
      const int BUFFERSIZE = 14096;
      char * buffer = new char[BUFFERSIZE];

      std::ifstream t(imageFilePath, std::ios::in | std::ios::binary);
      std::vector<uint8_t> contents((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

      int contentsSize = contents.size();
      const BYTE *data = &contents[0];
      test = base64_encode(data, contentsSize);

      Jabra_FreeString((char *)imageFilePath);
    }

    //    std::ofstream imageFile;
    //    imageFile.open(imageFilePath, std::ios::out | std::ios::binary);
    //    std::string fileContent;
    //    fileContent << imageFile;


    //std::string test = "abc";
    //std::string test = base64_encode(data, contentsSize);

    //    const unsigned char *test1;
    //  unsigned int test2;
    //base64_encode(test1, test2);
    //base64_encode();

    //std::string test = base64_encode(test1, test2);




    //  std::streamsize size = t.tellg();

    //    t.read(buffer, BUFFERSIZE);

    //    std::string str((std::istreambuf_iterator<char>(t)),
    //      std::istreambuf_iterator<char>());


    //std::string test = "abc"; // base64_encode(&contents[0], contents.size());

    //std::string test = base64::decode(str);

    //std::string test = base64::encode("YW55IGNhcm5hbCBwbGVhc3VyZQ==");
    //    std::string test = base64::encode("YW55IGNhcm5hbCBwbGVhc3VyZQ==");

    //Jabra_FreeString();


    std::string realSerialString = "?";
    char realSerial[255];
    if (Jabra_GetSerialNumber(m_devices[i].deviceID, realSerial, 255) == Return_Ok)
    {
      std::string tmp(realSerial);
      realSerialString = tmp;
    }

    std::string firmwareVersion = "?";
    char bufferTmp[255];
    if (Jabra_GetFirmwareVersion(m_devices[i].deviceID, bufferTmp, 255) == Return_Ok)
    {
      std::string tmp(bufferTmp);
      firmwareVersion = tmp;
    }


    devicesAsString += std::to_string(m_devices[i].deviceID);
    devicesAsString += ",";
    devicesAsString += m_devices[i].deviceName;
    devicesAsString += ",";
    devicesAsString += realSerialString; // m_devices[i].serialNumber;
    devicesAsString += ",";
    devicesAsString += test; // "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";
    devicesAsString += ",";
    devicesAsString += firmwareVersion; // m_devices[i].serialNumber;
    devicesAsString += ",";



    if (r == Not_Supported)
    {
      devicesAsString += "Not_Supported";
    }
    else if (r == Device_Unknown)
    {
      devicesAsString += "Device_Unknown";
    }
    else
    {
      devicesAsString += std::to_string(levelInPercent);
      devicesAsString += "%";

      if (charging)
      {
        devicesAsString += " charging";
      }
    }
  }

  return "devices " + devicesAsString;
}

void HeadsetIntegrationService::Error(std::string msg)
{
  m_callback("Error: " + msg);
}

void HeadsetIntegrationService::Event(std::string msg)
{
  m_callback("Event: " + msg);
}

void HeadsetIntegrationService::SetHookStatus(unsigned short id, bool mute)
{
  m_HookStatus[id] = mute;
}

bool HeadsetIntegrationService::GetHookStatus(unsigned short id)
{
  return m_HookStatus[id];
}

void HeadsetIntegrationService::SetRingerStatus(unsigned short id, bool ringer)
{
  m_RingerStatus[id] = ringer;
}

bool HeadsetIntegrationService::GetRingerStatus(unsigned short id)
{
  return m_RingerStatus[id];
}

void HeadsetIntegrationService::JabraDeviceAttachedFunc(Jabra_DeviceInfo deviceInfo)
{
  std::lock_guard<std::mutex> lock(m_mtx);

  m_devices.push_back(deviceInfo);
  std::string deviceName(deviceInfo.deviceName);
  Event("device attached");
}

void HeadsetIntegrationService::JabraDeviceRemovedFunc(unsigned short deviceID)
{
  std::lock_guard<std::mutex> lock(m_mtx);

  int index = -1;

  using Iter = std::vector<Jabra_DeviceInfo>::const_iterator;
  for (Iter it = m_devices.begin(); it != m_devices.end(); ++it) {
    index++;
    if ((*it).deviceID == deviceID)
    {
      std::string deviceName((*it).deviceName);
      Event("device detached");

      m_devices.erase(m_devices.begin() + index);

      break;
    }
  }

  // If the removed device was the active device, assign a new active device (if any)
  if (m_currentDeviceId == deviceID) {
    if (m_devices.size() >= 0) {
      SetCurrentDeviceId(m_devices[0].deviceID);
    }
  }
}

void HeadsetIntegrationService::ButtonInDataTranslatedFunc(unsigned short deviceID, Jabra_HidInput translatedInData, bool buttonInData)
{
  std::lock_guard<std::mutex> lock(m_mtx);

  // Only handle input data from current device
  if (deviceID != GetCurrentDeviceId())
    return;

  EventInterface *event = m_events[translatedInData];
  if (event != NULL)
  {
    event->Execute(buttonInData);
  }
  else
  {
    std::string inDatastring = std::to_string(translatedInData);

    Error("No handler impelmented for " + inDatastring);
  }
}

void HeadsetIntegrationService::StaticJabraDeviceAttachedFunc(Jabra_DeviceInfo deviceInfo)
{
  g_thisHeadsetIntegrationService->JabraDeviceAttachedFunc(deviceInfo);
}

void HeadsetIntegrationService::StaticJabraDeviceRemovedFunc(unsigned short deviceID)
{
  g_thisHeadsetIntegrationService->JabraDeviceRemovedFunc(deviceID);
}

void HeadsetIntegrationService::StaticButtonInDataTranslatedFunc(unsigned short deviceID, Jabra_HidInput translatedInData, bool buttonInData)
{
  g_thisHeadsetIntegrationService->ButtonInDataTranslatedFunc(deviceID, translatedInData, buttonInData);
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string HeadsetIntegrationService::base64_encode(BYTE const* buf, unsigned int bufLen) {
  std::string ret;
  int i = 0;
  int j = 0;
  BYTE char_array_3[3];
  BYTE char_array_4[4];

  while (bufLen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i <4); i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while ((i++ < 3))
      ret += '=';
  }

  return ret;
}