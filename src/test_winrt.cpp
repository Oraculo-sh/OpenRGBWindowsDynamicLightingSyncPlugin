#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Lights.h>
#include <winrt/Windows.Devices.Lights.Effects.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Lights;
using namespace Windows::Devices::Lights::Effects;

int main()
{
    init_apartment();
    return 0;
}