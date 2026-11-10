//-----------------------------------------------------------------------------
// COMPILE-TIME OPTIONS FOR DEAR RBLXUI ALLEGRO 5 EXAMPLE
// See rblxuiconfig.h for the full template
// Because Allegro doesn't support 16-bit vertex indices, we enable the compile-time option of rblxui to use 32-bit indices
//-----------------------------------------------------------------------------

#pragma once

// Use 32-bit vertex indices because Allegro doesn't support 16-bit ones
// This allows us to avoid converting vertices format at runtime
#define RblxDrawIdx  int
