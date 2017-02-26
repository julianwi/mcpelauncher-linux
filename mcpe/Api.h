#pragma once

namespace minecraft {
namespace api {

class Api {

public:

    void* vtable;
    std::string envPath;
    void* networkIfaceVtable;
    void* playerIfaceVtable;

};

}
}