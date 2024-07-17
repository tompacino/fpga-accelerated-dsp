#include <iostream>

#include "filters/lms/lms.h"
#include "AudioFile/AudioFile.h"

int main()
{
    LMS::VSS myFilter = LMS::VSS<float, 10U, true>(0.005F, 0.99F, 0.01F);
    return 0;
}
