
#pragma once

#include <cstdint>

void driver_sequence(uintptr_t driver_num);

void driver_wait_match(uintptr_t driver_num);

void isolated_drive(uintptr_t driver_num);

void wait_for_payment(uintptr_t driver_num);