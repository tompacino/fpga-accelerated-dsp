#include <iostream>

#include "filters/lms/lms.h"

int main()
{
    LMS::VSS myFilter = LMS::VSS<float, 1, 0.0F, 0.99F, 0.01F, true>(0.0F);
    return 0;
}
