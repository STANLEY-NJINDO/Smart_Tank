#ifndef _WATERLEVELSENSOR_H_
#define _WATERLEVELSENSOR_H_

#include <SinricProDevice.h>
#include <Capabilities/RangeController.h>
#include <Capabilities/PushNotification.h>

class WATERLEVELSENSOR 
: public SinricProDevice
, public RangeController<WATERLEVELSENSOR>
, public PushNotification<WATERLEVELSENSOR> {
  friend class RangeController<WATERLEVELSENSOR>;
  friend class PushNotification<WATERLEVELSENSOR>;
public:
  WATERLEVELSENSOR(const String &deviceId) : SinricProDevice(deviceId, "WATERLEVELSENSOR") {};
};

#endif
