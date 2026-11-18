#pragma once
#include <Arduino.h>

// Reads the number after `"key":` in a flat JSON string (key includes its quotes, e.g. "\"ex\"").
// No JSON lib for a couple of floats; won't handle nesting or duplicate keys.
inline bool jsonNum(const char *s, const char *key, float &out) {
  const char *p = strstr(s, key);
  if (!p || !(p = strchr(p, ':'))) return false;
  out = atof(p + 1);
  return true;
}
