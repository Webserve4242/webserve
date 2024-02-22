#pragma once

# include <string>

typedef int Fd;


enum ProcResult {
    Success,
    Continue,
    ExecutingCgi,
    PrepareNextProc,
    ConnectionClosed,
    
    Failure,
    FatalError
};
