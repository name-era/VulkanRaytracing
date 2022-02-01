#include "appBase.h"

int main() {

    AppBase* app = new AppBase();
    app->Initialize();
    app->Run();
    app->Destroy();

    return 1;
}