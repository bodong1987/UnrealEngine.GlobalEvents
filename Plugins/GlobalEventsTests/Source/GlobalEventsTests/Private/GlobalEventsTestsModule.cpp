#include "GlobalEventsTestsModule.h"

IMPLEMENT_MODULE(FGlobalEventsTestsModule, GlobalEventsTests)

void FGlobalEventsTestsModule::StartupModule()
{
    // 在此处添加模块启动时的代码

    TTuple<int, float, int64> tuple{ 1, 1.23f, 100 };


}

void FGlobalEventsTestsModule::ShutdownModule()
{
    // 在此处添加模块关闭时的代码
}