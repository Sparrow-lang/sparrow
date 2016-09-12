#include "Nest/Api/Backend.h"
#include "Nest/Utils/Assert.h"

static const int _maxBackends = 100;
Backend* _allBackends[100];
unsigned int _numBackends = 0;

int Nest_registerBackend(Backend* backend)
{
    if ( _numBackends >= _maxBackends )
        return -1;
    int idx = _numBackends++;
    _allBackends[idx] = backend;
    return idx;
}

int Nest_getNumBackends()
{
    return _numBackends;
}

Backend* Nest_getBackend(int idx)
{
    ASSERT(idx >= 0 && idx < _numBackends);
    return _allBackends[idx];
}
