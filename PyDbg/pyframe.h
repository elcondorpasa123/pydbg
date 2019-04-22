#pragma once

HRESULT CALLBACK pyframe(PDEBUG_CLIENT4 Client, PCSTR args);
void pyframe_impl(ULONG_PTR Address);